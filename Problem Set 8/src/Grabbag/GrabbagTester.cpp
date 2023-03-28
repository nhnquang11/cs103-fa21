#include "GrabbagTester.h"
#include "Grabbag.h"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

namespace {
    /* Loads the given Grabbag file. */
    Grabbag loadGrabbag() {
        ifstream input("res/tests/assignment.grabbag", ios::binary);
        return Grabbag(input);
    }
}

void runPrivateTest(const string& testName,
                    function<void(istream&)> callback) {
    /* Cache for future use. */
    static Grabbag grabbag = loadGrabbag();

    istringstream source(grabbag.contentsOf(testName));
    callback(source);
}
