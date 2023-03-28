#ifndef TuringParser_Included
#define TuringParser_Included

#include "TuringScanner.h"
#include <queue>
#include <memory>
#include "Turing.h"
#include "Utilities/Unicode.h"


namespace Turing {
    std::shared_ptr<Statement> parse(std::queue<Token>& q);
    std::shared_ptr<Statement> parse(std::queue<Token>&& q);
}

#endif
