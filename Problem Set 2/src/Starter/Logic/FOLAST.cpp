#include "FOLAST.h"
using namespace std;

namespace FOL {
  ostream& operator<< (ostream& out, const ASTNode& node) {
    node.print(out);
    return out;
  }

  void ASTNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void TrueNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void FalseNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void AndNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void OrNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void NotNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void ImpliesNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void IffNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void EqualsNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void NotEqualsNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void UniversalNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void ExistentialNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void InvokeNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
  void VariableNode::accept(ASTVisitor* visitor) {
    visitor->visit(*this);
  }
}
