#ifndef PLParser_Included
#define PLParser_Included

#include "PLScanner.h"
#include <queue>
#include <memory>
#include "PLExpression.h"


namespace PL {
    std::shared_ptr<Expression> parse(std::queue<Token>& q);
    std::shared_ptr<Expression> parse(std::queue<Token>&& q);
}

#endif
