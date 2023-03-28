#include "FormalLanguages/CFG.h"
#include "FormalLanguages/CFGParser.h"
#include "FormalLanguages/RegexParser.h"
#include "Grabbag/GrabbagTester.h"
#include "GUI/SimpleTest.h"
#include "FileParser/FileParser.h"
#include "Utilities/JSON.h"
#include "CFGLoader.h"
using namespace std;

namespace {
    void runTests(const string& sectionHeader);
}

PROVIDED_TEST("Q1.i") {
    runTests("Q1.i");
}
PROVIDED_TEST("Q1.ii") {
    runTests("Q1.ii");
}
PROVIDED_TEST("Q1.iii") {
    runTests("Q1.iii");
}
PROVIDED_TEST("Q1.iv") {
    runTests("Q1.iv");
}
PROVIDED_TEST("Q2.ii") {
    runTests("Q2.ii");
}
PROVIDED_TEST("Q3.ii") {
    runTests("Q3.ii");
}




/* * * * * Implementation Below This Point * * * * */
#include <chrono>

namespace {
    const size_t kMaxSize      = 15;
    const size_t kTestsPerSize = 350;

    const bool kDebugTimingOn = false;

    /* Fuzz-tests two CFGs against one another. Returns whether they seem to match, and if
     * not outputs a string that they disagree on.
     *
     * It's, in general, undecidable whether two CFGs are equal to one another, so there
     * isn't some magic nice procedure we can use to speed this up.
     */
    bool areProbablyEquivalent(const CFG::CFG& one, const CFG::CFG& two, string& out) {
        auto now = chrono::high_resolution_clock::now();
        auto match1 = CFG::matcherFor(one);
        auto match2 = CFG::matcherFor(two);

        auto time = chrono::high_resolution_clock::now() - now;
        if (kDebugTimingOn) cout << "Matchers: " << chrono::duration_cast<chrono::milliseconds>(time).count() << endl;
        now = chrono::high_resolution_clock::now();

        auto gen1   = CFG::generatorFor(one);
        auto gen2   = CFG::generatorFor(two);

        time = chrono::high_resolution_clock::now() - now;
        if (kDebugTimingOn) cout << "Generators: " << chrono::duration_cast<chrono::milliseconds>(time).count() << endl;
        now = chrono::high_resolution_clock::now();


        for (size_t i = 0; i < kMaxSize; i++) {
            for (size_t trial = 0; trial < kTestsPerSize; trial++) {
                /* L(one) subset L(two)? */
                auto str1 = gen1(i);
                if (str1.first && !match2(str1.second)) {
                    out = str1.second;
                    return false;
                }

                /* L(two) subset L(one)? */
                auto str2 = gen2(i);
                if (str2.first && !match1(str2.second)) {
                    out = str2.second;
                    return false;
                }
            }
        }

        time = chrono::high_resolution_clock::now() - now;
        if (kDebugTimingOn) cout << "Tests: " << chrono::duration_cast<chrono::milliseconds>(time).count() << endl;

        return true;
    }

    void runTests(const string& sectionHeader) {
        runPrivateTest(sectionHeader, [&](istream& in) {
            /* Read data about this problem. */
            JSON data = JSON::parse(in);
            auto alphabet = Languages::toAlphabet(data["alphabet"].asString());
            auto ourCFG   = CFG::parse(data["cfg"], alphabet);

            /* Load the student solution. */
            CFG::CFG studentCFG;
            try {
                studentCFG = loadCFG(sectionHeader, alphabet);
            } catch (const exception& e) {
                SHOW_ERROR(e.what());
            }

            string counterexample;
            if (!areProbablyEquivalent(studentCFG, ourCFG, counterexample)) {
                SHOW_ERROR("Answer is incorrect; does not handle string \"" + counterexample + "\" correctly.");
            }
        });
    }
}
