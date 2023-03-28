#pragma once

/* Routines to decompress a compressed tests file into a pair
 * of positive and negative world examples.
 */
#include "Logic/Entity.h"
#include <istream>
#include <utility>
#include <vector>

namespace Decompressor {
    std::pair<std::vector<World>, std::vector<World>> parse(std::istream& in);
}
