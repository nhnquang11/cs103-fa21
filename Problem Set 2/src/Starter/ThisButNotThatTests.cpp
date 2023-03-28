#include "GUI/SimpleTest.h"
#include "Logic/WorldParser.h"
#include "Logic/FOLParser.h"
#include "Logic/RealEntity.h"
#include "FileParser/FileParser.h"
#include "Grabbag/GrabbagTester.h"
using namespace std;

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

    /* Hash key derived from a world. We just count the number of entities and loves. */
    string keyFor(const World& w) {
        size_t entities = w.size();
        size_t loves    = 0;

        for (const auto& entry: w) {
            loves += entry->loves.size();
        }

        return to_string(entities) + "#" + to_string(loves);
    }

    /* Tests, probabilistically, whether the given world is minimal for the given part. */
    bool isMinimal(const string& part, const World& w) {
        string key = keyFor(w);

        /* Run the test. */
        bool success = true;
        runPrivateTest("ThisButNotThatHashes_" + part, [&](istream& input) {
            for (uint64_t base, multiplier, result; input >> base >> multiplier >> result; ) {
                if (rollingHash(key, base, multiplier) != result) {
                    success = false;
                }
            }
        });
        return success;
    }

    /* Checks if we pass the "this, but not that" criteria. */
    void thisAndNotThat(const string& part, const World& w, const string& thisFormula, const string& thatFormula) {
        auto thisFOL = buildExpressionFor(FOL::parse(FOL::scan(thisFormula)), entityBuildContext());
        auto thatFOL = buildExpressionFor(FOL::parse(FOL::scan(thatFormula)), entityBuildContext());

        if (!thisFOL->evaluate(w)) {
            SHOW_ERROR("\"This\" formula is not true.");
        }
        if (thatFOL->evaluate(w)) {
            SHOW_ERROR("\"That\" formula is not false.");
        }
        if (!isMinimal(part, w)) {
            SHOW_ERROR("Solution works, but is not minimal.");
        }
    }

    World loadWorld(const string& section) {
        try {
            return parseWorld(*parseFile("res/ThisButNotThat.worlds").at(section));
        } catch (const out_of_range &) {
            SHOW_ERROR("Error loading your solution: Could not find section " + section);
        } catch (const exception& e) {
            SHOW_ERROR("Error loading your solution: " + string(e.what()));
        }
    }
}

PROVIDED_TEST("Part (i)") {
    auto world = loadWorld("[Part (i)]");
    thisAndNotThat("i", world, "∀y. ∃x. Loves(x, y)", "∃x. ∀y. Loves(x, y)");
}


PROVIDED_TEST("Part (ii)") {
    auto world = loadWorld("[Part (ii)]");
    thisAndNotThat("ii", world, "∀x. (Person(x) ∨ Cat(x))", "(∀x. Person(x)) ∨ (∀x. Cat(x))");
}

PROVIDED_TEST("Part (iii)") {
    auto world = loadWorld("[Part (iii)]");
    thisAndNotThat("iii", world, "(∃x. Robot(x)) ∧ (∃x. Loves(x, x))", "∃x. (Robot(x) ∧ Loves(x, x))");
}

PROVIDED_TEST("Part (iv)") {
    auto world = loadWorld("[Part (iv)]");
    thisAndNotThat("iv", world, "(∀x. Cat(x)) → (∀y. Loves(y, y))", "∀x. ∀y. (Cat(x) → Loves(y, y))");
}

PROVIDED_TEST("Part (v)") {
    auto world = loadWorld("[Part (v)]");
    thisAndNotThat("v", world, "∃x. (Robot(x) → ∀y. Robot(y))", "(∀x. Robot(x)) ∨ (∀x. ¬Robot(x))");
}
