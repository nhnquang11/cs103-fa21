#ifndef IntDynParser_Included
#define IntDynParser_Included

#include "IntDynScanner.h"
#include <queue>
#include <utility>
#include <stdexcept>


namespace IntDyn {
    std::vector<std::pair<std::string, std::string>> parse(std::queue<Token>& q);
    std::vector<std::pair<std::string, std::string>> parse(std::queue<Token>&& q);
}

#endif
