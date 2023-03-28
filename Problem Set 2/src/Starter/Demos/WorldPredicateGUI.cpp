#include "WorldPredicateGUI.h"
#include "Common.h"
#include "../GUI/MiniGUI.h"
#include "../Logic/RealEntity.h"
#include "../Logic/FOLExpressionBuilder.h"
#include "../Logic/FOLParser.h"
#include "../FileParser/FileParser.h"
#include "../Logic/WorldParser.h"
#include <functional>
#include <vector>
#include <map>
using namespace std;
using namespace MiniGUI;

namespace {
    /* Fraction of the window for predicate names. */
    const double kPredicateWidth  = 0.25;
    const double kPredicateHeight = 0.6;
    const double kTitleHeight     = 0.1;
    const double kDescHeight      = 0.3;

    /* Result of evaluating a predicate on a world. */
    enum class Result {
        TRUE,
        FALSE,
        ERROR
    };

    /* Map from results to colors. */
    const map<Result, string> kColors = {
        { Result::TRUE,  "black"   },
        { Result::FALSE, "black"   },
        { Result::ERROR, "#7C0A02" }, // Barn red
    };

    /* Legend Parameters. */
    const Font kLegendFont(FontFamily::MONOSPACE, FontStyle::NORMAL, 18, "black");
    const string kLegendBorderColor = "black";

    /* Title / label parameters. */
    const Font kTitleFont(FontFamily::SERIF, FontStyle::BOLD, 24, "black");
    const Font kDescFont (FontFamily::SERIF, FontStyle::NORMAL, 18, "black");

    const Font kErrorFont(FontFamily::SANS_SERIF, FontStyle::NORMAL, 18, "#800000");

    Result evaluate(const Predicate& p, const World& world) {
        try {
            return p.pred(world)? Result::TRUE : Result::FALSE;
        } catch (...) {
            return Result::ERROR;
        }
    }

    string to_string(Result r) {
        if      (r == Result::TRUE)  return "true";
        else if (r == Result::FALSE) return "false";
        else if (r == Result::ERROR) return "error";
        else abort(); // Logic error!
    }
}

WorldPredicateGUI::WorldPredicateGUI(GWindow& window,
                                     const vector<PredicatedWorld>& predicatedWorlds,
                                     const string& title,
                                     const string& description) : ProblemHandler(window), title(title), description(description) {
    /* Build the viewer list from the list of worlds found in the file. */
    for (auto entry: predicatedWorlds) {
        viewers.push_back(make_shared<WorldViewer>(entry));
    }

    console = make_temporary<GContainer>(window, "SOUTH", GContainer::LAYOUT_GRID);
    desc = new GLabel("Oops! You aren't supposed to see this.");

    buttons = new GContainer();

    prev = new GButton("⏪");
    next = new GButton("⏩");

    buttons->add(prev);
    buttons->add(next);

    console->addToGrid(desc, 0, 0);
    console->addToGrid(buttons, 1, 0);

    updateBounds();
    setIndex(0);
}

void WorldPredicateGUI::repaint() {
    clearDisplay(window(), "white");
    drawTitle();
    drawDesc();

    if (viewers[index]->isError()) {
        drawErrorMessage(viewers[index]->errorMessage());
    } else {
        viewers[index]->draw(window().getCanvas());
        drawPredicates();
    }
}

void WorldPredicateGUI::drawErrorMessage(const string& message) {
    auto render = TextRender::construct("Error loading this world:\n\n" + message, viewerBounds(), kErrorFont);
    render->alignCenterHorizontally();
    render->alignCenterVertically();
    render->draw(window());
}

void WorldPredicateGUI::windowResized() {
    updateBounds();
}

void WorldPredicateGUI::updateBounds() {
    for (auto viewer: viewers) {
        viewer->setBounds(viewerBounds());
    }
    requestRepaint();
}

GRectangle WorldPredicateGUI::viewerBounds() {
    double width = window().getCanvasWidth();
    return { width * kPredicateWidth, 0, width * (1 - kPredicateWidth), window().getCanvasHeight() };
}

GRectangle WorldPredicateGUI::predicateBounds() {
    double width  = window().getCanvasWidth();
    double height = window().getCanvasHeight();
    return { 0, height * (1 - kPredicateHeight), width * kPredicateWidth, height * kPredicateHeight };
}

GRectangle WorldPredicateGUI::titleBounds() {
    double width  = window().getCanvasWidth();
    double height = window().getCanvasHeight();
    return { 0, 0, width * kPredicateWidth, height * kTitleHeight };
}

GRectangle WorldPredicateGUI::descBounds() {
    double width  = window().getCanvasWidth();
    double height = window().getCanvasHeight();
    return { 0, height * kTitleHeight, width * kPredicateWidth, height * kDescHeight };
}

void WorldPredicateGUI::setIndex(size_t index) {
    this->index = index;
    desc->setText(viewers[index]->name());
    requestRepaint();
}

void WorldPredicateGUI::actionPerformed(GObservable* source) {
    if (source == next) {
        setIndex(index + 1 == viewers.size()? 0 : index + 1);
    } else if (source == prev) {
        setIndex(index == 0? viewers.size() - 1 : index - 1);
    }
}

void WorldPredicateGUI::drawPredicates() {
    vector<string> labels;
    vector<string> colors;

    auto world = viewers[index]->world();
    auto preds = viewers[index]->predicates();

    /* Align all strings the same way to match the length of the longest
     * name.
     */
    size_t maxLen = max_element(preds.begin(), preds.end(), [](const Predicate& lhs, const Predicate& rhs) {
        return lhs.name.length() < rhs.name.length();
    })->name.length();

    for (const auto& p: preds) {
        auto result = evaluate(p, world);

        ostringstream label;
        label << left << setw(maxLen + 2) << (p.name + ": ");
        label << setw(5) << to_string(result); // 5 = length of false/error
        labels.push_back(label.str());
        colors.push_back(kColors.at(result));
    }

    LegendRender::construct(labels, colors,
                            predicateBounds(),
                            colors,
                            kLegendFont, kLegendBorderColor,
                            LineBreak::NO_BREAK_SPACES)->draw(window());
}

void WorldPredicateGUI::drawTitle() {
    auto text = TextRender::construct(title, titleBounds(), kTitleFont, LineBreak::NO_BREAK_SPACES);
    if (text) {
        text->draw(window());
    }
}

void WorldPredicateGUI::drawDesc() {
    auto text = TextRender::construct(description, descBounds(), kDescFont);
    if (text) {
        text->draw(window());
    }
}

function<bool(const World& world)> WorldPredicateGUI::parse(istream& in, const FOL::BuildContext& context) {
    try {
        auto expr = FOL::buildExpressionFor(FOL::parse(FOL::scan(in)), context);
        return [=](const World& world) {
            return expr->evaluate(world);
        };
    } catch (...) {
        return [](const World &) -> bool {
            throw runtime_error("Parse error.");
        };
    }
}

function<bool(const World& world)> WorldPredicateGUI::parse(const string& formula, FOL::BuildContext context) {
    istringstream input(formula);
    return parse(input, context);
}

namespace {
    void printPredicatedWorld(const shared_ptr<WorldViewer>& world) {
        cout << "Sample world: " << world->name() << endl;

        if (world->isError()) {
            cerr << "An error occurred parsing this world." << endl;
        } else {
            cout << "Here's a description of the world: " << endl;
            cout << world->world() << endl;

            cout << "Here are the results of the predicates:" << endl;
            for (const auto& predicate: world->predicates()) {
                cout << predicate.name << ": " << to_string(evaluate(predicate, world->world())) << endl;
            }
        }
    }
}

void WorldPredicateGUI::doConsole(const vector<PredicatedWorld>& worlds,
                                  const string& title,
                                  const string& whatsBeingShown) {
    cout << title << endl;

    vector<string> options;
    for (const auto& entry: worlds) {
        options.push_back(entry.name);
    }

    vector<shared_ptr<WorldViewer>> viewers;
    for (const auto& entry: worlds) {
        viewers.push_back(make_shared<WorldViewer>(entry));
    }

    do {
        cout << whatsBeingShown << endl;
        int choice = makeSelectionFrom("Choose a world: ", options);

        printPredicatedWorld(viewers[choice]);
    } while (getYesOrNo("See another world?" ));
}
