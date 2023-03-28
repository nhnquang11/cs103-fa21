/**
 * Logic to build FOL expression trees.
 */
#ifndef FOLExpressionBuilder_Included
#define FOLExpressionBuilder_Included

#include "FOLExpression.h"
#include "FOLAST.h"
#include <map>
#include <string>
#include <memory>

namespace FOL {
  /* Types representing information about predicates and functions. */
  struct PredicateInfo {
    std::size_t arity;
    FOLPredicate callback;
  };

  struct FunctionInfo {
    std::size_t arity;
    FOLFunction callback;
  };

  /* Context for building an expression. This includes information about what
   * each predicate and function is and what constants, if any, exist.
   */
  struct BuildContext {
    std::map<std::string, Entity>        constants;
    std::map<std::string, PredicateInfo> predicates;
    std::map<std::string, FunctionInfo>  functions;
  };

  /* Translates an AST into a expression tree given information about what
   * predicates and functions are available. If the expression doesn't parse,
   * or it parses into something other than a first-order sentence (e.g.
   * a function application), this function throws an exception.
   */
  std::shared_ptr<BooleanExpression> buildExpressionFor(std::shared_ptr<ASTNode> ast,
                                                        const BuildContext& context);

  /* Generates a random legal boolean expression. The quality of that expression is not
   * guaranteed to be high. :-)
   */
  std::shared_ptr<BooleanExpression> randomExpression(const BuildContext& context);
}

#endif
