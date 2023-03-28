#include "../GUI/SimpleTest.h"
#include "../FormalLanguages/Automaton.h"
#include "../Demos/AutomataEditorCore.h"
#include "../Grabbag/GrabbagTester.h"
#include <fstream>
using namespace std;

namespace {
    void runTests(const string& filename, const string& section, const string& alphabet, bool isDFA);
}

PROVIDED_TEST("DFA Part (i)") {
    runTests("Q1.i.automaton",   "DFA_i", "BI", true);
}

PROVIDED_TEST("DFA Part (ii)") {
    runTests("Q1.ii.automaton",  "DFA_ii", "yd", true);
}

PROVIDED_TEST("DFA Part (iii)") {
    runTests("Q1.iii.automaton", "DFA_iii", "ab", true);
}

PROVIDED_TEST("DFA Part (iv)") {
    runTests("Q1.iv.automaton",  "DFA_iv", "acmo", true);
}

PROVIDED_TEST("NFA Part (i)") {
    runTests("Q2.i.automaton",   "NFA_i", "abc", false);
}

PROVIDED_TEST("NFA Part (ii)") {
    runTests("Q2.ii.automaton",  "NFA_ii", "abcde", false);
}

PROVIDED_TEST("NFA Part (iii)") {
    runTests("Q2.iii.automaton", "NFA_iii", "abcde", false);
}

PROVIDED_TEST("NFA Part (iv)") {
    runTests("Q2.iv.automaton",  "NFA_iv", "ab", false);
}

/* * * * * Implementation Below This Point * * * * */
namespace {
    const size_t kTooBig = 50;

    void checkValidity(Editor::Automaton& automaton,
                       const Languages::Alphabet& alphabet,
                       bool shouldBeDFA) {
        if (automaton.alphabet() != alphabet) {
            SHOW_ERROR("Automaton's alphabet is incorrect (did you manually edit the .automaton files?)");
        }
        if (automaton.isDFA() != shouldBeDFA) {
            SHOW_ERROR("Automaton has the wrong type (did you manually edit the .automaton files?");
        }

        auto errors = automaton.checkValidity();
        if (!errors.empty()) {
            SHOW_ERROR("Automaton has structural errors (use the editor to find and correct them).");
        }
    }

    void runTests(const string& filename, const string& section, const string& alphabet, bool isDFA) {
        runPrivateTest(section, [&](istream& in) {
            /* Load the student solution. */
            ifstream input("res/" + filename);
            auto viewer = new Editor::Automaton(JSON::parse(input));
            checkValidity(*viewer, Languages::toAlphabet(alphabet), isDFA);

            /* Make sure the automaton isn't too big. */
            auto studentNFA = viewer->toNFA();
            if (studentNFA.states.size() >= kTooBig) {
                SHOW_ERROR("Automaton uses too many states. See if you can find a smaller automaton.");
            }

            auto studentDFA = Automata::subsetConstruct(studentNFA);

            /* Convert alphabet to a proper alphabet object. */
            Languages::Alphabet sigma;
            for (char ch: alphabet) {
                sigma.insert(ch);
            }

            /* Load our reference solution and convert that to a DFA. */
            Automata::DFA ourDFA;
            in >> ourDFA;

            string counterexample;
            if (!Automata::areEquivalent(studentDFA, ourDFA, counterexample)) {
                SHOW_ERROR("Answer is incorrect; does not handle string \"" + counterexample + "\" correctly.");
            }
        });
    }
}
