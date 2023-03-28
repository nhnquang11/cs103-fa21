#include "WorldPredicateGUI.h"
#include "../FileParser/FileParser.h"
#include "../Logic/FOLParser.h"
using namespace std;

namespace {
    vector<PredicatedWorld> worlds() {
        auto file = parseFile("res/ThisButNotThat.worlds");
        return {
            {
                {
                    { "This", WorldPredicateGUI::parse("∀y. ∃x. Loves(x, y)") },
                    { "That", WorldPredicateGUI::parse("∃x. ∀y. Loves(x, y)") },
                }, file["[Part (i)]"], "[Part (i)]"
            },
            {
                {
                    { "This", WorldPredicateGUI::parse("∀x. (Person(x) ∨ Cat(x))") },
                    { "That", WorldPredicateGUI::parse("(∀x. Person(x)) ∨ (∀x. Cat(x))") },
                }, file["[Part (ii)]"], "[Part (ii)]"
            },
            {
                {
                    { "This", WorldPredicateGUI::parse("(∃x. Robot(x)) ∧ (∃x. Loves(x, x))") },
                    { "That", WorldPredicateGUI::parse("∃x. (Robot(x) ∧ Loves(x, x))") },
                }, file["[Part (iii)]"], "[Part (iii)]"
            },
            {
                {
                    { "This", WorldPredicateGUI::parse("(∀x. Cat(x)) → (∀y. Loves(y, y))") },
                    { "That", WorldPredicateGUI::parse("∀x. ∀y. (Cat(x) → Loves(y, y))") },
                }, file["[Part (iv)]"], "[Part (iv)]"
            },
            {
                {
                    { "This", WorldPredicateGUI::parse("∃x. (Robot(x) → ∀y. Robot(y))") },
                    { "That", WorldPredicateGUI::parse("(∀x. Robot(x)) ∨ (∀x. ¬Robot(x))") },
                }, file["[Part (v)]"], "[Part (v)]"
            },
        };
    }

}

GRAPHICS_HANDLER("This, But Not That", GWindow& window) {
    return make_shared<WorldPredicateGUI>(window, worlds(),
                                          "This, But Not That",
                                          "Below are the formulas from \"This, but not That\" and how they evaluate in the worlds you've given as your answers.");
}

CONSOLE_HANDLER("This, But Not That") {
    WorldPredicateGUI::doConsole(worlds(), "This, But Not That", "Below are your sample worlds. Choose one to see how the \"This, But Not That\" formulas behave on them.");
}
