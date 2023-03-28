#include "GUI/SimpleTest.h"
#include "FileParser/FileParser.h"
#include "Logic/WorldParser.h"
#include "Logic/IntDynParser.h"
#include "Logic/FOLParser.h"
#include "Logic/FOLExpression.h"
#include "Logic/FOLExpressionBuilder.h"
#include "Logic/RealEntity.h"
using namespace std;

namespace {
    /* List of Roman Numeral answer styles, in sorted order. */
    const vector<string> kSortedNumerals = {
        "[Part (i)]",
        "[Part (ii)]",
        "[Part (iii)]",
        "[Part (iv)]",
        "[Part (v)]",
        "[Part (vi)]",
        "[Part (vii)]",
        "[Part (viii)]",
        "[Part (ix)]",
        "[Part (x)]",
        "[Part (xi)]",
        "[Part (xii)]",
        "[Part (xiii)]",
        "[Part (xiv)]",
        "[Part (xv)]",
        "[Part (xvi)]",
        // Add more entries here if needed!
    };

    /* All the formulas used in the Interpersonal Dynamics problem. */
    const vector<string> kIDFormulas = {
        "Loves(p1, p3)",
        "Loves(p3, p4)",
        "Loves(p1, p2) ∧ Loves(p2, p1)",
        "Loves(p1, p2) ∨ Loves(p2, p1)",
        "Loves(p1, p1) → Loves(p5, p5)",
        "Loves(p1, p2) → Loves(p4, p3)",
        "Loves(p1, p3) → Loves(p3, p6)",
        "Loves(p1, p4) → Loves(p4, p5)",
        "Loves(p1, p4) ↔ Loves(p2, p3)",
        "Loves(p1, p3) ↔ Loves(p5, p5)",
        "∀x. ∃y. Loves(x, y)",
        "∀x. ∃y. Loves(y, x)",
        "∀x. ∃y. (x ≠ y ∧ Loves(x, y))",
        "∀x. ∃y. (x ≠ y ∧ Loves(y, x))",
        "∃x. ∀y. Loves(x, y)",
        "∃x. ∀y. (x ≠ y → Loves(x, y))",
    };

    /* World description for Interpersonal Dynamics. */
    const string kIDWorld = R"(
                            Person(p1)
                            Person(p2)
                            Person(p3)
                            Person(p4)
                            Person(p5)
                            Person(p6)
                            Loves(p1, p1)
                            Loves(p1, p3)
                            Loves(p3, p1)
                            Loves(p3, p2)
                            Loves(p4, p3)
                            Loves(p5, p5)
                            )";

    /* Entities in this world. */
    const vector<string> kIDEntities = { "p1", "p2", "p3", "p4", "p5", "p6" };

    /* The Interpersonal Dynamics world. */
    World idWorld() {
        /* Get the world, and build a context for it. */
        istringstream worldStream(kIDWorld);
        return parseWorld(worldStream);
    }

    /* The Interpersonal Dynamics build context. */
    FOL::BuildContext idContext(const World& world) {
        /* Start with the default. */
        auto result = entityBuildContext();

        /* Add in the people. */
        for (auto entity: world) {
            result.constants[entity->name] = entity;
        }

        return result;
    }

    /* Given a world, adds some more links into it. */
    World worldPlus(const World& original, const vector<pair<string, string>>& toAdd) {
        /* Serialize, add, deserialize. */
        stringstream builder;
        builder << original;

        for (const auto& entry: toAdd) {
            builder << "Loves(" << entry.first << ", " << entry.second << ")" << '\n';
        }

        return parseWorld(builder);
    }

    /* Recursive search to find whether there's a solution whose size is less than
     * the given size.
     */
    bool hasSmallerSoln(const shared_ptr<FOL::BooleanExpression>& formula,
                        World& world,
                        const vector<RealEntity*>& entities,
                        size_t size) {
        /* If this solution works, we're done. */
        if (formula->evaluate(world)) {
            return true;
        }

        /* If this is too long, stop and skip the work of generating successors. */
        if (size == 1) return false;

        /* See follow-ups. */
        for (size_t i = 0; i < entities.size(); i++) {
            for (size_t j = 0; j < entities.size(); j++) {
                /* Skip this if it's already present. */
                if (entities[i]->loves.insert(entities[j]).second) {
                    if (hasSmallerSoln(formula, world, entities, size - 1)) {
                        return true;
                    }
                    entities[i]->loves.erase(entities[j]);
                }
            }
        }

        return false;
    }

    /* Given the AST for a formula and the submitted answer, checks if there's a
     * smaller solution that also works.
     */
    bool isMinimal(shared_ptr<FOL::ASTNode> ast, size_t size) {
        if (size == 0) return true;

        World world = idWorld();

        /* Entity map, for efficiency. */
        vector<RealEntity*> entities(world.size());
        for (const auto& entity: world) {
            entities[entity->name[1] - '1'] = entity.get();
        }

        /* Build the formula once, for efficiency. */
        auto formula = FOL::buildExpressionFor(ast, idContext(world));
        return !hasSmallerSoln(formula, world, entities, size);
    }

    void testAnswer(size_t problemNumber1Indexed) {
        /* Get student's answer, the reference world, and the formula in question. */
        vector<pair<string, string>> answer;
        try {
            auto answers = parseFile("res/Interpersonal.dynamics");
            answer  = IntDyn::parse(IntDyn::scan(*answers.at(kSortedNumerals[problemNumber1Indexed - 1])));
        } catch (const out_of_range& e) {
            SHOW_ERROR("Error loading your answer: Section " + kSortedNumerals[problemNumber1Indexed - 1] + " not found.");
        } catch (const exception& e) {
            SHOW_ERROR("Error loading your answer: " + string(e.what()));
        }

        auto world   = worldPlus(idWorld(), answer);
        auto context = idContext(world);
        auto ast     = FOL::parse(FOL::scan(kIDFormulas[problemNumber1Indexed - 1]));
        auto formula = FOL::buildExpressionFor(ast, context);

        /* Confirm the formula is true in this world. */
        EXPECT(formula->evaluate(world));
        EXPECT(isMinimal(ast, answer.size()));
    }
}

PROVIDED_TEST("Part (i)") {
    testAnswer(1);
}

PROVIDED_TEST("Part (ii)") {
    testAnswer(2);
}

PROVIDED_TEST("Part (iii)") {
    testAnswer(3);
}

PROVIDED_TEST("Part (iv)") {
    testAnswer(4);
}

PROVIDED_TEST("Part (v)") {
    testAnswer(5);
}

PROVIDED_TEST("Part (vi)") {
    testAnswer(6);
}

PROVIDED_TEST("Part (vii)") {
    testAnswer(7);
}

PROVIDED_TEST("Part (viii)") {
    testAnswer(8);
}

PROVIDED_TEST("Part (ix)") {
    testAnswer(9);
}

PROVIDED_TEST("Part (x)") {
    testAnswer(10);
}

PROVIDED_TEST("Part (xi)") {
    testAnswer(11);
}

PROVIDED_TEST("Part (xii)") {
    testAnswer(12);
}

PROVIDED_TEST("Part (xiii)") {
    testAnswer(13);
}

PROVIDED_TEST("Part (xiv)") {
    testAnswer(14);
}

PROVIDED_TEST("Part (xv)") {
    testAnswer(15);
}

PROVIDED_TEST("Part (xvi)") {
    testAnswer(16);
}
