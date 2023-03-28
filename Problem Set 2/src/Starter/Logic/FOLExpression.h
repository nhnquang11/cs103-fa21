/**
 * Expression types for first-order logic expressions.
 *
 * Unlike an AST, which is essentially a slightly cleaned-up parse tree,
 * an expression tree can be used to actually evaluate first-order logic
 * formulas.
 *
 * There are two key expression types. The first is BooleanExpression,
 * which represents an expression that evaluates to a boolean value (basically,
 * a first-order sentence). The other is ValueExpression, which evaluates to
 * a value.
 */

#ifndef FOLExpression_Included
#define FOLExpression_Included

#include "Entity.h"
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <ostream>
#include <functional>

namespace FOL {
  class ExpressionVisitor;

  /* Base Types */

  class Expression {
  public:
    virtual ~Expression() = default;

    virtual void print(std::ostream& out) const = 0;
    virtual void accept(ExpressionVisitor* visitor) const;

  protected:
    /* Evaluation context. */
    struct Context {
      World world;
      std::map<std::string, Entity> entities;
    };
  };
  std::ostream& operator<< (std::ostream& out, const Expression& e);

  class BooleanExpression: public Expression {
  public:
    bool evaluate(const World& world) const;
    virtual bool evaluate(Context& world) const = 0;
    virtual void accept(ExpressionVisitor* visitor) const override;
  };

  class ValueExpression: public Expression {
  public:
    Entity evaluate(const World& world) const;
    virtual Entity evaluate(Context& world) const = 0;
    virtual void accept(ExpressionVisitor* visitor) const override;
  };

  /* Specific Expressions */

  class TrueExpression: public BooleanExpression {
  public:
    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;
  };

  class FalseExpression: public BooleanExpression {
  public:
    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;
  };

  class NotExpression: public BooleanExpression {
  public:
    NotExpression(std::shared_ptr<BooleanExpression> expr) : expr(expr) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::shared_ptr<BooleanExpression> underlying() const {
      return expr;
    }

  private:
    std::shared_ptr<BooleanExpression> expr;
  };

  class AndExpression: public BooleanExpression {
  public:
    AndExpression(std::shared_ptr<BooleanExpression> lhs, std::shared_ptr<BooleanExpression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::shared_ptr<BooleanExpression> left() const {
      return lhs;
    }
    std::shared_ptr<BooleanExpression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<BooleanExpression> lhs, rhs;
  };

  class OrExpression: public BooleanExpression {
  public:
    OrExpression(std::shared_ptr<BooleanExpression> lhs, std::shared_ptr<BooleanExpression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::shared_ptr<BooleanExpression> left() const {
      return lhs;
    }
    std::shared_ptr<BooleanExpression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<BooleanExpression> lhs, rhs;
  };

  class ImpliesExpression: public BooleanExpression {
  public:
    ImpliesExpression(std::shared_ptr<BooleanExpression> lhs, std::shared_ptr<BooleanExpression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::shared_ptr<BooleanExpression> left() const {
      return lhs;
    }
    std::shared_ptr<BooleanExpression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<BooleanExpression> lhs, rhs;
  };

  class IffExpression: public BooleanExpression {
  public:
    IffExpression(std::shared_ptr<BooleanExpression> lhs, std::shared_ptr<BooleanExpression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::shared_ptr<BooleanExpression> left() const {
      return lhs;
    }
    std::shared_ptr<BooleanExpression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<BooleanExpression> lhs, rhs;
  };

  class EqualsExpression: public BooleanExpression {
  public:
    EqualsExpression(std::shared_ptr<ValueExpression> lhs, std::shared_ptr<ValueExpression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::shared_ptr<ValueExpression> left() const {
      return lhs;
    }
    std::shared_ptr<ValueExpression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<ValueExpression> lhs, rhs;
  };

  class NotEqualsExpression: public BooleanExpression {
  public:
    NotEqualsExpression(std::shared_ptr<ValueExpression> lhs, std::shared_ptr<ValueExpression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::shared_ptr<ValueExpression> left() const {
      return lhs;
    }
    std::shared_ptr<ValueExpression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<ValueExpression> lhs, rhs;
  };

  class UniversalExpression: public BooleanExpression {
  public:
    UniversalExpression(const std::string& variable, std::shared_ptr<BooleanExpression> expr) : variable(variable), expr(expr) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::string quantifiedVariable() const {
      return variable;
    }

    std::shared_ptr<BooleanExpression> underlying() const {
      return expr;
    }

  private:
    std::string variable;
    std::shared_ptr<BooleanExpression> expr;
  };

  class ExistentialExpression: public BooleanExpression {
  public:
    ExistentialExpression(const std::string& variable, std::shared_ptr<BooleanExpression> expr)  : variable(variable), expr(expr) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::string quantifiedVariable() const {
      return variable;
    }

    std::shared_ptr<BooleanExpression> underlying() const {
      return expr;
    }

  private:
    std::string variable;
    std::shared_ptr<BooleanExpression> expr;
  };

  using FOLPredicate = std::function<bool (std::vector<Entity>)>;

  class PredicateExpression: public BooleanExpression {
  public:
    PredicateExpression(const std::string& name,
                        std::vector<std::shared_ptr<ValueExpression>> args,
                        FOLPredicate pred) : name(name), args(args), pred(pred) {}

    void print(std::ostream& out) const override;
    bool evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::string predicateName() const {
      return name;
    }

    std::vector<std::shared_ptr<ValueExpression>> arguments() const {
      return args;
    }

  private:
    std::string name;
    std::vector<std::shared_ptr<ValueExpression>> args;
    FOLPredicate pred;
  };

  using FOLFunction = std::function<Entity (std::vector<Entity>)>;

  class FunctionExpression: public ValueExpression {
  public:
    FunctionExpression(const std::string& name,
                       std::vector<std::shared_ptr<ValueExpression>> args,
                       FOLFunction fn) : name(name), args(args), fn(fn) {}

    void print(std::ostream& out) const override;
    Entity evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::string functionName() const {
      return name;
    }

    std::vector<std::shared_ptr<ValueExpression>> arguments() const {
      return args;
    }

  private:
    std::string name;
    std::vector<std::shared_ptr<ValueExpression>> args;
    FOLFunction fn;
  };

  class VariableExpression: public ValueExpression {
  public:
    VariableExpression(const std::string& name) : name(name) {}

    void print(std::ostream& out) const override;
    Entity evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::string variableName() const {
      return name;
    }

  private:
    std::string name;
  };

  class ConstantExpression: public ValueExpression {
  public:
    ConstantExpression(const std::string& name, Entity e) : name(name), e(e) {}

    void print(std::ostream& out) const override;
    Entity evaluate(Context&) const override;
    virtual void accept(ExpressionVisitor* visitor) const override;

    std::string constantName() const {
      return name;
    }
    Entity entity() const {
      return e;
    }

  private:
    std::string name;
    Entity e;
  };

  /* Visitor type. This will not walk the tree automatically; for that, use
   * ExpressionTreeWalker.
   */
  class ExpressionVisitor {
  public:
    virtual ~ExpressionVisitor() = default;

    virtual void visit(const Expression &) {}
    virtual void visit(const BooleanExpression &) {}
    virtual void visit(const ValueExpression &) {}
    virtual void visit(const TrueExpression &) {}
    virtual void visit(const FalseExpression &) {}
    virtual void visit(const NotExpression &) {}
    virtual void visit(const AndExpression &) {}
    virtual void visit(const OrExpression &) {}
    virtual void visit(const ImpliesExpression &) {}
    virtual void visit(const IffExpression &) {}
    virtual void visit(const EqualsExpression &) {}
    virtual void visit(const NotEqualsExpression &) {}
    virtual void visit(const UniversalExpression &) {}
    virtual void visit(const ExistentialExpression &) {}
    virtual void visit(const PredicateExpression &) {}
    virtual void visit(const FunctionExpression &) {}
    virtual void visit(const ConstantExpression &) {}
    virtual void visit(const VariableExpression &) {}
  };

  /* Visitor subtype that walks the full tree automatically, calling into
   * specified callbacks.
   */
  class ExpressionTreeWalker: public ExpressionVisitor {
  public:
    virtual void visit(const Expression &) override final;
    virtual void visit(const BooleanExpression &) override final;
    virtual void visit(const ValueExpression &) override final;
    virtual void visit(const TrueExpression &) override final;
    virtual void visit(const FalseExpression &) override final;
    virtual void visit(const NotExpression &) override final;
    virtual void visit(const AndExpression &) override final;
    virtual void visit(const OrExpression &) override final;
    virtual void visit(const ImpliesExpression &) override final;
    virtual void visit(const IffExpression &) override final;
    virtual void visit(const EqualsExpression &) override final;
    virtual void visit(const NotEqualsExpression &) override final;
    virtual void visit(const UniversalExpression &) override final;
    virtual void visit(const ExistentialExpression &) override final;
    virtual void visit(const PredicateExpression &) override final;
    virtual void visit(const FunctionExpression &) override final;
    virtual void visit(const ConstantExpression &) override final;
    virtual void visit(const VariableExpression &) override final;

  protected:
    virtual void handle(const TrueExpression &) {}
    virtual void handle(const FalseExpression &) {}
    virtual void handle(const NotExpression &) {}
    virtual void handle(const AndExpression &) {}
    virtual void handle(const OrExpression &) {}
    virtual void handle(const ImpliesExpression &) {}
    virtual void handle(const IffExpression &) {}
    virtual void handle(const EqualsExpression &) {}
    virtual void handle(const NotEqualsExpression &) {}
    virtual void handle(const UniversalExpression &) {}
    virtual void handle(const ExistentialExpression &) {}
    virtual void handle(const PredicateExpression &) {}
    virtual void handle(const FunctionExpression &) {}
    virtual void handle(const ConstantExpression &) {}
    virtual void handle(const VariableExpression &) {}
  };
}

#endif
