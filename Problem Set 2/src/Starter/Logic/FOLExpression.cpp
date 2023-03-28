#include "FOLExpression.h"
using namespace std;

namespace FOL {
  ostream& operator<< (ostream& out, const Expression& e) {
    e.print(out);
    return out;
  }

  void Expression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }

  /* Kick off the recursion! */
  bool BooleanExpression::evaluate(const World& world) const {
    Context c;
    c.world = world;
    return evaluate(c);
  }

  void BooleanExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }

  Entity ValueExpression::evaluate(const World& world) const {
    Context c;
    c.world = world;
    return evaluate(c);
  }

  void ValueExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool TrueExpression::evaluate(Context &) const {
    return true;
  }
  void TrueExpression::print(ostream& out) const {
    out << "⊤";
  }
  void TrueExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool FalseExpression::evaluate(Context &) const {
    return false;
  }
  void FalseExpression::print(ostream& out) const {
    out << "⊥";
  }
  void FalseExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool NotExpression::evaluate(Context& c) const {
    return !expr->evaluate(c);
  }
  void NotExpression::print(ostream& out) const {
    out << "¬" << *expr;
  }
  void NotExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool AndExpression::evaluate(Context& c) const {
    return lhs->evaluate(c) && rhs->evaluate(c);
  }
  void AndExpression::print(ostream& out) const {
    out << "(" << *lhs << " ∧ " << *rhs << ")";
  }
  void AndExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool OrExpression::evaluate(Context& c) const {
    return lhs->evaluate(c) || rhs->evaluate(c);
  }
  void OrExpression::print(ostream& out) const {
    out << "(" << *lhs << " ∨ " << *rhs << ")";
  }
  void OrExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool ImpliesExpression::evaluate(Context& c) const {
    return !lhs->evaluate(c) || rhs->evaluate(c);
  }
  void ImpliesExpression::print(ostream& out) const {
    out << "(" << *lhs << " → " << *rhs << ")";
  }
  void ImpliesExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool IffExpression::evaluate(Context& c) const {
    return lhs->evaluate(c) == rhs->evaluate(c);
  }
  void IffExpression::print(ostream& out) const {
    out << "(" << *lhs << " ↔ " << *rhs << ")";
  }
  void IffExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool EqualsExpression::evaluate(Context& c) const {
    return lhs->evaluate(c) == rhs->evaluate(c);
  }
  void EqualsExpression::print(ostream& out) const {
    out << "(" << *lhs << " = " << *rhs << ")";
  }
  void EqualsExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool NotEqualsExpression::evaluate(Context& c) const {
    return lhs->evaluate(c) != rhs->evaluate(c);
  }
  void NotEqualsExpression::print(ostream& out) const {
    out << "(" << *lhs << " ≠ " << *rhs << ")";
  }
  void NotEqualsExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool UniversalExpression::evaluate(Context& c) const {
    for (const auto& entity: c.world) {
      c.entities[variable] = entity;
      if (!expr->evaluate(c)) {
        c.entities.erase(variable);
        return false;
      }
    }

    c.entities.erase(variable);
    return true;
  }
  void UniversalExpression::print(ostream& out) const {
    out << "∀" << variable << ". " << *expr;
  }
  void UniversalExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool ExistentialExpression::evaluate(Context& c) const {
    for (const auto& entity: c.world) {
      c.entities[variable] = entity;
      if (expr->evaluate(c)) {
        c.entities.erase(variable);
        return true;
      }
    }

    c.entities.erase(variable);
    return false;
  }
  void ExistentialExpression::print(ostream& out) const {
    out << "∃" << variable << ". " << *expr;
  }
  void ExistentialExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  Entity ConstantExpression::evaluate(Context&) const {
    return e;
  }
  void ConstantExpression::print(ostream& out) const {
    out << name;
  }
  void ConstantExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  Entity VariableExpression::evaluate(Context& c) const {
    return c.entities.at(name);
  }
  void VariableExpression::print(ostream& out) const {
    out << name;
  }
  void VariableExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  bool PredicateExpression::evaluate(Context& c) const {
    vector<Entity> arguments;
    for (auto arg: args) {
      arguments.push_back(arg->evaluate(c));
    }
    return pred(arguments);
  }
  void PredicateExpression::print(ostream& out) const {
    out << name << "(";

    for (size_t i = 0; i < args.size(); i++) {
      out << *args[i];
      if (i + 1 != args.size()) out << ", ";
    }

    out << ")";
  }
  void PredicateExpression::accept(ExpressionVisitor* visitor) const {
    visitor->visit(*this);
  }


  Entity FunctionExpression::evaluate(Context& c) const {
    vector<Entity> arguments;
    for (auto arg: args) {
      arguments.push_back(arg->evaluate(c));
    }
    return fn(arguments);
  }
  void FunctionExpression::print(ostream& out) const {
    out << name << "(";

    for (size_t i = 0; i < args.size(); i++) {
      out << *args[i];
      if (i + 1 != args.size()) out << ", ";
    }

    out << ")";
  }
  void FunctionExpression::accept(ExpressionVisitor* visitor) const {
      visitor->visit(*this);
  }


  /* ExpressionTreeWalker implementation. */
  void ExpressionTreeWalker::visit(const Expression &) {
    throw runtime_error("Unknown expression type.");
  }
  void ExpressionTreeWalker::visit(const ValueExpression &) {
    throw runtime_error("Unknown value expression type.");
  }
  void ExpressionTreeWalker::visit(const BooleanExpression &) {
    throw runtime_error("Unknown boolean expression type.");
  }
  void ExpressionTreeWalker::visit(const TrueExpression& e) {
    handle(e);
  }
  void ExpressionTreeWalker::visit(const FalseExpression& e) {
    handle(e);
  }
  void ExpressionTreeWalker::visit(const ConstantExpression& e) {
    handle(e);
  }
  void ExpressionTreeWalker::visit(const VariableExpression& e) {
    handle(e);
  }
  void ExpressionTreeWalker::visit(const NotExpression& e) {
    handle(e);
    e.underlying()->accept(this);
  }
  void ExpressionTreeWalker::visit(const UniversalExpression& e) {
    handle(e);
    e.underlying()->accept(this);
  }
  void ExpressionTreeWalker::visit(const ExistentialExpression& e) {
    handle(e);
    e.underlying()->accept(this);
  }
  void ExpressionTreeWalker::visit(const AndExpression& e) {
    handle(e);
    e.left()->accept(this);
    e.right()->accept(this);
  }
  void ExpressionTreeWalker::visit(const OrExpression& e) {
    handle(e);
    e.left()->accept(this);
    e.right()->accept(this);
  }
  void ExpressionTreeWalker::visit(const ImpliesExpression& e) {
    handle(e);
    e.left()->accept(this);
    e.right()->accept(this);
  }
  void ExpressionTreeWalker::visit(const IffExpression& e) {
    handle(e);
    e.left()->accept(this);
    e.right()->accept(this);
  }
  void ExpressionTreeWalker::visit(const EqualsExpression& e) {
    handle(e);
    e.left()->accept(this);
    e.right()->accept(this);
  }
  void ExpressionTreeWalker::visit(const NotEqualsExpression& e) {
    handle(e);
    e.left()->accept(this);
    e.right()->accept(this);
  }
  void ExpressionTreeWalker::visit(const PredicateExpression& e) {
    handle(e);
    for (const auto& subexpr: e.arguments()) {
        subexpr->accept(this);
    }
  }
  void ExpressionTreeWalker::visit(const FunctionExpression& e) {
    handle(e);
    for (const auto& subexpr: e.arguments()) {
        subexpr->accept(this);
    }
  }
}
