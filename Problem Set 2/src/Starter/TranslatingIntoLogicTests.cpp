#include "../ExecutableLogic.h"
#include "WorldDecompressor.h"
#include "Logic/FOLParser.h"
#include "Logic/FOLExpressionBuilder.h"
#include "Logic/WorldParser.h"
#include "Logic/RealEntity.h"
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

    shared_ptr<FOL::BooleanExpression> loadFormula(const string& section) {
        try {
            return FOL::buildExpressionFor(FOL::parse(FOL::scan(*parseFile("res/TranslatingIntoLogic.fol").at(section))), entityBuildContext());
        } catch (const out_of_range &) {
            SHOW_ERROR("Error loading your answer: Section " + section + " not found.");
        } catch (const exception& e) {
            SHOW_ERROR("Error loading your answer: " + to_string(e.what()));
        }
    }
}

PROVIDED_TEST("Part (i)") {
    auto examples = examplesFrom("TranslationTest1.worlds");
    auto formula = loadFormula("[Part (i)]");
    EXPECT(passesTests(formula, examples.first, examples.second));
}

PROVIDED_TEST("Part (ii)") {
    auto examples = examplesFrom("TranslationTest2.worlds");
    auto formula = loadFormula("[Part (ii)]");
    EXPECT(passesTests(formula, examples.first, examples.second));
}

PROVIDED_TEST("Part (iii)") {
    auto examples = examplesFrom("TranslationTest3.worlds");
    auto formula = loadFormula("[Part (iii)]");
    EXPECT(passesTests(formula, examples.first, examples.second));
}

PROVIDED_TEST("Part (iv)") {
    auto examples = examplesFrom("TranslationTest4.worlds");
    auto formula = loadFormula("[Part (iv)]");
    EXPECT(passesTests(formula, examples.first, examples.second));
}

PROVIDED_TEST("Part (v)") {
    auto examples = examplesFrom("TranslationTest5.worlds");
    auto formula = loadFormula("[Part (v)]");
    EXPECT(passesTests(formula, examples.first, examples.second));
}

PROVIDED_TEST("Part (vi)") {
    auto examples = examplesFrom("TranslationTest6.worlds");
    auto formula = loadFormula("[Part (vi)]");
    EXPECT(passesTests(formula, examples.first, examples.second));
}
