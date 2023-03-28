#include "TruthTableViewer.h"
#include "../GUI/MiniGUI.h"
#include "../FileParser/FileParser.h"
#include "../Logic/PLExpression.h"
#include "../Logic/PLParser.h"
#include "../Utilities/Unicode.h"
#include "Common.h"
#include "gthread.h"
using namespace std;

namespace {
    FormulaOrError loadFormula(istream& in, const string& section) {
        try {
            return { PL::parse(PL::scan(in)), "", section };
        } catch (const exception& e) {
            return { nullptr, e.what(), section };
        }
    }

    /* Converts a PL formula to a string. */
    string toString(shared_ptr<PL::Expression> expr) {
        ostringstream result;
        result << *expr;
        return result.str();
    }

    /* Given a UTF-8 string, returns the number of Unicode characters in it. */
    size_t unicodeLength(const string& str) {
        size_t result = 0;
        for (char32_t unused: utf8Reader(str)) {
            (void) unused;
            result++;
        }
        return result;
    }

    /* Centers a boolean inside of a string of the given length. */
    string centered(bool value, size_t length) {
        string result(length, ' ');
        result[result.size() / 2] = (value? 'T' : 'F');
        return result;
    }

    void printTruthTableFor(ostream& out, PL::Formula expr, const string& prefix) {
        string formula = toString(expr);

        /* Begin by listing the variables in some predicable order. */
        vector<string> variables;
        for (const auto& var: variablesIn(expr)) {
            variables.push_back(var);
        }

        /* Print the variables in that order. */
        out << prefix;
        for (string var: variables) {
            out << var << ' ';
        }
        out << formula << endl;

        for (const auto& entry: truthTableFor(expr)) {
            /* Print the values of each variable. */
            out << prefix;
            for (size_t i = 0; i < entry.first.size(); i++) {
                out << centered(entry.first[i], unicodeLength(variables[i])) << ' ';
            }

            /* And the result! */
            out << centered(entry.second, unicodeLength(formula)) << endl;
        }
    }

    /* Loads formulas, returning them in sorted order. */
    vector<FormulaOrError> loadFormulas(const string& filename) {
        auto contents = parseFile(filename);

        /* Load the files. */
        vector<FormulaOrError> result;
        for (const auto& entry: contents) {
            result.push_back(loadFormula(*entry.second, entry.first));
        }

        /* Sort them. */
        sort(result.begin(), result.end(), [](const FormulaOrError& lhs, const FormulaOrError& rhs) {
            return LogicUI::compareRoman(lhs.section, rhs.section);
        });
        return result;
    }

    /* Janky code to determine the maximum index present in a list of formulas. */
    int highestIndexIn(const vector<FormulaOrError>& formulas) {
        if (formulas.empty()) error("No formulas were found.");

        /* Count upward until we hit the back one. */
        int result = 1;
        while (true) {
            if (formulas.back().section == "[Part (" + LogicUI::toRoman(result) + ")]") {
                return result;
            }

            /* Too big; this can't happen. */
            if (result == 100) {
                error("Malformed data file.");
            }

            result++;
        }
    }

    /* We need to use a special printing format for the truth table on the console so that the
     * formula is easy to use on screen readers.
     */
    void showConsoleTruthTableFor(shared_ptr<PL::Expression> expr) {
        /* Linearize the variables. */
        auto variableSet = PL::variablesIn(expr);
        vector<string> vars(variableSet.begin(), variableSet.end());

        cout << "Your formula: " << *expr << endl;
        cout << "Its truth table: " << endl;

        /* Print the truth table. */
        for (const auto& row: PL::truthTableFor(expr)) {
            cout << "  ";
            for (size_t i = 0; i < row.first.size(); i++) {
                cout << vars[i] << "=" << (row.first[i]? "T" : "F") << " ";
            }
            cout << "Formula: " << (row.second? "T" : "F") << endl;
        }
    }
}

TruthTableViewer::TruthTableViewer(GWindow& window,
                                   const string& problemName,
                                   const string& filename) : ProblemHandler(window), problemName(problemName) {
    auto entries = loadFormulas(filename);
    for (const auto& entry: entries) {
        formulas[entry.section] = entry;
    }

    /* Build the chooser. */
    chooser = make_temporary<GComboBox>(window, "SOUTH");
    for (const auto& entry: entries) {
        chooser->addItem(entry.section);
    }
    chooser->setEditable(false);

    /* Set up the central console. */
    console = make_temporary<GColorConsole>(window, "CENTER");

    updateDisplay(selectedItem());
}

void TruthTableViewer::updateDisplay(FormulaOrError f) {
    GThread::runOnQtGuiThread([&] {
        console->clearDisplay();
        console->doWithStyle(FontSize(16), [&] {
            *console << problemName << ": " << f.section << endl << endl;
        });

        if (f.formula) {
            console->doWithStyle("#000088", FontSize(14), [&] {
                *console << "Your formula: " << *f.formula << endl << endl;
                *console << "Truth table: " << endl << endl;
            });
            console->doWithStyle(FontSize(14), [&] {
                printTruthTableFor(*console, f.formula, "  ");
            });
        } else {
            console->doWithStyle("#800000", GColorConsole::BOLD_ITALIC, [&] {
                *console << "  Error loading formula: " << f.section << endl;
            });
        }
    });
}

void TruthTableViewer::changeOccurredIn(GObservable* source) {
    if (source == chooser) {
        updateDisplay(selectedItem());
    }
}

FormulaOrError TruthTableViewer::selectedItem() {
    return formulas.at(chooser->getSelectedItem());
}

void TruthTableViewer::doConsole(const string& problemName, const string& filename) {
    cout << problemName << endl;

    /* Load the formulas. Infer the range of possible values. */
    auto formulas = loadFormulas(filename);
    int maxIndex = highestIndexIn(formulas);

    do {
        int part = LogicUI::getIntegerRoman("Which subproblem do you want to see? ", 1, maxIndex);

        /* Find that part. */
        auto itr = find_if(formulas.begin(), formulas.end(), [&](const FormulaOrError& f) {
            return f.section == "[Part (" + LogicUI::toRoman(part) + ")]";
        });

        if (itr != formulas.end()) {
            if (itr->formula) {
                showConsoleTruthTableFor(itr->formula);
            } else {
                cerr << "Error reading your formula: " << itr->error << endl;
            }
        } else {
            cerr << "That subproblem is not present in " + filename + ", though it should be.";
        }
    } while (getYesOrNo("See another truth table? "));
}
