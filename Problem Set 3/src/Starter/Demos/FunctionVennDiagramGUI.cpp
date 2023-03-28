#include "../GUI/MiniGUI.h"
#include "../Utilities/JSON.h"
#include "../PropertiesOfFunctions.h"
#include "gobjects.h"
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;
using namespace MiniGUI;

namespace {
    const string kBackgroundColor = "white";
    const Font kLabelFont(FontFamily::SERIF, FontStyle::ITALIC, 18, "black");
    const Font kAnswerFont(FontFamily::MONOSPACE, FontStyle::NORMAL, 16, "blue");
    const string kBoundsColor    = "black";

    /* RGB constructor. */
    int rgb(int r, int g, int b) {
        return 0xFF000000 + (r << 16) + (g << 8) + b;
    }

    /* Inputs are colors, output is squared Euclidean distance between colors. */
    int distanceBetween(int lhs, int rhs) {
        int dr = ((lhs >> 16) & 0xFF) - ((rhs >> 16) & 0xFF);
        int dg = ((lhs >>  8) & 0xFF) - ((rhs >>  8) & 0xFF);
        int db = ((lhs >>  0) & 0xFF) - ((rhs >>  0) & 0xFF);

        return dr * dr + dg * dg + db * db;
    }

    GImage& imageFor(FnLoc location) {
        if (location == FnLoc::NOT_A_FUNCTION) {
            static GImage result("res/images/venn-notfunction.png");
            return result;
        } else if (location == FnLoc::FUNCTION) {
            static GImage result("res/images/venn-function.png");
            return result;
        } else if (location == FnLoc::INJECTION) {
            static GImage result("res/images/venn-injection.png");
            return result;
        } else if (location == FnLoc::SURJECTION) {
            static GImage result("res/images/venn-surjection.png");
            return result;
        } else if (location == FnLoc::BIJECTION) {
            static GImage result("res/images/venn-bijection.png");
            return result;
        } else if (location == FnLoc::UNSELECTED) {
            static GImage result("res/images/venn-none.png");
            return result;
        } else {
            error("Internal error: Unknown function type?");
        }
    }

    /* Given a coordinate within the Venn diagram image (mapped to [0, 1) x [0, 1)),
     * returns the type of location that's selected.
     */
    FnLoc locationFor(double xLogical, double yLogical) {
        static GImage theMap("res/images/venn-map.png");

        int x = xLogical * theMap.getWidth();
        int y = yLogical * theMap.getHeight();

        /* Default to UNSELECTED if we're out of bounds. */
        if (x < 0 || y < 0 || x >= theMap.getWidth() || y >= theMap.getHeight()) {
            return FnLoc::UNSELECTED;
        }

        /* Otherwise, use the colors of the pixel to decode where we are. Is this
         * a super janky hack? Of course! Could we do better? Of course!
         *
         * We can't do an exact match here, so instead we'll have to compare the
         * pixel color against all the other colors and find whichever one is
         * closest.
         */
        const static vector<pair<FnLoc, int>> theColors = {
            make_pair(FnLoc::NOT_A_FUNCTION, rgb(0xFF, 0x00, 0x00)),
            make_pair(FnLoc::FUNCTION,       rgb(0x00, 0xFF, 0x00)),
            make_pair(FnLoc::INJECTION,      rgb(0x00, 0x00, 0xFF)),
            make_pair(FnLoc::SURJECTION,     rgb(0x00, 0x00, 0x00)),
            make_pair(FnLoc::BIJECTION,      rgb(0xFF, 0xFF, 0xFF)),
        };

        int color = theMap.getPixel(x, y);

        int bestDist = numeric_limits<int>::max();
        FnLoc best = FnLoc::UNSELECTED;

        for (const auto& entry: theColors) {
            int dist = distanceBetween(color, entry.second);
            if (dist < bestDist) {
                bestDist = dist;
                best = entry.first;
            }
        }

        return best;
    }

    /* Gives the bounding rectangle for where to place the label for
     * the given class. Determined empirically and unscientifically. :-)
     */
    GRectangle labelBoxFor(FnLoc location) {
        if (location == FnLoc::NOT_A_FUNCTION) {
            return {0.4,   0.8,  0.2, 0.1};
        } else if (location == FnLoc::FUNCTION) {
            return {0.4,   0.08, 0.2, 0.1};
        } else if (location == FnLoc::INJECTION) {
            return {0.173, 0.4,  0.2, 0.1};
        } else if (location == FnLoc::SURJECTION) {
            return {0.627, 0.4,  0.2, 0.1};
        } else if (location == FnLoc::BIJECTION) {
            return {0.4,   0.42, 0.2, 0.1};
        } else if (location == FnLoc::UNSELECTED) {
            error("Internal error: Why are you asking where the 'unselected' label goes?");
        } else {
            error("Internal error: Unknown FnLoc value.");
        }
    }

    /* Bounding box for where individual function numbers should be drawn. */
    GRectangle answerBoxFor(FnLoc location) {
        auto box = labelBoxFor(location);
        return {
            box.x,
            box.y + box.height,
            box.width,
            0.08
        };
    }

    class FnVennGUI: public ProblemHandler {
    public:
        FnVennGUI(GWindow& window);

        void mouseMoved(double x, double y) override;
        void mousePressed(double x, double y) override;
        void mouseExited() override;

        void windowResized() override;
        bool shuttingDown() override;

    protected:
        void repaint() override;

    private:
        void drawLabelFor(FnLoc category, const string& label);
        void drawAnswers();

        Temporary<GContainer> panel;
        vector<GRadioButton*> buttons;

        GPoint     worldToGraphics(const GPoint& pt);
        GPoint     graphicsToWorld(const GPoint& pt);
        GRectangle worldToGraphics(const GRectangle& r);
        GRectangle graphicsToWorld(const GRectangle& r);

        /* List of all answers by region. */
        vector<FnLoc> answers;

        /* Dimensions of the master Venn diagram images. */
        GRectangle imageBounds;

        /* Bounding rectangle for the display. */
        GRectangle bounds;

        /* Scaling factor for the GImages that get displayed. */
        double imageScale;

        void recalculateGeometry();
        void initChrome();

        void loadAnswers();
        void saveAnswers();

        FnLoc hover = FnLoc::UNSELECTED;
    };

    FnVennGUI::FnVennGUI(GWindow& window): ProblemHandler(window) {
        GImage image("res/images/venn-map.png");
        imageBounds = image.getBounds();

        loadAnswers();
        initChrome();
        recalculateGeometry();
    }

    void FnVennGUI::initChrome() {
        static int radioButtonGroup = 0;

        GContainer* buttonPanel = new GContainer();
        for (size_t i = 0; i < kNumFunctions; i++) {
            buttons.push_back(new GRadioButton(to_string(i + 1), "buttonGroup" + to_string(radioButtonGroup)));
            buttonPanel->add(buttons.back());
            if (i + 1 != kNumFunctions) buttonPanel->add(new GLabel("|"));
        }

        GContainer* labelPanel = new GContainer();
        labelPanel->add(new GLabel("Select a function, then click to place it."));

        panel = make_temporary<GContainer>(window(), "SOUTH", GContainer::LAYOUT_FLOW_VERTICAL);
        panel->add(labelPanel);
        panel->add(buttonPanel);

        radioButtonGroup++;
    }

    void FnVennGUI::recalculateGeometry() {
        /* Get a clean copy of the Venn diagram map for measuring purposes. */
        const double aspectRatio = imageBounds.width / imageBounds.height;

        bounds = fitToBounds({ 0, 0, window().getCanvasWidth(), window().getCanvasHeight() }, aspectRatio);
        imageScale  = bounds.width / imageBounds.width;
    }

    void FnVennGUI::mouseMoved(double x, double y) {
        auto loc = graphicsToWorld(GPoint{x, y});
        hover = locationFor(loc.x, loc.y);
        requestRepaint();
    }

    void FnVennGUI::mouseExited() {
        hover = FnLoc::UNSELECTED;
        requestRepaint();
    }

    void FnVennGUI::mousePressed(double x, double y) {
        /* It's possible we've never moved the mouse; if that happens, send the
         * signal to move the mouse so that we get the right region.
         */
        mouseMoved(x, y);
        if (hover != FnLoc::UNSELECTED) {
            /* Figure out which item, if any, is selected. */
            for (size_t i = 0; i < buttons.size(); i++) {
                if (buttons[i]->isSelected()) {
                    answers[i] = hover;
                    requestRepaint();
                    return;
                }
            }
        }
    }

    void FnVennGUI::repaint() {
        clearDisplay(window(), kBackgroundColor);

        auto& image = imageFor(hover);
        image.resetTransform();
        image.scale(imageScale);

        /* TODO: Have to undo the transform to position x and y correctly due to
         * a bug in the positioning logic.
         */
        image.setLocation(bounds.x / imageScale, bounds.y / imageScale);
        window().draw(image);

        /* Draw a bounding rectangle to demarcate where the image stops. */
        window().setColor(kBoundsColor);
        window().drawRect(bounds);

        /* Label all sections except for "not a function," since that could give the
         * impression that "non-functions" includes "functions" as a subset.
         */
        drawLabelFor(FnLoc::FUNCTION,   "Functions");
        drawLabelFor(FnLoc::INJECTION,  "Injections");
        drawLabelFor(FnLoc::SURJECTION, "Surjections");
        drawLabelFor(FnLoc::BIJECTION,  "Bijections");

        drawAnswers();
    }

    void FnVennGUI::drawAnswers() {
        /* Map from category to a string label of what's there. */
        map<FnLoc, string> labels;
        for (size_t i = 0; i < answers.size(); i++) {
            if (labels[answers[i]].empty()) {
                labels[answers[i]] = to_string(i + 1);
            } else {
                labels[answers[i]] += ", " + to_string(i + 1);
            }
        }

        /* Draw everything but the unselected ones. */
        for (const auto& entry: labels) {
            if (entry.first != FnLoc::UNSELECTED) {
                auto text = TextRender::construct(entry.second, worldToGraphics(answerBoxFor(entry.first)), kAnswerFont);
                text->alignCenterHorizontally();
                text->draw(window());
            }
        }
    }

    void FnVennGUI::drawLabelFor(FnLoc loc, const string& label) {
        auto text = TextRender::construct(label, worldToGraphics(labelBoxFor(loc)), kLabelFont, LineBreak::NO_BREAK_SPACES);
        text->alignCenterHorizontally();
        text->draw(window());
    }

    GRectangle FnVennGUI::worldToGraphics(const GRectangle& r) {
        /* Slightly tricky. We have to first map from world coordinates
         * to image coordinates, then from image coordinates to
         * graphics coordinates.
         */
        return {
            r.x * imageBounds.width  * imageScale + bounds.x,
            r.y * imageBounds.height * imageScale + bounds.y,
            r.width  * imageBounds.width  * imageScale,
            r.height * imageBounds.height * imageScale
        };
    }

    GPoint FnVennGUI::graphicsToWorld(const GPoint& pt) {
        return {
            (pt.x - bounds.x) / (imageScale * imageBounds.width),
            (pt.y - bounds.y) / (imageScale * imageBounds.height)
        };
    }

    void FnVennGUI::windowResized() {
        recalculateGeometry();
        ProblemHandler::windowResized();
    }

    vector<FnLoc> loadAnswers() {
        ifstream input("res/PropertiesOfFunctions.answers");
        if (!input) error("Cannot open file res/PropertiesOfFunctions.answers.");

        JSON j = JSON::parse(input);

        vector<FnLoc> result;
        for (size_t i = 0; i < kNumFunctions; i++) {
            result.push_back(static_cast<FnLoc>(j["answers"][i].asInteger()));
        }

        return result;
    }

    void FnVennGUI::loadAnswers() {
        answers = ::loadAnswers();
    }

    void saveAnswers(const vector<FnLoc>& answers) {
        vector<JSON> jAnswers;
        for (size_t i = 0; i < answers.size(); i++) {
            jAnswers.push_back(JSON(static_cast<int>(answers[i])));
        }

        ofstream output("res/PropertiesOfFunctions.answers");
        output << JSON::object({
            {"", "DO NOT EDIT THIS FILE MANUALLY - USE THE PROGRAM TO ENTER YOUR ANSWERS" },
            {"answers", jAnswers}
        }) << flush;

        if (!output) error("Error saving your answers - please contact the course staff.");
    }

    void FnVennGUI::saveAnswers() {
        ::saveAnswers(answers);
    }

    bool FnVennGUI::shuttingDown() {
        saveAnswers();
        return ProblemHandler::shuttingDown();
    }
}

GRAPHICS_HANDLER("Properties of Functions", GWindow& window) {
    return make_shared<FnVennGUI>(window);
}

/* Console handler */

namespace {
    string nameFor(FnLoc location) {
        if (location == FnLoc::NOT_A_FUNCTION) {
            return "Not a Function";
        } else if (location == FnLoc::FUNCTION) {
            return "Function, Not Injective, Not Surjective";
        } else if (location == FnLoc::INJECTION) {
            return "Injection, but not a Surjection";
        } else if (location == FnLoc::SURJECTION) {
            return "Surjection, but not an Injection";
        } else if (location == FnLoc::BIJECTION) {
            return "Bijection";
        } else if (location == FnLoc::UNSELECTED) {
            return "(Not Yet Placed)";
        } else {
            error("Internal error: Unknown function type?");
        }
    }

    const vector<FnLoc> kAllLocations = {
        FnLoc::UNSELECTED,
        FnLoc::NOT_A_FUNCTION,
        FnLoc::FUNCTION,
        FnLoc::INJECTION,
        FnLoc::SURJECTION,
        FnLoc::BIJECTION
    };

    void userPlaceFunction(FnLoc& location) {
        cout << "Currently, this function is in this location: " << nameFor(location) << endl;

        vector<string> options;
        for (FnLoc loc: kAllLocations) {
            options.push_back(nameFor(loc));
        }

        location = static_cast<FnLoc>(makeSelectionFrom("Where should this item be placed?", options));
    }
}

CONSOLE_HANDLER("Functions Venn Diagram") {
    auto answers = loadAnswers();
    do {
        int part = getIntegerBetween("Enter the number of a function: ", 1, kNumFunctions);

        /* -1 because we're switching from a 1-indexed problem to a 0-indexed array. */
        userPlaceFunction(answers[part - 1]);
        saveAnswers(answers);
    } while (getYesOrNo("Place another function? "));
}
