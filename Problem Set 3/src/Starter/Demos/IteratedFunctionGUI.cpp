#include "../GUI/MiniGUI.h"
#include "../../IteratedFunctions.h"
#include <vector>
#include <functional>
#include <iomanip>
using namespace std;

namespace {
    struct Demo {
        string name;
        function<double(double)> fn;
    };

    const vector<Demo> kDemos = {
        { "cos100",    cos100    },
        { "magic2_00", magic2_00 },
        { "magic2_75", magic2_75 },
        { "magic3_25", magic3_25 },
        { "magic3_50", magic3_50 },
        { "magic3_75", magic3_75 },
        { "magic3_99", magic3_99 }
    };

    const int kNumSteps = 20;

    void displayValues(ostream& out, const Demo& demo) {
        try {
            vector<pair<double, double>> values;
            for (int i = 1; i < kNumSteps; i++) {
                double value = 1.0 * i / kNumSteps;
                values.push_back(make_pair(value, demo.fn(value)));
            }

            for (const auto& entry: values) {
                out << demo.name << "(" << fixed << setprecision(6) << left << entry.first << ") = " << entry.second << '\n';
            }
            out << flush;
        } catch (const exception& e) {
            out << "Error: " << e.what() << endl;
        }
    }

    class IterateGUI: public ProblemHandler {
    public:
        IterateGUI(GWindow& window);
        void actionPerformed(GObservable* source) override;

    private:
        Temporary<GColorConsole> console;
        vector<Temporary<GButton>> buttons;
        map<GObservable*, Demo> functions;
    };

    IterateGUI::IterateGUI(GWindow& window) : ProblemHandler(window) {
        console = make_temporary<GColorConsole>(window, "CENTER");

        for (const auto& demo: kDemos) {
            buttons.push_back(make_temporary<GButton>(window, "SOUTH", demo.name));
            functions[buttons.back().get()] = demo;
        }
    }

    void IterateGUI::actionPerformed(GObservable* source) {
        if (functions.count(source)) {
            console->clearDisplay();
            displayValues(*console, functions[source]);
        }
    }
}

CONSOLE_HANDLER("Iterated Functions") {
    /* Make a list of all available options. */
    vector<string> options;
    for (const auto& demo: kDemos) {
        options.push_back(demo.name);
    }

    do {
        int choice = makeSelectionFrom("Pick a function to explore.", options);
        displayValues(cout, kDemos[choice]);
    } while (getYesOrNo("See another function? "));
}

GRAPHICS_HANDLER("Iterated Functions", GWindow& window) {
    return make_shared<IterateGUI>(window);
}
