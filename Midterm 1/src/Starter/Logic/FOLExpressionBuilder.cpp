#include "FOLExpressionBuilder.h"
#include <stdexcept>
#include <set>
#include <random>
using namespace std;

namespace FOL {
  namespace {
    /* Like a regular BuildContext, but with variables! */
    struct FullBuildContext {
      map<string, Entity>        constants;
      map<string, PredicateInfo> predicates;
      map<string, FunctionInfo>  functions;
      set<string>                variables;
    };

    /* Assertions for specific expression types. */
    shared_ptr<BooleanExpression> asBoolean(shared_ptr<Expression> expr) {
      if (auto result = dynamic_pointer_cast<BooleanExpression>(expr)) return result;
      throw runtime_error("Wrong expression type.");
    }

    shared_ptr<ValueExpression> asValue(shared_ptr<Expression> expr) {
      if (auto result = dynamic_pointer_cast<ValueExpression>(expr)) return result;
      throw runtime_error("Wrong expression type.");
    }

    /* Returns a pointer to the PredicateInfo / FunctionInfo object with the given name,
     * if one exists.
     */
    const PredicateInfo* predicateNamed(const FullBuildContext& context, const string& name) {
      auto itr = context.predicates.find(name);
      return itr == context.predicates.end()? nullptr : &itr->second;
    }
    const FunctionInfo* functionNamed(const FullBuildContext& context, const string& name) {
      auto itr = context.functions.find(name);
      return itr == context.functions.end()? nullptr : &itr->second;
    }

    /* Is this name used anywhere? */
    bool nameExists(const string& name, const FullBuildContext& context) {
      return context.constants.count(name) ||
          context.predicates.count(name) ||
          context.functions.count(name) ||
          context.variables.count(name);
    }

    /* Recursive walk that does the conversion. */
    shared_ptr<Expression> convert(shared_ptr<ASTNode> ast,
                                   const FullBuildContext& context) {
      shared_ptr<Expression> result;

      class Converter: public ASTVisitor {
      public:
        Converter(const FullBuildContext& context,
                  shared_ptr<Expression>* result) : context(context), result(result) {}
        FullBuildContext context;
        shared_ptr<Expression>* result;

        /* Default behavior is to fail so we know something's up. */
        void visit(const ASTNode&) override {
          throw runtime_error("Unknown node type.");
        }
        void visit(const TrueNode&) override {
          *result = make_shared<TrueExpression>();
        }
        void visit(const FalseNode&) override {
          *result = make_shared<FalseExpression>();
        }
        void visit(const NotNode& node) override {
          *result = make_shared<NotExpression>(asBoolean(convert(node.underlying, context)));
        }
        void visit(const AndNode& node) override {
          *result = make_shared<AndExpression>(asBoolean(convert(node.lhs, context)),
                                               asBoolean(convert(node.rhs, context)));
        }
        void visit(const OrNode& node) override {
          *result = make_shared<OrExpression>(asBoolean(convert(node.lhs, context)),
                                              asBoolean(convert(node.rhs, context)));
        }
        void visit(const ImpliesNode& node) override {
          *result = make_shared<ImpliesExpression>(asBoolean(convert(node.lhs, context)),
                                                   asBoolean(convert(node.rhs, context)));
        }
        void visit(const IffNode& node) override {
          *result = make_shared<IffExpression>(asBoolean(convert(node.lhs, context)),
                                               asBoolean(convert(node.rhs, context)));
        }
        void visit(const EqualsNode& node) override {
          *result = make_shared<EqualsExpression>(asValue(convert(node.lhs, context)),
                                                  asValue(convert(node.rhs, context)));
        }
        void visit(const NotEqualsNode& node) override {
          *result = make_shared<NotEqualsExpression>(asValue(convert(node.lhs, context)),
                                                     asValue(convert(node.rhs, context)));
        }
        void visit(const UniversalNode& node) override {
          /* Confirm that there isn't already a variable with the given name in the context. */
          if (nameExists(node.variable, context)) throw runtime_error("Variable name already in use.");

          /* Create a new context introducing this variable. */
          auto nextContext = context;
          nextContext.variables.insert(node.variable);

          *result = make_shared<UniversalExpression>(node.variable, asBoolean(convert(node.underlying, nextContext)));
        }
        void visit(const ExistentialNode& node) override {
          /* Confirm that there isn't already a variable with the given name in the context. */
          if (nameExists(node.variable, context)) throw runtime_error("Variable name already in use.");

          /* Create a new context introducing this variable. */
          auto nextContext = context;
          nextContext.variables.insert(node.variable);

          *result = make_shared<ExistentialExpression>(node.variable, asBoolean(convert(node.underlying, nextContext)));
        }
        void visit(const InvokeNode& node) override {
          /* This could be either a predicate or a function. We'll handle each case separately. */
          if (auto* predicate = predicateNamed(context, node.identifier)) {
            if (predicate->arity != node.args.size()) throw runtime_error("Wrong number of arguments.");

            vector<shared_ptr<ValueExpression>> args;
            for (auto arg: node.args) {
              args.push_back(asValue(convert(arg, context)));
            }

            *result = make_shared<PredicateExpression>(node.identifier, args, predicate->callback);
          } else if (auto* function = functionNamed(context, node.identifier)) {
            if (function->arity != node.args.size()) throw runtime_error("Wrong number of arguments.");

            vector<shared_ptr<ValueExpression>> args;
            for (auto arg: node.args) {
              args.push_back(asValue(convert(arg, context)));
            }

            *result = make_shared<FunctionExpression>(node.identifier, args, function->callback);
          } else {
            throw runtime_error("There is predicate or function named \"" + node.identifier + "\".");
          }
        }
        void visit(const VariableNode& node) override {
          /* See whether we're a variable or a constant. */
          if (context.variables.count(node.name)) {
            *result = make_shared<VariableExpression>(node.name);
          } else if (context.constants.count(node.name)) {
            *result = make_shared<ConstantExpression>(node.name, context.constants.at(node.name));
          } else {
            throw runtime_error("The name \"" + node.name + "\" doesn't refer to a variable or constant in scope.");
          }
        }
      };

      Converter c(context, &result);
      ast->accept(&c);
      return result;
    }

    /* The random device used for all generation purposes. */
    mt19937 generator{random_device{}()};

    /* Maximum depth of a formula. */
    const size_t kMaxFormulaDepth = 7;

    size_t randomBetween(size_t low, size_t high) {
      return uniform_int_distribution<int>(low, high)(generator);
    }

    shared_ptr<BooleanExpression> randomBooleanExpression(FullBuildContext& c, size_t depth);
    shared_ptr<ValueExpression>   randomValueExpression(FullBuildContext& c, size_t depth);

    /* Given the current build context, could you make a value expression? */
    bool canMakeValueExpression(const FullBuildContext& c) {
      /* Yes, if there is a constant or a variable! */
      return !c.constants.empty() || !c.variables.empty();
    }

    /* Distribution over formula types. */
    discrete_distribution<size_t> kBoolWeights = {
      1,   // T
      1,   // F
      5,   // ~
      10,  // &
      3,   // |
      10,  // ->
      5,   // <->
      3,   // =
      3,   // !=
      10,  // A
      10,  // E
      30,  // Predicate
    };

    size_t randomExpressionType() {
      return kBoolWeights(generator);
    }

    shared_ptr<BooleanExpression> randomBooleanExpression(FullBuildContext& c,
                                                          size_t depth) {
      /* Base case: If the depth is zero, we have to end with true or false. */
      if (depth == kMaxFormulaDepth) {
        if (randomBetween(0, 1) == 0) {
          return make_shared<TrueExpression>();
        } else {
          return make_shared<FalseExpression>();
        }
      }

      /* Recursive case: Pick anything you'd like. */
      switch (randomExpressionType()) {
      case 0:
        return make_shared<TrueExpression>();
      case 1:
        return make_shared<FalseExpression>();
      case 2:
        return make_shared<NotExpression>(randomBooleanExpression(c, depth + 1));
      case 3:
        return make_shared<AndExpression>(randomBooleanExpression(c, depth + 1),
                                          randomBooleanExpression(c, depth + 1));
      case 4:
        return make_shared<OrExpression>(randomBooleanExpression(c, depth + 1),
                                         randomBooleanExpression(c, depth + 1));
      case 5:
        return make_shared<ImpliesExpression>(randomBooleanExpression(c, depth + 1),
                                              randomBooleanExpression(c, depth + 1));
      case 6:
        return make_shared<IffExpression>(randomBooleanExpression(c, depth + 1),
                                          randomBooleanExpression(c, depth + 1));
      case 7:
        /* Only do this if we could make a value expression. */
        if (!canMakeValueExpression(c)) return randomBooleanExpression(c, depth);
        return make_shared<EqualsExpression>(randomValueExpression(c, depth + 1),
                                             randomValueExpression(c, depth + 1));
      case 8:
        if (!canMakeValueExpression(c)) return randomBooleanExpression(c, depth);
        return make_shared<NotEqualsExpression>(randomValueExpression(c, depth + 1),
                                                randomValueExpression(c, depth + 1));
      case 9: {
        string var = "a" + to_string(c.variables.size());
        c.variables.insert(var);
        auto result = make_shared<UniversalExpression>(var,
                                                       randomBooleanExpression(c, depth + 1));
        c.variables.erase(var);
        return result;
      }

      case 10: {
        string var = "a" + to_string(c.variables.size());
        c.variables.insert(var);
        auto result = make_shared<ExistentialExpression>(var,
                                                         randomBooleanExpression(c, depth + 1));
        c.variables.erase(var);
        return result;
      }

      case 11: {
        /* We'd like to use a predicate, but we can only do that if we actually have
         * some variables / constants in scope along with a predicate to use.
         */
        if (c.predicates.empty() || (c.variables.empty() && c.constants.empty())) {
          return randomBooleanExpression(c, depth);
        }

        /* Select a predicate at random. */
        auto itr = next(c.predicates.begin(), randomBetween(0, c.predicates.size() - 1));

        vector<shared_ptr<ValueExpression>> args;
        for (size_t i = 0; i < itr->second.arity; i++) {
          args.push_back(randomValueExpression(c, depth + 1));
        }

        return make_shared<PredicateExpression>(itr->first, args, itr->second.callback);
      }

      default:
        throw runtime_error("Unknown expression type.");
      }
    }

    shared_ptr<ValueExpression> randomValueExpression(FullBuildContext& c,
                                                      size_t depth) {
      /* For simplicity, make a list of all constants and variables. */
      vector<shared_ptr<ValueExpression>> constants;
      for (const auto& entry: c.constants) {
        constants.push_back(make_shared<ConstantExpression>(entry.first, entry.second));
      }
      for (const string& name: c.variables) {
        constants.push_back(make_shared<VariableExpression>(name));
      }

      /* Base case: at max depth, we have no choice but to return a variable or
       * constant.
       */
      if (depth == kMaxFormulaDepth) {
        if (constants.empty()) throw runtime_error("Formula needs a constant or variable, but none exist.");
        return constants[randomBetween(0, constants.size() - 1)];
      }

      /* Recursive case: We can pick any constant/variable, or any function. */
      size_t numOptions = constants.size() + c.functions.size();
      if (numOptions == 0) throw runtime_error("Cannot make value expression; no constants/variables/functions.");

      size_t option = randomBetween(0, numOptions - 1);
      if (option < constants.size()) return constants[option];

      /* Otherwise, make a random function call. */
      auto itr = next(c.functions.begin(), option - constants.size());

      vector<shared_ptr<ValueExpression>> args;
      for (size_t i = 0; i < itr->second.arity; i++) {
        args.push_back(randomValueExpression(c, depth + 1));
      }
      return make_shared<FunctionExpression>(itr->first, args, itr->second.callback);
    }
  }

  shared_ptr<BooleanExpression> buildExpressionFor(shared_ptr<ASTNode> ast,
                                                   const BuildContext& context) {
    FullBuildContext fullContext;
    fullContext.constants  = context.constants;
    fullContext.predicates = context.predicates;
    fullContext.functions  = context.functions;

    return asBoolean(convert(ast, fullContext));
  }

  /* Generates a random formula. The quality of this formula isn't guaranteed to be
   * very high. :-)
   */
  shared_ptr<BooleanExpression> randomExpression(const BuildContext& context) {
    FullBuildContext fullContext;
    fullContext.constants  = context.constants;
    fullContext.predicates = context.predicates;
    fullContext.functions  = context.functions;
    return randomBooleanExpression(fullContext, 0);
  }
}
