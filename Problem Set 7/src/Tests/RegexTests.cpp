#include "../FileParser/FileParser.h"
#include "../Grabbag/GrabbagTester.h"
#include "../GUI/SimpleTest.h"
#include "../FormalLanguages/RegexParser.h"
#include "../FormalLanguages/RegexScanner.h"
#include "../FormalLanguages/Automaton.h"
#include <fstream>
using namespace std;

namespace {
    void runTests(const string& sectionHeader, const string& problemNumber, const string& alphabet);
}

PROVIDED_TEST("Part (i)") {
    runTests("[Part (i)]", "DesigningRegularExpressions_i", "abcde");
}

PROVIDED_TEST("Part (ii)") {
    runTests("[Part (ii)]", "DesigningRegularExpressions_ii", "abcde");
}

PROVIDED_TEST("Part (iii)") {
    runTests("[Part (iii)]", "DesigningRegularExpressions_iii", "a/");
}

PROVIDED_TEST("Part (iv)") {
    runTests("[Part (iv)]", "DesigningRegularExpressions_iv", "yd");
}

PROVIDED_TEST("Part (v)") {
    runTests("[Part (v)]", "DesigningRegularExpressions_v", "MDCLXVI");
}

/* * * * * Implementation Below This Point * * * * */
namespace {
    void runTests(const string& sectionHeader, const string& problemNumber, const string& alphabet) {
        runPrivateTest(problemNumber, [&](istream& in) {
            /* Load the student solution. */
            auto allAnswers = parseFile("res/RegularExpressions.regexes");
            if (!allAnswers.count(sectionHeader)) {
                SHOW_ERROR("No section labeled " + sectionHeader + " in res/RegularExpressions.regexes");
            }

            Regex::Regex studentRegex;
            try {
                studentRegex = parse(Regex::scan(*allAnswers[sectionHeader]));
            } catch (const exception& e) {
                SHOW_ERROR("Error reading regex: " + string(e.what()));
            }

            /* Convert alphabet to a proper alphabet object. */
            Languages::Alphabet sigma;
            for (char ch: alphabet) {
                sigma.insert(ch);
            }

            /* Convert that into a DFA. */
            Automata::DFA studentDFA = Automata::subsetConstruct(Automata::fromRegex(studentRegex, sigma));

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
