#include "GUI/SimpleTest.h"
#include "Turing/Turing.h"
#include "Turing/TuringScanner.h"
#include "Turing/TuringParser.h"
#include "Grabbag/GrabbagTester.h"
#include <string>
#include <vector>
#include <utility>
#include <fstream>
using namespace std;

namespace {
    void runTests(const string& filename);
}

PROVIDED_TEST("MiddleA.tm") {
    runTests("MiddleA.tm");
}

PROVIDED_TEST("Power2.tm") {
    runTests("Power2.tm");
}

PROVIDED_TEST("Equal.tm") {
    runTests("Equal.tm");
}

namespace {
    const string kBaseDir = "res/";

    /* Maximum number of steps to run things for. */
    const int kMaxSteps = 10000000;

    bool readEntry(pair<string, bool>& entry, istream& in) {
        string first, second;
        if (!getline(in, first) || !getline(in, second)) return false;

        entry = make_pair(first, second == "true");
        return true;
    }

    void runTests(const string& filename) {
        ifstream input(kBaseDir + filename, ios::binary);
        if (!input) SHOW_ERROR("Error opening file " + filename);

        Turing::Program tm(input);
        if (!tm.isValid()) SHOW_ERROR("TM contains errors and cannot be run; use the debugger to see what they are.");

        runPrivateTest(filename, [&](istream& in) {
            for (pair<string, bool> entry; readEntry(entry, in); ) {
                Turing::Interpreter interpreter(tm, { entry.first.begin(), entry.first.end() });

                for (int i = 0; i <= kMaxSteps && interpreter.state() == Turing::Result::RUNNING; i++) {
                    interpreter.step();
                }

                if (interpreter.state() == Turing::Result::RUNNING) {
                    SHOW_ERROR("TM still running after " + addCommasTo(kMaxSteps) + " steps on input \"" + entry.first + "\". Possible infinite loop?");
                } else if (interpreter.state() == Turing::Result::ACCEPT) {
                    if (!entry.second) {
                        SHOW_ERROR("TM accepted \"" + entry.first + "\", but it should reject this input.");
                    }
                }  else if (interpreter.state() == Turing::Result::REJECT) {
                    if (entry.second) {
                        SHOW_ERROR("TM rejected \"" + entry.first + "\", but it should accept this input.");
                    }
                }
            }
        });
    }
}
