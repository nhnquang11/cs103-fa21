#if 0
#include "../ExecutableLogic.h"
#include "Logic/FOLExpressionBuilder.h"
#include "Logic/FOLParser.h"
#include "Logic/PLParser.h"
#include "Logic/PLExpression.h"
#include "FileParser/FileParser.h"
#include "Logic/WorldParser.h"
#include "Logic/RealEntity.h"
#include "Utilities/Unicode.h"
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <iomanip>
#include "console.h"
#include "simpio.h"
#include "vector.h"
#include "filelib.h"
#include "strlib.h"
#include "set.h"
using namespace std;

namespace MenuDriver {
    /* Type: MenuOption
     *
     * Type representing an option that can be selected from the menu, along with
     * the code to run when that option is selected.
     */
    struct MenuOption {
        string displayName;
        function<void()> demoFunction;
    };

    #define CALLBACK_FUNCTION(name) [] { void name(); name(); }

    /* List of all menu options. */
    const Vector<MenuOption> kMenuOptions = {
        { "Propositional Completeness",         CALLBACK_FUNCTION(propositionalCompleteness) },
        { "Set Theory and Propositional Logic", CALLBACK_FUNCTION(setTheoryAndPL)            },
        { "Executable Logic" ,                  CALLBACK_FUNCTION(testCode)                  },
        { "First-Order Negations",              CALLBACK_FUNCTION(firstOrderNegations)       },
        { "This, But Not That",                 CALLBACK_FUNCTION(thisButNotThat)            },
        { "Translating Into Logic",             CALLBACK_FUNCTION(translatingIntoLogic)      },
    };

    int makeSelectionFrom(const string& title, const Vector<string>& options) {
        cout << title << endl;

        for (int i = 0; i < options.size(); i++) {
            cout << i << " " <<  options[i] << endl;
        }

        while (true) {
            int result = getInteger("Your choice: ");
            if (result >= 0 && result < options.size()) return result;

            cout << "Please enter a number between 0 and " << options.size() - 1 << endl;
        }
    }

    int makeMenuSelection() {
        Vector<string> options;
        for (const auto& menuOption: kMenuOptions) {
            options += menuOption.displayName;
        }

        return makeSelectionFrom("Please make a selection:", options);
    }

    /* Lists all files with the given suffix and asks the user to pick one. */
    string makeFileSelection(const string& suffix) {
        Vector<string> options;
        for (string file: listDirectory(".")) {
            if (endsWith(file, suffix)) {
                options += file;
            }
        }

        return options[makeSelectionFrom("Please choose a demo file from this list:", options)];
    }

    void run() {
        do {
            int selection = makeMenuSelection();
            kMenuOptions[selection].demoFunction();

            cout << endl;
        } while (getYesOrNo("You are back at the main menu. Would you like to pick again?"));
        cout << endl;
        cout << "Exiting..." << endl;
    }

    /***** Assignment-Specific Code Here *****/
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
        // Add more entries here if needed!
    };

    /* Prints all PL formulas from a given file. */
    void printPLFormulas(const string& filename) {
        auto data = parseFile(filename);
        cout << "Propositional Logic Formulas from " << filename << ": " << endl;

        for (const auto& number: kSortedNumerals) {
            if (data.count(number)) {
                cout << "  " << number << endl;

                try {
                    auto formula = PL::parse(PL::scan(*data[number]));
                    cout << "    " << *formula << endl;
                } catch (const exception& e) {
                    cerr << "    Error: " << e.what() << endl;
                }

                cout << endl;
            }
        }
    }

    /* Prints all FOL formulas from a given file. */
    void printFOLFormulas(const string& filename) {
        auto data = parseFile(filename);
        cout << "Here are the formulas that you currently have entered into the file" << endl;
        cout << filename << ":" << endl;

        for (const auto& number: kSortedNumerals) {
            if (data.count(number)) {
                cout << "  " << number << endl;

                try {
                    auto formula = FOL::buildExpressionFor(FOL::parse(FOL::scan(*data[number])), entityBuildContext());
                    cout << "    " << *formula << endl;
                } catch (const exception& e) {
                    cerr << "    Error: " << e.what() << endl;
                }

                cout << endl;
            }
        }
    }

    /* Prints a world, indented by the specified indentation. */
    void printWorldIndented(const string& indent, const World& world) {
        stringstream converter;
        converter << world;

        for (string line; getline(converter, line); ) {
            cout << indent << line << endl;
        }
    }

    /* Prints all worlds from a given file. */
    void printWorlds(const string& filename) {
        auto data = parseFile(filename);
        cout << "Worlds from " << filename << ": " << endl;

        for (const auto& number: kSortedNumerals) {
            if (data.count(number)) {
                cout << "  " << number << endl;

                try {
                    printWorldIndented("    ", parseWorld(*data[number]));
                } catch (const exception& e) {
                    cerr << "    Error: " << e.what() << endl;
                }

                cout << endl;
            }
        }
    }

    /* Parses all answers entered by the student, reporting how they were interpreted. */
    void printAnswers() {
        printPLFormulas("res/PropositionalCompleteness.proplogic");
        printPLFormulas("res/SetTheory.proplogic");
        printWorlds("res/ThisButNotThat.worlds");
        printFOLFormulas("res/TranslatingIntoLogic.fol");
    }

    /* List of test functions. */
    using WorldPredicate = function<bool(std::set<Entity>)>;
    const vector<tuple<string, string, WorldPredicate>> kFOLFunctions = {
        make_tuple("[Part (i)]"  , "∃x. Cat(x)",                                    isFormulaTrueFor_partI  ),
        make_tuple("[Part (ii)]" , "∀x. Robot(x)",                                  isFormulaTrueFor_partII ),
        make_tuple("[Part (iii)]", "∃x. (Person(x) ∧ Loves(x, x))",                 isFormulaTrueFor_partIII),
        make_tuple("[Part (iv)]" , "∀x. (Cat(x) → Loves(x, x))",                    isFormulaTrueFor_partIV ),
        make_tuple("[Part (v)]"  , "∀x. (Cat(x) → ∃y. (Person(y) ∧ ¬Loves(x, y)))", isFormulaTrueFor_partV  ),
        make_tuple("[Part (vi)]" , "∃x. (Robot(x) ↔  ∀y. (Loves(x, y)))",           isFormulaTrueFor_partVI ),
    };

    void testCode() {
        cout << "We're going to test your C++ functions on the sample worlds from" << endl;
        cout << "SampleWorlds.worlds and compare the answers produced against what" << endl;
        cout << "the first-order logic formula says." << endl;
        cout << endl;

        auto worlds = parseFile("res/SampleWorlds.worlds");

        for (const auto& entry: worlds) {
            cout << entry.first << endl;
            cout << "  World: " << endl;
            auto world = parseWorld(*entry.second);

            printWorldIndented("    ", world);
            cout << endl;

            cout << "  Your Results:" << endl;
            for (const auto& fol: kFOLFunctions) {
                cout << "    " << left << setw(12) << get<0>(fol) << ": ";

                bool student;
                try {
                    /* Run the student code. */
                    student = get<2>(fol)(world);

                } catch (const exception& e) {
                    cerr << "(code triggered an exception)" << endl;
                    continue;
                }

                /* Compare against the actual formula. */
                auto formula = FOL::buildExpressionFor(FOL::parse(FOL::scan(get<1>(fol))), entityBuildContext());
                bool correct = formula->evaluate(world);

                cout << boolalpha << setw(5) << student;

                if (student != correct) {
                    cerr << " (incorrect)";
                }

                cout << endl;
            }
            cout << endl;
            (void) getLine("Press ENTER to continue...");
        }
    }

    Set<string> variablesIn(shared_ptr<PL::Expression> expr) {
        class VariableWalker: public PL::ExpressionTreeWalker {
        public:
          Set<string> result;

          void handle(const PL::VariableExpression& e) {
            result.insert(e.variableName());
          }
        };

        VariableWalker walker;
        expr->accept(&walker);
        return walker.result;
      }

    /* Given one variable assignment, advances to the next. If this isn't possible,
     * this returns false.
     */
    bool nextAssignment(Vector<bool>& assignment) {
        /* Walk backwards to the next false slot. */
        for (int index = assignment.size(); index > 0; index--) {
            if (!assignment[index - 1]) {
                assignment[index - 1] = true;
                for (size_t i = index; i < assignment.size(); i++) {
                    assignment[i] = false;
                }
                return true;
            }
        }

        /* Oops, all true. */
        return false;
    }

    /* Converts a PL formula to a string. */
    string toString(shared_ptr<PL::Expression> expr) {
        ostringstream result;
        result << *expr;
        return result.str();
    }

    /* Given a UTF-8 string, returns the number of Unicode characters in it. */
    size_t unicodeLength(const string& str) {
        istringstream extractor(str);

        size_t result = 0;

        /* TODO: This is the wrong way to do this. Find a better one. */
        while (true) {
            try {
                (void) readChar(extractor);
                result++;
            } catch (const UTFException &) {
                break;
            }
        }

        return result;
    }

    /* Centers a boolean inside of a string of the given length. */
    string centered(bool value, size_t length) {
        string result(length, ' ');
        result[result.size() / 2] = (value? 'T' : 'F');
        return result;
    }

    void printTruthTableFor(shared_ptr<PL::Expression> expr, const string& prefix) {
        string formula = toString(expr);

        /* Begin by listing the variables in some predicable order. */
        Vector<string> variables;
        for (const auto& var: variablesIn(expr)) {
            variables.push_back(var);
        }

        /* Print the variables in that order. */
        cout << prefix;
        for (string var: variables) {
            cout << var << ' ';
        }
        cout << formula << endl;

        /* List of variables' values, from left to right. */
        Vector<bool> curr(variables.size(), false);

        do {
            /* Print the values of each variable. */
            cout << prefix;
            for (int i = 0; i < curr.size(); i++) {
                cout << centered(curr[i], unicodeLength(variables[i])) << ' ';
            }

            /* Convert from list of booleans to evaluation context. */
            PL::Context context;
            for (int i = 0; i < curr.size(); i++) {
                context[variables[i]] = curr[i];
            }

            cout << centered(expr->evaluate(context), unicodeLength(formula)) << endl;
        } while (nextAssignment(curr));
    }

    void listPLFormulas(const string& filename) {
        cout << "Below are the propositional formulas you have written in the file " << endl;
        cout << filename << ", along with their truth tables." << endl;
        cout << endl;

        auto data = parseFile(filename);
        for (const auto& number: kSortedNumerals) {
            if (data.count(number)) {
                cout << number << endl;

                try {
                    auto formula = PL::parse(PL::scan(*data[number]));
                    cout << "    Your Formula: " << *formula << endl;
                    cout << "    Truth Table:" << endl;
                    printTruthTableFor(formula, "        ");
                } catch (const exception& e) {
                    cerr << "    Error: " << e.what() << endl;
                }

                cout << endl;
            }
        }
    }

    void propositionalCompleteness() {
        listPLFormulas("res/PropositionalCompleteness.proplogic");
    }

    void setTheoryAndPL() {
        listPLFormulas("res/SetTheory.proplogic");
    }

    void firstOrderNegations() {
        printFOLFormulas("res/FirstOrderNegations.fol");
    }

    void thisButNotThat() {
        printWorlds("res/ThisButNotThat.worlds");
    }

    void translatingIntoLogic() {
        printFOLFormulas("res/TranslatingIntoLogic.fol");
    }
}

int main() {
    MenuDriver::run();
    return 0;
}
#endif
