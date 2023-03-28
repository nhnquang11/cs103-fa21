#include "../GUI/SimpleTest.h"
#include "../FormalLanguages/Automaton.h"
#include "../Grabbag/GrabbagTester.h"
#include "../FormalLanguages/Utilities/JSON.h"
#include <fstream>
using namespace std;

namespace {
    void runTests(const string& filename, const string& testName);
}

PROVIDED_TEST("5-Tuple DFA (i)") {
    runTests("res/Q7.i.automaton",   "5Tuple_i");
}

PROVIDED_TEST("5-Tuple DFA (ii)") {
    runTests("res/Q7.ii.automaton",  "5Tuple_ii");
}

/* * * * * Implementation Below This Point * * * * */
namespace {
    /* Yes, we know that we're supposed to have a DFA here. But we store things
     * internally as NFAs because we can't guarantee that what's on disk really
     * is an NFA.
     */
    Automata::NFA loadDFA(const string& filename) {
        ifstream input(filename);
        if (!input) SHOW_ERROR("Cannot open " + filename + ".");

        try {
            istringstream data(JSON::parse(input)["aux"]["automaton"].asString());
            Automata::NFA result;

            data >> result;
            if (!data) SHOW_ERROR("Error parsing file " + filename);

            return result;
        } catch (const JSONException& e) {
            SHOW_ERROR("Error parsing file " + filename);
        }
    }

    /* UTF-aware string comparsion. */
    bool utf8Compare(const string& lhs, const string& rhs) {
        vector<char32_t> l;
        for (char32_t ch: utf8Reader(lhs)) {
            l.push_back(ch);
        }

        vector<char32_t> r;
        for (char32_t ch: utf8Reader(rhs)) {
            r.push_back(ch);
        }

        return lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
    }

    /* Converts an NFA into a canonical string representation. We list the states
     * separated by semicolons, with each state storing whether it's accepting
     * and then listing the names of the states it transitions to.
     */
    string keyFor(const Automata::NFA& nfa) {
        /* Get the states in sorted order. */
        vector<shared_ptr<Automata::State>> states(nfa.states.begin(), nfa.states.end());
        sort(states.begin(), states.end(), [](shared_ptr<Automata::State> lhs, shared_ptr<Automata::State> rhs) {
            return utf8Compare(lhs->name, rhs->name);
        });

        ostringstream result;

        /* List each state. */
        for (auto state: states) {
            /* State name and properties. */
            result << state->name << ":" << boolalpha << state->isAccepting << ":" << state->isStart << ":";

            /* State transitions. Sort to keep canonical. */
            vector<pair<char32_t, string>> transitions;
            for (auto entry: state->transitions) {
                transitions.push_back(make_pair(entry.first, entry.second->name));
            }

            sort(transitions.begin(), transitions.end(), [](pair<char32_t, string> lhs, pair<char32_t, string> rhs) {
                if (lhs.first != rhs.first) {
                    return lhs.first < rhs.first;
                }
                return utf8Compare(lhs.second, rhs.second);
            });

            for (auto entry: transitions) {
                result << toUTF8(entry.first) << " -> " << entry.second << ";";
            }

            result << "$";
        }

        return result.str();
    }

    /* Rabin rolling hash code. Our implementation works by checking your answers against
     * a series of hash codes for the correct answer. This gives high confidence that your
     * answer is correct without actually revealing what the answer is. :-)
     */
    uint32_t rollingHash(const string& str, uint64_t base, uint64_t multiplier) {
        const uint64_t modulus = 0x7FFFFFFF;
        uint64_t result = base;
        for (char ch: str) {
            result = (((result * multiplier) % modulus) + uint8_t(ch)) % modulus;
        }
        return result % modulus;
    }

    void runTests(const string& filename, const string& testName) {
        auto key = keyFor(loadDFA(filename));

        /* Run the test. */
        runPrivateTest(testName, [&](istream& input) {
            for (uint64_t base, multiplier, result; input >> base >> multiplier >> result; ) {
                if (rollingHash(key, base, multiplier) != result) {
                    SHOW_ERROR("Answer is incorrect.");
                }
            }
        });
    }
}
