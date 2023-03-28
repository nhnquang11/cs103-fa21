#pragma once

/* How many languages are in the Lava Diagram. */
const size_t kNumLanguages = 12;

/* Locations where the languages can be. */
enum class LangLoc {
    UNSELECTED,
    REG,
    R,
    RE,
    ALL
};
