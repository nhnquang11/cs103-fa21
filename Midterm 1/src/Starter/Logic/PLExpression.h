/**
 * Expression types for propositional logic.
 */

#ifndef PLExpression_Included
#define PLExpression_Included

#include <memory>
#include <string>
#include <ostream>
#include <unordered_map>
#include <vector>
#include <set>

namespace PL {
  class ExpressionVisitor;

  /* Evaluation context - map from variables to true/false. */
  using Context = std::unordered_map<std::string, bool>;

  /* Base Type */
  class Expression {
  public:
    virtual ~Expression() = default;

    virtual void print(std::ostream& out) const = 0;
    virtual void accept(ExpressionVisitor* visitor) const;
    virtual bool evaluate(const Context& context) const = 0;
  };
  std::ostream& operator<< (std::ostream& out, const Expression& e);

  /* Specific Expressions */

  class TrueExpression: public Expression {
  public:
    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;
  };

  class FalseExpression: public Expression {
  public:
    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;
  };

  class NotExpression: public Expression {
  public:
    NotExpression(std::shared_ptr<Expression> expr) : expr(expr) {}

    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;

    std::shared_ptr<Expression> underlying() const {
      return expr;
    }

  private:
    std::shared_ptr<Expression> expr;
  };

  class AndExpression: public Expression {
  public:
    AndExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;

    std::shared_ptr<Expression> left() const {
      return lhs;
    }
    std::shared_ptr<Expression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<Expression> lhs, rhs;
  };

  class OrExpression: public Expression {
  public:
    OrExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;

    std::shared_ptr<Expression> left() const {
      return lhs;
    }
    std::shared_ptr<Expression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<Expression> lhs, rhs;
  };

  class ImpliesExpression: public Expression {
  public:
    ImpliesExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;

    std::shared_ptr<Expression> left() const {
      return lhs;
    }
    std::shared_ptr<Expression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<Expression> lhs, rhs;
  };

  class IffExpression: public Expression {
  public:
    IffExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;

    std::shared_ptr<Expression> left() const {
      return lhs;
    }
    std::shared_ptr<Expression> right() const {
      return rhs;
    }

  private:
    std::shared_ptr<Expression> lhs, rhs;
  };

  class VariableExpression: public Expression {
  public:
    VariableExpression(const std::string& name) : name(name) {}

    void print(std::ostream& out) const override;
    void accept(ExpressionVisitor* visitor) const override;
    bool evaluate(const Context &) const override;

    std::string variableName() const {
      return name;
    }

  private:
    std::string name;
  };

  /* Visitor type. This will not walk the tree automatically; for that, use
   * ExpressionTreeWalker.
   */
  class ExpressionVisitor {
  public:
    virtual ~ExpressionVisitor() = default;

    virtual void visit(const Expression &) {}
    virtual void visit(const TrueExpression &) {}
    virtual void visit(const FalseExpression &) {}
    virtual void visit(const NotExpression &) {}
    virtual void visit(const AndExpression &) {}
    virtual void visit(const OrExpression &) {}
    virtual void visit(const ImpliesExpression &) {}
    virtual void visit(const IffExpression &) {}
    virtual void visit(const VariableExpression &) {}
  };

  /* Visitor subtype that walks the full tree automatically, calling into
   * specified callbacks.
   */
  class ExpressionTreeWalker: public ExpressionVisitor {
  public:
    virtual void visit(const Expression &) override final;
    virtual void visit(const TrueExpression &) override final;
    virtual void visit(const FalseExpression &) override final;
    virtual void visit(const NotExpression &) override final;
    virtual void visit(const AndExpression &) override final;
    virtual void visit(const OrExpression &) override final;
    virtual void visit(const ImpliesExpression &) override final;
    virtual void visit(const IffExpression &) override final;
    virtual void visit(const VariableExpression &) override final;

  protected:
    virtual void handle(const TrueExpression &) {}
    virtual void handle(const FalseExpression &) {}
    virtual void handle(const NotExpression &) {}
    virtual void handle(const AndExpression &) {}
    virtual void handle(const OrExpression &) {}
    virtual void handle(const ImpliesExpression &) {}
    virtual void handle(const IffExpression &) {}
    virtual void handle(const VariableExpression &) {}
  };

  /* General PL utilities. */
  using Formula = std::shared_ptr<Expression>;

  /* Returns the set of variables used in the given PL formula. */
  std::set<std::string> variablesIn(Formula f);

  /* Produces a truth table for the given formula. The variables are
   * implicitly ordered in the same ordering as what's given by
   * variablesIn.
   */
  std::vector<std::pair<std::vector<bool>, bool>> truthTableFor(Formula f);
}

#endif
