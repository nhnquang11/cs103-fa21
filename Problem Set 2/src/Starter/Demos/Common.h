#pragma once

#include <string>

/* Graphics and UI utilities. */
namespace LogicUI {
    /* Comparator for Roman numeral entries of the form [Part (___)] */
    bool compareRoman(const std::string& lhs, const std::string& rhs);

    /* Roman numeral to value. */
    int romanToInt(const std::string& romanNumerals);

    /* Integer to Roman numeral. Works up to 99. */
    std::string toRoman(size_t value);

    /* Reads a section from the console. The user can enter either Hindu-Arabic numerals
     * or Roman numerals.
     */
    int getIntegerRoman(const std::string& prompt, int lowInclusive, int highInclusive);
}
