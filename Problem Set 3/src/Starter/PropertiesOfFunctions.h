#pragma once

#include <cstddef>

/* How many functions are in the Venn Diagram. */
const std::size_t kNumFunctions = 15;

/* Locations where the purported functions can be. */
enum class FnLoc {
    UNSELECTED,
    NOT_A_FUNCTION,
    FUNCTION,
    INJECTION,
    SURJECTION,
    BIJECTION
};
