#include "../FileParser/FileParser.h"
#include "../Grabbag/GrabbagTester.h"
#include "../GUI/SimpleTest.h"
#include "../FormalLanguages/RegexParser.h"
#include "../FormalLanguages/RegexScanner.h"
#include "../FormalLanguages/Automaton.h"
#include <fstream>
using namespace std;

namespace {
    void runTests(const string& part);
}

PROVIDED_TEST("Part (i)") {
    runTests("i");
}

PROVIDED_TEST("Part (ii)") {
    runTests("ii");
}

PROVIDED_TEST("Part (iii)") {
    runTests("iii");
}

PROVIDED_TEST("Part (iv)") {
    runTests("iv");
}

PROVIDED_TEST("Part (v)") {
    runTests("v");
}

/* * * * * Implementation Below This Point * * * * */
namespace {
    void runTests(const string& part) {
        runPrivateTest("StateElimination_" + part, [&](istream& in) {
            /* Load the student solution. */
            auto allAnswers = parseFile("res/StateElimination.regexes");
            if (!allAnswers.count("[Part (" + part + ")]")) {
                SHOW_ERROR("No section labeled [Part (" + part + ")] in res/StateElimination.regexes");
            }

            Regex::Regex studentRegex;
            try {
                studentRegex = parse(Regex::scan(*allAnswers["[Part (" + part + ")]"]));
            } catch (const exception& e) {
                SHOW_ERROR("Error reading regex: " + string(e.what()));
            }

            /* Convert that into a DFA. */
            Automata::DFA studentDFA = Automata::subsetConstruct(Automata::fromRegex(studentRegex, {'a', 'b'}));

            /* Load our reference solution and convert that to a DFA. */
            Automata::NFA ourNFA;
            in >> ourNFA;
            Automata::DFA ourDFA = Automata::subsetConstruct(ourNFA);

            string counterexample;
            if (!Automata::areEquivalent(studentDFA, ourDFA, counterexample)) {
                SHOW_ERROR("Answer is incorrect; does not handle string \"" + counterexample + "\" correctly.");
            }
        });
    }
}
