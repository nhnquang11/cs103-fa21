#include "../SetTheory.h"
#include "../FileParser/FileParser.h"
#include "../Grabbag/GrabbagTester.h"
#include "../SetTheory/ObjectParser.h"
#include "../GUI/SimpleTest.h"
#include <sstream>
using namespace std;

namespace {
    void testPart(const string& partName);
}

PROVIDED_TEST("Part (i)") {
    testPart("i");
}

PROVIDED_TEST("Part (ii)") {
    testPart("ii");
}

PROVIDED_TEST("Part (iii)") {
    testPart("iii");
}

PROVIDED_TEST("Part (iv)") {
    testPart("iv");
}

PROVIDED_TEST("Part (v)") {
    testPart("v");
}

PROVIDED_TEST("Part (vi)") {
    testPart("vi");
}


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

    /* Tests a specific part of the problem. */
    void testPart(const string& partName) {
        /* Open master file. */
        decltype(parseFile("")) contents;
        try {
            contents = parseFile("res/MuchAdoAboutNothing.sets");
        } catch (const exception &) {
            SHOW_ERROR("Could not open file res/MuchAdoAboutNothing.sets");
        }

        /* Extract relevant section. */
        shared_ptr<istream> src;
        try {
            src = contents.at("[Part (" + partName +")]");
        } catch (const exception &) {
            SHOW_ERROR("Could not find section [Part (" + partName + ")] in res/MuchAdoAboutNothing.sets");
        }

        /* Get our set. */
        string set;
        try {
            ostringstream builder;
            builder << SetTheory::parse(*src) << endl; // endl needed since we did that during test generation
            set = builder.str();
        } catch (const exception& e) {
            SHOW_ERROR("Error parsing answer for part (" + partName + "): " + e.what());
        }

        /* Run the test. */
        runPrivateTest("MuchAdoAboutNothing_" + partName, [&](istream& input) {
            for (uint64_t base, multiplier, result; input >> base >> multiplier >> result; ) {
                if (rollingHash(set, base, multiplier) != result) {
                    SHOW_ERROR("Answer is incorrect.");
                }
            }
        });
    }
}
