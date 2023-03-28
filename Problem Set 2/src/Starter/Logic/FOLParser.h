#ifndef FOLParser_Included
#define FOLParser_Included

#include "FOLScanner.h"
#include <queue>
#include <memory>
#include "FOLAST.h"


namespace FOL {
    std::shared_ptr<ASTNode> parse(std::queue<Token>& q);
    std::shared_ptr<ASTNode> parse(std::queue<Token>&& q);
}

#endif
