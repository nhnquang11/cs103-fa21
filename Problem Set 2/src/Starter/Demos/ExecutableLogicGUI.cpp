#include "WorldPredicateGUI.h"
#include "../../ExecutableLogic.h"
#include "../FileParser/FileParser.h"
using namespace std;

namespace {
    vector<PredicatedWorld> makeWorlds() {
        vector<Predicate> preds = {
            { "Part (i)",   isFormulaTrueFor_partI    },
            { "Part (ii)",  isFormulaTrueFor_partII   },
            { "Part (iii)", isFormulaTrueFor_partIII  },
            { "Part (iv)",  isFormulaTrueFor_partIV   },
            { "Part (v)",   isFormulaTrueFor_partV    },
            { "Part (vi)",  isFormulaTrueFor_partVI   },
        };
        auto file = parseFile("res/SampleWorlds.worlds");

        vector<PredicatedWorld> pws;
        for (auto entry: file) {
            pws.push_back({ preds, entry.second, entry.first });
        }

        return pws;
    }
}

GRAPHICS_HANDLER("Executable Logic", GWindow& window) {
    return make_shared<WorldPredicateGUI>(window, makeWorlds(),
                                          "Executable Logic",
                                          "Here are the values returned by your functions in ExecutableLogic.cpp on the sample world shown here.");
}

CONSOLE_HANDLER("Executable Logic") {
    WorldPredicateGUI::doConsole(makeWorlds(), "Executable Logic", "Here are some sample worlds you can run your functions on to see how they behave.");
}
