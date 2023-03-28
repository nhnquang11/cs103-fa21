#include "../GUI/MiniGUI.h"
#include "Common.h"
#include "../Logic/WorldParser.h"
#include "../Logic/IntDynParser.h"
#include "../Logic/FOLParser.h"
#include "../Logic/FOLExpressionBuilder.h"
#include "../Logic/RealEntity.h"
#include "../GraphEditor/GraphViewer.h"
#include "../FileParser/FileParser.h"
using namespace MiniGUI;
using namespace std;

namespace {

    /* List of Roman Numeral answer styles, in sorted order. */
    const vector<string> kSortedNumerals = {
        "[Part (i)]",
        "[Part (ii)]",
        "[Part (iii)]",
        "[Part (iv)]",
        "[Part (v)]",
        "[Part (vi)]",
        "[Part (vii)]",
        "[Part (viii)]",
        "[Part (ix)]",
        "[Part (x)]",
        "[Part (xi)]",
        "[Part (xii)]",
        "[Part (xiii)]",
        "[Part (xiv)]",
        "[Part (xv)]",
        "[Part (xvi)]",
        // Add more entries here if needed!
    };

    /* All the formulas used in the Interpersonal Dynamics problem. */
    const vector<string> kIDFormulas = {
        "Loves(p1, p3)",
        "Loves(p3, p4)",
        "Loves(p1, p2) ∧ Loves(p2, p1)",
        "Loves(p1, p2) ∨ Loves(p2, p1)",
        "Loves(p1, p1) → Loves(p5, p5)",
        "Loves(p1, p2) → Loves(p4, p3)",
        "Loves(p1, p3) → Loves(p3, p6)",
        "Loves(p1, p4) → Loves(p4, p5)",
        "Loves(p1, p4) ↔ Loves(p2, p3)",
        "Loves(p1, p3) ↔ Loves(p5, p5)",
        "∀x. ∃y. Loves(x, y)",
        "∀x. ∃y. Loves(y, x)",
        "∀x. ∃y. (x ≠ y ∧ Loves(x, y))",
        "∀x. ∃y. (x ≠ y ∧ Loves(y, x))",
        "∃x. ∀y. Loves(x, y)",
        "∃x. ∀y. (x ≠ y → Loves(x, y))",
    };

    /* World description for Interpersonal Dynamics. */
    const string kIDWorld = R"(
                            Person(p1)
                            Person(p2)
                            Person(p3)
                            Person(p4)
                            Person(p5)
                            Person(p6)
                            Loves(p1, p1)
                            Loves(p1, p3)
                            Loves(p3, p1)
                            Loves(p3, p2)
                            Loves(p4, p3)
                            Loves(p5, p5)
                            )";

    /* List of edges in the world. */
    const vector<pair<string, string>> kIDEdges = {
        { "p1", "p1" },
        { "p1", "p3" },
        { "p3", "p1" },
        { "p3", "p2" },
        { "p4", "p3" },
        { "p5", "p5" }
    };

    /* Entities in this world. */
    const vector<string> kIDEntities = { "p1", "p2", "p3", "p4", "p5", "p6" };


    class IDGUI: public ProblemHandler {
    public:
        IDGUI(GWindow& window);

        void windowResized() override;
        void actionPerformed(GObservable* source) override;

    protected:
        void repaint() override;

    private:
        void updateBounds();
        void setIndex(size_t index);

        void initChrome();
        void initViewers();

        GRectangle viewerBounds();

        void drawErrorMessage();

        /* world
         * formula
         * [<] [>] */
        Temporary<GContainer> console;
          GLabel*  desc;
          GLabel*  formula;
          GButton* prev;
          GButton* next;

        /* Student answer consists of a pair of the the final world render and the
         * edges to highlight.
         */
        struct Answer {
            /* This can be null if an error occurred. */
            shared_ptr<GraphEditor::Viewer<>> viewer;

            /* Edge styles to highlight the changes. */
            unordered_map<GraphEditor::Edge*, GraphEditor::EdgeStyle> styles;

            /* Formula is true here? */
            bool isTrue;
        };

        vector<Answer> answers;
        size_t index = 0;
    };

    IDGUI::IDGUI(GWindow& window) : ProblemHandler(window) {
        initChrome();
        initViewers();

        updateBounds();
        setIndex(0);
    }

    void IDGUI::initChrome() {
        console = make_temporary<GContainer>(window(), "SOUTH", GContainer::LAYOUT_GRID);

        GContainer* text = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL);

        desc    = new GLabel("Oops! You aren't supposed to see this.");
        formula = new GLabel("Oops! You aren't supposed to see this.");

        text->add(desc);
        text->add(formula);

        GContainer* buttons = new GContainer();

        prev = new GButton("⏪");
        next = new GButton("⏩");

        buttons->add(prev);
        buttons->add(next);

        console->addToGrid(text,    0, 0);
        console->addToGrid(buttons, 1, 0);
    }

    void IDGUI::repaint() {
        clearDisplay(window(), "white");

        if (!answers[index].viewer) {
            drawErrorMessage();
        } else {
            answers[index].viewer->draw(window().getCanvas(), {}, answers[index].styles);
        }
    }

    const Font kErrorFont(FontFamily::SANS_SERIF, FontStyle::NORMAL, 18, "#800000");

    void IDGUI::drawErrorMessage() {
        auto render = TextRender::construct("Error parsing your answer.", viewerBounds(), kErrorFont);
        render->alignCenterHorizontally();
        render->alignCenterVertically();
        render->draw(window());
    }

    void IDGUI::windowResized() {
        updateBounds();
    }

    void IDGUI::updateBounds() {
        for (auto& answer: answers) {
            if (answer.viewer) answer.viewer->setBounds(viewerBounds());
        }
        requestRepaint();
    }

    GRectangle IDGUI::viewerBounds() {
        return { 0, 0, window().getCanvasWidth(), window().getCanvasHeight() };
    }

    void IDGUI::setIndex(size_t index) {
        this->index = index;
        desc->setText("Part (" + LogicUI::toRoman(index + 1) + ")");

        if (!answers[index].viewer) {
            formula->setText("");
        } else if (answers[index].isTrue) {
            formula->setText(kIDFormulas[index] + " evalutes to true.");
        } else {
            formula->setText("⚠ " + kIDFormulas[index] + " evalutes to false. ⚠");
        }

        requestRepaint();
    }

    void IDGUI::actionPerformed(GObservable* source) {
        if (source == next) {
            setIndex(index + 1 == answers.size()? 0 : index + 1);
        } else if (source == prev) {
            setIndex(index == 0? answers.size() - 1 : index - 1);
        }
    }

    /* The Interpersonal Dynamics world. */
    World idWorld() {
        /* Get the world, and build a context for it. */
        istringstream worldStream(kIDWorld);
        return parseWorld(worldStream);
    }

    /* The Interpersonal Dynamics build context. */
    FOL::BuildContext idContext(const World& world) {
        /* Start with the default. */
        auto result = entityBuildContext();

        /* Add in the people. */
        for (auto entity: world) {
            result.constants[entity->name] = entity;
        }

        return result;
    }

    /* Given a world, adds some more links into it. */
    World worldPlus(const World& original, const vector<pair<string, string>>& toAdd) {
        /* Serialize, add, deserialize. */
        stringstream builder;
        builder << original;

        for (const auto& entry: toAdd) {
            builder << "Loves(" << entry.first << ", " << entry.second << ")" << '\n';
        }

        return parseWorld(builder);
    }

    /* 1 x 1. */
    const double kAspectRatio = 1.0;

    /* Map from entities to their positions, as given in the handout. */
    const map<string, GPoint> kPositions = {
        { "p1", { 0.5, 0.2 } },
        { "p2", { 0.2, 0.5 } },
        { "p3", { 0.5, 0.5 } },
        { "p4", { 0.8, 0.5 } },
        { "p5", { 0.5, 0.8 } },
        { "p6", { 0.8, 0.8 } }
    };

    /* Make added edges really pop. */
    const string kAddedEdgeColor = "#E6A817"; // Harvest gold
    const double kAddedEdgeWidth = 3 * GraphEditor::kEdgeWidth;

    /* Loads the student answers and configures all the viewers. */
    void IDGUI::initViewers() {
        auto file = parseFile("res/Interpersonal.dynamics");

        for (size_t i = 0; i < kSortedNumerals.size(); i++) {
            try {
                /* Parse the answer. Use of .at will jump to exception handler if
                 * not present.
                 */
                auto added = IntDyn::parse(IntDyn::scan(*file.at(kSortedNumerals[i])));

                /* Translate the world into a viewer. */
                Answer answer;
                answer.viewer = make_shared<GraphEditor::Viewer<>>();
                answer.viewer->aspectRatio(kAspectRatio);

                /* Add nodes. */
                for (const auto& entry: kPositions) {
                    auto node = answer.viewer->newNode(entry.second);
                    node->label(entry.first);
                }

                /* Add base edges. */
                for (const auto& edge: kIDEdges) {
                    auto from = answer.viewer->nodeLabeled(edge.first);
                    auto to   = answer.viewer->nodeLabeled(edge.second);
                    answer.viewer->newEdge(from, to);
                }

                /* Add new edges. */
                for (const auto& edge: added) {
                    auto from = answer.viewer->nodeLabeled(edge.first);
                    auto to   = answer.viewer->nodeLabeled(edge.second);
                    auto e    = answer.viewer->newEdge(from, to);

                    GraphEditor::EdgeStyle style;
                    style.lineColor = kAddedEdgeColor;
                    style.lineWidth = kAddedEdgeWidth;
                    answer.styles[e] = style;
                }

                /* See if the formula is true here. */
                auto world   = worldPlus(idWorld(), added);
                auto context = idContext(world);

                answer.isTrue = FOL::buildExpressionFor(FOL::parse(FOL::scan(kIDFormulas[i])), context)->evaluate(world);

                answers.push_back(answer);
            } catch (...) {
                /* Insert a blank answer; this renders as an error. */
                answers.push_back(Answer());
            }
        }
    }

    /* Displays a world in plain-text. */
    void showWorld(const vector<pair<string, string>>& studentAdded) {
        cout << "These links already exist in the world: " << endl;
        for (auto love: kIDEdges) {
            cout << "  Loves(" << love.first << ", " << love.second << ")" << endl;
        }

        if (studentAdded.empty()) {
            cout << "You have not added any links because you believe the formula is already true here." << endl;
        } else {
            cout << "You have added these links: " << endl;
            for (auto love: studentAdded) {
                cout << "  Loves(" << love.first << ", " << love.second << ")" << endl;
            }
        }
    }

    void showCorrectness(int part, const vector<pair<string, string>>& studentAdded) {
        cout << "Formula: " << kIDFormulas.at(part - 1) << endl;

        /* Add student additions to world. */
        auto world = worldPlus(idWorld(), studentAdded);

        /* Build the formula. Subtract one from the part number; we're 1-indexed on
         * input and 0-indexed internally.
         */
        auto expr = FOL::buildExpressionFor(FOL::parse(FOL::scan(kIDFormulas.at(part - 1))), idContext(world));
        if (expr->evaluate(world)) {
            cout << "Formula is true for this world." << endl;
        } else {
            cerr << "Formula is not true for this world." << endl;
        }
    }
}

GRAPHICS_HANDLER("Int. Dynamics", GWindow& window) {
    return make_shared<IDGUI>(window);
}

CONSOLE_HANDLER("Interpersonal Dynamics") {
    do {
        int part = LogicUI::getIntegerRoman("Enter part: ", 1, 16);

        try {
            auto studentAdded = IntDyn::parse(IntDyn::scan(*parseFile("res/Interpersonal.dynamics").at("[Part (" + LogicUI::toRoman(part) + ")]")));
            showWorld(studentAdded);
            showCorrectness(part, studentAdded);
        } catch (const exception& e) {
            cerr << "An error occurred parsing your answer to that part." << endl;
        }
    } while (getYesOrNo("See another world? "));
}
