#include "Logic/PLParser.h"
#include "GUI/SimpleTest.h"
#include "FileParser/FileParser.h"
#include <unordered_set>
#include <set>
#include <typeindex>
using namespace std;

namespace {
    bool usesOnlyTheseVariables(shared_ptr<PL::Expression> expr, const set<string>& variables) {
        return variablesIn(expr) == variables;
    }

    bool usesOnlyTheseConnectives(shared_ptr<PL::Expression> expr, const unordered_set<type_index>& types) {
        bool result = true;
        class TypeChecker: public PL::ExpressionTreeWalker {
        public:
            const unordered_set<type_index>* types;
            bool* result;
            TypeChecker(const unordered_set<type_index>* types, bool* result) : types(types), result(result) {}

            void handle(const PL::AndExpression&) override {
                *result &= types->count(typeid(PL::AndExpression));
            }
            void handle(const PL::OrExpression&) override {
                *result &= types->count(typeid(PL::OrExpression));
            }
            void handle(const PL::IffExpression&) override {
                *result &= types->count(typeid(PL::IffExpression));
            }
            void handle(const PL::ImpliesExpression&) override {
                *result &= types->count(typeid(PL::ImpliesExpression));
            }
            void handle(const PL::NotExpression&) override {
                *result &= types->count(typeid(PL::NotExpression));
            }
            void handle(const PL::TrueExpression&) override {
                *result &= types->count(typeid(PL::TrueExpression));
            }
            void handle(const PL::FalseExpression&) override {
                *result &= types->count(typeid(PL::FalseExpression));
            }
        };

        TypeChecker checker(&types, &result);
        expr->accept(&checker);
        return result;
    }

    bool isEquivalent(shared_ptr<PL::Expression> expr, const string& to) {
        return truthTableFor(expr) == truthTableFor(PL::parse(PL::scan(to)));
    }

    shared_ptr<PL::Expression> loadFormula(const string& section) {
        try {
            return PL::parse(PL::scan(*parseFile("res/PropositionalCompleteness.proplogic").at(section)));
        } catch (const out_of_range &) {
            SHOW_ERROR("Error loading formula: Section " + section + " not found.");
        } catch (const exception& e) {
            SHOW_ERROR("Error loading formlua: " + string(e.what()));
        }
    }
}

PROVIDED_TEST("Part (i)") {
    auto formula = loadFormula("[Part (i)]");
    EXPECT(usesOnlyTheseVariables(formula, {}) &&
           usesOnlyTheseConnectives(formula, { typeid(PL::AndExpression),
                                               typeid(PL::OrExpression),
                                               typeid(PL::NotExpression),
                                               typeid(PL::TrueExpression) }) &&
           isEquivalent(formula, "false"));
}

PROVIDED_TEST("Part (ii)") {
    auto formula = loadFormula("[Part (ii)]");
    EXPECT(usesOnlyTheseVariables(formula, {"p", "q"}) &&
           usesOnlyTheseConnectives(formula, { typeid(PL::AndExpression),
                                               typeid(PL::OrExpression),
                                               typeid(PL::NotExpression),
                                               typeid(PL::TrueExpression) }) &&
           isEquivalent(formula, "p -> q"));
}

PROVIDED_TEST("Part (iii)") {
    auto formula = loadFormula("[Part (iii)]");
    EXPECT(usesOnlyTheseVariables(formula, {"p", "q"}) &&
           usesOnlyTheseConnectives(formula, { typeid(PL::AndExpression),
                                               typeid(PL::OrExpression),
                                               typeid(PL::NotExpression),
                                               typeid(PL::TrueExpression) }) &&
           isEquivalent(formula, "p <-> q"));
}

PROVIDED_TEST("Part (iv)") {
    auto formula = loadFormula("[Part (iv)]");
    EXPECT(usesOnlyTheseVariables(formula, {"p", "q"}) &&
           usesOnlyTheseConnectives(formula, { typeid(PL::AndExpression),
                                               typeid(PL::NotExpression),
                                               typeid(PL::TrueExpression) }) &&
           isEquivalent(formula, "p or q"));
}

PROVIDED_TEST("Part (v)") {
    auto formula = loadFormula("[Part (v)]");
    EXPECT(usesOnlyTheseVariables(formula, {}) &&
           usesOnlyTheseConnectives(formula, { typeid(PL::ImpliesExpression),
                                               typeid(PL::FalseExpression) }) &&
           isEquivalent(formula, "true"));
}

PROVIDED_TEST("Part (vi)") {
    auto formula = loadFormula("[Part (vi)]");
    EXPECT(usesOnlyTheseVariables(formula, {"p"}) &&
           usesOnlyTheseConnectives(formula, { typeid(PL::ImpliesExpression),
                                               typeid(PL::FalseExpression) }) &&
           isEquivalent(formula, "not p"));
}

PROVIDED_TEST("Part (vii)") {
    auto formula = loadFormula("[Part (vii)]");
    EXPECT(usesOnlyTheseVariables(formula, {"p", "q"}) &&
           usesOnlyTheseConnectives(formula, { typeid(PL::ImpliesExpression),
                                               typeid(PL::FalseExpression) }) &&
           isEquivalent(formula, "p and q"));
}
