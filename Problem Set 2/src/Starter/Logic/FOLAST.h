/**
 * FOL abstract syntax tree. This is the direct output of the parser.
 * It is possible that the AST is not semantically valid; to 
 * actually evaluate ASTs, first convert them to syntax trees.
 */

#ifndef FOLAST_Included
#define FOLAST_Included

#include <ostream>
#include <memory>
#include <vector>

namespace FOL {
  class ASTVisitor;

  class ASTNode {
  public:
    virtual ~ASTNode() = default;

    virtual void print(std::ostream &) const = 0;
    virtual void accept(ASTVisitor* visitor);
  };

  std::ostream& operator<< (std::ostream& out, const ASTNode& node);

  class TrueNode: public ASTNode {
  public:
    TrueNode() = default;

    void print(std::ostream& out) const override {
      out << "T";
    };
    void accept(ASTVisitor* visitor) override;
  };

  class FalseNode: public ASTNode {
  public:
    FalseNode() = default;

    void print(std::ostream& out) const override {
      out << "F";
    };

    void accept(ASTVisitor* visitor) override;
  };

  class NotNode: public ASTNode {
  public:
    NotNode(std::shared_ptr<ASTNode> underlying) : underlying(underlying) {}

    void print(std::ostream& out) const override {
      out << "~" << *underlying;
    };
    void accept(ASTVisitor* visitor) override;

    std::shared_ptr<ASTNode> underlying;
  };

  class AndNode: public ASTNode {
  public:
    AndNode(std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override {
      out << "(" << *lhs << " /\\ " << *rhs << ")";
    };

    void accept(ASTVisitor* visitor) override;
    std::shared_ptr<ASTNode> lhs, rhs;
  };

  class OrNode: public ASTNode {
  public:
    OrNode(std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override {
      out << "(" << *lhs << " \\/ " << *rhs << ")";
    };

    void accept(ASTVisitor* visitor) override;
    std::shared_ptr<ASTNode> lhs, rhs;
  };

  class ImpliesNode: public ASTNode {
  public:
    ImpliesNode(std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override {
      out << "(" << *lhs << " -> " << *rhs << ")";
    };

    void accept(ASTVisitor* visitor) override;
    std::shared_ptr<ASTNode> lhs, rhs;
  };

  class IffNode: public ASTNode {
  public:
    IffNode(std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override {
      out << "(" << *lhs << " <-> " << *rhs << ")";
    };

    void accept(ASTVisitor* visitor) override;
    std::shared_ptr<ASTNode> lhs, rhs;
  };

  class EqualsNode: public ASTNode {
  public:
    EqualsNode(std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override {
      out << "(" << *lhs << " == " << *rhs << ")";
    };

    void accept(ASTVisitor* visitor) override;
    std::shared_ptr<ASTNode> lhs, rhs;
  };

  class NotEqualsNode: public ASTNode {
  public:
    NotEqualsNode(std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs) : lhs(lhs), rhs(rhs) {}

    void print(std::ostream& out) const override {
      out << "(" << *lhs << " != " << *rhs << ")";
    };

    void accept(ASTVisitor* visitor) override;
    std::shared_ptr<ASTNode> lhs, rhs;
  };


  class UniversalNode: public ASTNode {
  public:
    UniversalNode(const std::string& variable, std::shared_ptr<ASTNode> underlying) : variable(variable), underlying(underlying) {}

    void print(std::ostream& out) const override {
      out << "forall " << variable << ". " << *underlying;
    };

    void accept(ASTVisitor* visitor) override;
    std::string variable;
    std::shared_ptr<ASTNode> underlying;
  };

  class ExistentialNode: public ASTNode {
  public:
    ExistentialNode(const std::string& variable, std::shared_ptr<ASTNode> underlying) : variable(variable), underlying(underlying) {}

    void print(std::ostream& out) const override {
      out << "exists " << variable << ". " << *underlying;
    };

    void accept(ASTVisitor* visitor) override;
    std::string variable;
    std::shared_ptr<ASTNode> underlying;
  };

  class InvokeNode: public ASTNode {
  public:
    InvokeNode(const std::string& identifier, const std::vector<std::shared_ptr<ASTNode>>& args) : identifier(identifier), args(args) {}

    void print(std::ostream& out) const override {
      out << identifier << "(";
      for (size_t i = 0; i < args.size(); i++) {
        out << *args[i];
        if (i + 1 != args.size()) out << ", ";
      }
      out << ")";
    }

    void accept(ASTVisitor* visitor) override;
    std::string identifier;
    std::vector<std::shared_ptr<ASTNode>> args;
  };

  class VariableNode: public ASTNode {
  public:
    VariableNode(const std::string& name) : name(name) {}

    void print(std::ostream& out) const override {
      out << name;
    }

    void accept(ASTVisitor* visitor) override;
    std::string name;
  };

  class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;

    virtual void visit(const ASTNode&) {}
    virtual void visit(const TrueNode&) {}
    virtual void visit(const FalseNode&) {}
    virtual void visit(const NotNode&) {}
    virtual void visit(const AndNode&) {}
    virtual void visit(const OrNode&) {}
    virtual void visit(const ImpliesNode&) {}
    virtual void visit(const IffNode&) {}
    virtual void visit(const EqualsNode&) {}
    virtual void visit(const NotEqualsNode&) {}
    virtual void visit(const UniversalNode&) {}
    virtual void visit(const ExistentialNode&) {}
    virtual void visit(const InvokeNode&) {}
    virtual void visit(const VariableNode&) {}
  };
}

#endif
