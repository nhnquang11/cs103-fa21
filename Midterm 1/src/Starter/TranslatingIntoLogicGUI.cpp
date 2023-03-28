#include "FileParser/FileParser.h"
#include "Logic/FOLExpressionBuilder.h"
#include "Logic/FOLParser.h"
#include "Logic/FOLExpression.h"
#include "Logic/RealEntity.h"
#include "GUI/MiniGUI.h"
#include "Demos/Common.h"
#include <fstream>
#include <cstdlib>
using namespace std;

namespace {
    const int kNumParts = 3;

    string partToRoman(int part) {
        return "[Part (" + LogicUI::toRoman(part) + ")]";
    }

    /* Throws an exception if the answer can't be loaded. */
    void displayAnswerTo(ostream& out, const string& part) {
        auto file = parseFile("res/TranslatingIntoLogic.fol");
        if (!file.count(part)) error("No section named " + part + " was found in res/TranslatingIntoLogic.fol");

        auto expr = FOL::buildExpressionFor(FOL::parse(FOL::scan(*file.at(part))), entityBuildContext());

        out << "Your answer for " << part << ", as understood by our parser: " << '\n';
        out << *expr << endl;
    }
}

CONSOLE_HANDLER("Translating Into Logic") {
    do {
        int part = LogicUI::getIntegerRoman("Which subproblem do you want to see? ", 1, kNumParts);

        try {
            displayAnswerTo(cout, partToRoman(part));
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    } while (getYesOrNo("See another formula? "));
}

namespace {
    class TILGUI: public ProblemHandler {
    public:
        TILGUI(GWindow& window);

        void actionPerformed(GObservable* source);

    private:
        vector<Temporary<GButton>> options;
        Temporary<GColorConsole> console;
    };

    TILGUI::TILGUI(GWindow& window): ProblemHandler(window) {
        console = make_temporary<GColorConsole>(window, "CENTER");

        for (int part = 1; part <= kNumParts; part++) {
            options.push_back(make_temporary<GButton>(window, "SOUTH", partToRoman(part)));
        }

        *console << "Choose an option to display how our parser understands it." << endl;
    }

    void TILGUI::actionPerformed(GObservable* source) {
        for (auto& option: options) {
            if (option == source) {
                console->clearDisplay();
                try {
                    displayAnswerTo(*console, option->getText());
                } catch (const exception& e) {
                    console->doWithStyle("#800000", GColorConsole::BOLD, [&] {
                        *console << "Error: " << e.what() << endl;
                    });
                }
            }
        }
    }
}

GRAPHICS_HANDLER("Translating Into Logic", GWindow& window) {
    return make_shared<TILGUI>(window);
}
