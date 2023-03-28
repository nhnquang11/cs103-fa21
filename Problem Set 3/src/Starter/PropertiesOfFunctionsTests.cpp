#include "GUI/SimpleTest.h"
#include "Utilities/JSON.h"
#include "Grabbag/GrabbagTester.h"
#include "PropertiesOfFunctions.h"
#include <set>
#include <fstream>
using namespace std;

/******* Hairy Scary Test Case Implementations Below This Point *******/

namespace {
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

    /* Given a list of seven answers, produces a key for those answers. */
    string keyFor(int answer) {
        return string(137, char(answer));
    }

    bool checkCorrectness(int part) {
        ifstream input("res/PropertiesOfFunctions.answers");
        if (!input) SHOW_ERROR("Cannot open file res/PropertiesOfFunctions.answers");

        /* -1 because we're zero-indexed. */
        string key = keyFor(JSON::parse(input)["answers"][part - 1].asInteger());

        bool correct = true;
        runPrivateTest("PropertiesOfFunctions_" + to_string(part), [&](istream& input) {
            for (uint64_t base, multiplier, result; input >> base >> multiplier >> result; ) {
                if (rollingHash(key, base, multiplier) != result) {
                    correct = false;
                }
            }
        });
        return correct;
    }
}

PROVIDED_TEST("Venn Diagram Answers") {
    size_t incorrect = 0;
    for (size_t i = 1; i <= kNumFunctions; i++) {
        if (!checkCorrectness(i)) incorrect++;
    }

    if (incorrect != 0) {
        size_t block = (incorrect / 3) * 3;
        if (block == 0) {
            SHOW_ERROR("At least one answer is incorrect.");
        } else {
            SHOW_ERROR("At least " + to_string(block) + " answers are incorrect.");
        }
    }
}
