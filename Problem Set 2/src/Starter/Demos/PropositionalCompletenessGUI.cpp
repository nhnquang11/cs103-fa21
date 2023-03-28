#include "TruthTableViewer.h"

GRAPHICS_HANDLER("Prop. Completeness", GWindow& window) {
    return std::make_shared<TruthTableViewer>(window, "Propositional Completeness", "res/PropositionalCompleteness.proplogic");
}

CONSOLE_HANDLER("Propositional Completeness") {
    TruthTableViewer::doConsole("Propositional Completeness", "res/PropositionalCompleteness.proplogic");
}
