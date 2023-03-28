#include "../SetTheory.h"
#include "../FileParser/FileParser.h"
#include "../Grabbag/GrabbagTester.h"
#include "../SetTheory/ObjectParser.h"
#include "../GUI/SimpleTest.h"
#include <sstream>
using namespace std;

namespace {
    void testPart(const string& partName, bool predicate(Object, Object));
}

PROVIDED_TEST("isElementOf") {
    testPart("isElementOf", isElementOf);
}

PROVIDED_TEST("isSubsetOf") {
    testPart("isSubsetOf", isSubsetOf);
}

PROVIDED_TEST("areDisjointSets") {
    testPart("areDisjointSets", areDisjointSets);
}

PROVIDED_TEST("isSingletonOf") {
    testPart("isSingletonOf", isSingletonOf);
}

PROVIDED_TEST("isElementOfPowerSet") {
    testPart("isElementOfPowerSet", isElementOfPowerSet);
}

PROVIDED_TEST("isSubsetOfPowerSet") {
    testPart("isSubsetOfPowerSet", isSubsetOfPowerSet);
}

PROVIDED_TEST("isSubsetOfDoublePowerSet") {
    testPart("isSubsetOfDoublePowerSet", isSubsetOfDoublePowerSet);
}


/******* Hairy Scary Test Case Implementations Below This Point *******/

namespace {
    /* Tests a specific part of the problem. */
    void testPart(const string& partName, bool predicate(Object, Object)) {
        /* Run the test. */
        runPrivateTest("ExecutableSetTheory_" + partName, [&](istream& input) {
            for (string s, t, result; getline(input, s) && getline(input, t) && getline(input, result); ) {
                istringstream sInput(s), tInput(t);

                Object S = SetTheory::parse(sInput);
                Object T = SetTheory::parse(tInput);
                bool expected = result == "true";

                if (predicate(S, T) != expected) {
                    SHOW_ERROR(partName + "(" + s + ", " + t + ") != " + result);
                }
            }
        });
    }
}
