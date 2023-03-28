#include "../ExecutableLogic.h"
#include "WorldDecompressor.h"
#include "Logic/FOLParser.h"
#include "Logic/FOLExpressionBuilder.h"
#include "Logic/WorldParser.h"
#include "FileParser/FileParser.h"
#include "GUI/SimpleTest.h"
#include "Grabbag/GrabbagTester.h"
using namespace std;

namespace {
    /* Given a collection of positive and negative examples, does the given function evaluate
     * to true for all the positive examples and false for all the negative examples?
     */
    template <typename Examples>
    bool passesTests(function<bool(World)> predicate,
                     const Examples& negative,
                     const Examples& positive) {
        try {
            for (const auto& example: negative) {
                if (predicate(example)) {
                    return false;
                }
            }
            for (const auto& example: positive) {
                if (!predicate(example)) {
                    return false;
                }
            }

            return true;
        } catch (const exception& e) {
            SHOW_ERROR("Function reported an error: " + string(e.what()));
        }
    }

    /* Convenience wrapper for the above function given an FOL formula. */
    template <typename Examples>
    bool passesTests(shared_ptr<FOL::BooleanExpression> expr,
                     const Examples& negative,
                     const Examples& positive) {
        return passesTests([&](const World& world) { return expr->evaluate(world); },
        negative, positive);
    }

    /* Given a filename, returns all the negative and positive examples from that file,
     * respectively.
     */
    pair<vector<World>, vector<World>> examplesFrom(const string& filename) {
        pair<vector<World>, vector<World>> result;
        runPrivateTest(filename, [&](istream& in) {
            result = Decompressor::parse(in);
        });
        return result;
    }
}

PROVIDED_TEST("Part (i)") {
    auto examples = examplesFrom("ExecutableLogicTest1.worlds");
    EXPECT(passesTests(isFormulaTrueFor_partI, examples.first, examples.second));
}

PROVIDED_TEST("Part (ii)") {
    auto examples = examplesFrom("ExecutableLogicTest2.worlds");
    EXPECT(passesTests(isFormulaTrueFor_partII, examples.first, examples.second));
}

PROVIDED_TEST("Part (iii)") {
    auto examples = examplesFrom("ExecutableLogicTest3.worlds");
    EXPECT(passesTests(isFormulaTrueFor_partIII, examples.first, examples.second));
}

PROVIDED_TEST("Part (iv)") {
    auto examples = examplesFrom("ExecutableLogicTest4.worlds");
    EXPECT(passesTests(isFormulaTrueFor_partIV, examples.first, examples.second));
}

PROVIDED_TEST("Part (v)") {
    auto examples = examplesFrom("ExecutableLogicTest5.worlds");
    EXPECT(passesTests(isFormulaTrueFor_partV, examples.first, examples.second));
}

PROVIDED_TEST("Part (vi)") {
    auto examples = examplesFrom("ExecutableLogicTest6.worlds");
    EXPECT(passesTests(isFormulaTrueFor_partVI, examples.first, examples.second));
}
