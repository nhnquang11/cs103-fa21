#include "WorldPredicateGUI.h"
#include "../FileParser/FileParser.h"
#include "Common.h"
using namespace std;

namespace {
    vector<PredicatedWorld> worlds() {
        /* Sort alphabetically. */
        auto file = parseFile("res/TranslatingIntoLogic.fol");
        vector<pair<string, shared_ptr<istream>>> entries(file.begin(), file.end());
        sort(entries.begin(), entries.end(), [](const pair<string, shared_ptr<istream>>& lhs, const pair<string, shared_ptr<istream>>& rhs) {
            return LogicUI::compareRoman(lhs.first, rhs.first);
        });

        /* Build list of predicates. */
        vector<Predicate> preds;
        for (auto entry: entries) {
            preds.push_back({ trim(entry.first), WorldPredicateGUI::parse(*entry.second) });
        }

        vector<PredicatedWorld> pws;
        for (auto entry: parseFile("res/SampleWorlds.worlds")) {
            pws.push_back({ preds, entry.second, entry.first });
        }

        return pws;
    }
}

GRAPHICS_HANDLER("Translating Into Logic", GWindow& window) {
    return make_shared<WorldPredicateGUI>(window, worlds(),
                                          "Translating into Logic",
                                          "Here are some sample worlds, along with how your formulas evaluate in each of those worlds.");
}

CONSOLE_HANDLER("Translating Into Logic") {
    WorldPredicateGUI::doConsole(worlds(), "Translating Into Logic", "Here are some sample worlds you can evaluate your formulas on to see how they behave.");
}
