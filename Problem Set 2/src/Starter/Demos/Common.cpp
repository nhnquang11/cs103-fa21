#include "Common.h"
#include "strlib.h"
#include "simpio.h"
#include <map>
using namespace std;

namespace LogicUI {
    namespace {
        /* Given a string of the form [Part (___)], isolates ___. */
        string romanFrom(const string& str) {
            size_t start = str.find('(');
            if (start == string::npos) throw runtime_error("Can't find open parenthesis.");

            size_t end   = str.find(')', start);
            if (end == string::npos) throw runtime_error("Can't find close parenthesis.");

            return string(str.begin() + start + 1, str.begin() + end);
        }

        /* Map from subtractive pairs to their additive equivalents. */
        vector<pair<string, string>> kSubtractives = {
            { "CM", "DCCCC" },
            { "CD", "CCCC"  },
            { "XC", "LXXXX" },
            { "XL", "XXXX"  },
            { "IX", "VIIII" },
            { "IV", "IIII"  }
        };

        /* Replaces all subtractives with their additive equivalents. */
        string replaceSubtractivesIn(const string& str) {
            string result = str;
            for (const auto& entry: kSubtractives) {
                size_t index = result.find(entry.first);
                if (index != string::npos) {
                    result.erase(index, entry.first.size());
                    result.insert(index, entry.second);
                }
            }
            return result;
        }

        /* Map from characters to values. */
        map<char, int> kValues = {
            { 'M', 1000 },
            { 'D', 500  },
            { 'C', 100  },
            { 'L', 50   },
            { 'X', 10   },
            { 'V', 5    },
            { 'I', 1    }
        };

        /* Additive form to number. */
        int valueOf(const string& str) {
            int result = 0;
            for (char ch: str) {
                result += kValues.at(ch);
            }
            return result;
        }

        /* Number to Roman numeral. Works up to 100. */
        string convertDigit(size_t digit, char one, char five, char ten) {
            if (digit <= 3) return string(digit, one);
            if (digit == 4) return string(1, one) + string(1, five);
            if (digit <= 8) return string(1, five) + string(digit - 5, one);
            return string(1, one) + string(1, ten);
        }
    }

    string toRoman(size_t number) {
        if (number == 0 || number >= 100) {
            error("This function only works on values between 1 and 99, inclusive.");
        }

        return convertDigit(number / 10, 'x', 'l', 'c') + convertDigit(number % 10, 'i', 'v', 'x');
    }

    bool compareRoman(const string& lhs, const string& rhs) {
        /* Isolate the relevant sections of each string. */
        int romanL = valueOf(replaceSubtractivesIn(toUpperCase(romanFrom(lhs))));
        int romanR = valueOf(replaceSubtractivesIn(toUpperCase(romanFrom(rhs))));

        return romanL < romanR;
    }

    int romanToInt(const string& value) {
        return valueOf(replaceSubtractivesIn(toUpperCase(trim(value))));
    }

    /* Works from 1 to 99. */
    bool isRomanNumeral(const string& str) {
        string shouldBeSorted = replaceSubtractivesIn(toUpperCase(trim(str)));

        /* Confirm everything is sorted numerically and that there aren't runs of five
         * or more identical characters.
         */
        char lastChar = 'M'; // Pretend it's M
        int lastFreq = 0;    // But we haven't seen any.
        for (char ch: shouldBeSorted) {
            if (!kValues.count(ch)) return false; // Not a letter
            if (ch == lastChar) {
                lastFreq++;
                /* Okay, this is technically wrong. It allows for things like iiii for
                 * 4. But you know what? I'm okay with that, and I have some higher-
                 * priority tasks right now (9/22/21). So please feel free to fix this
                 * if you'd like. :-)
                 */
                if (lastFreq == 5) return false;
            } else {
                if (kValues.at(ch) > kValues.at(lastChar)) return false; // Values increased
                lastChar = ch;
                lastFreq = 1;
            }
        }
        return true;
    }

    int getIntegerRoman(const string& prompt, int low, int high) {
        while (true) {
            string input = trim(getLine(prompt));

            if (input.empty()) {
                cerr << "Please enter a number." << endl;
                continue;
            }

            int result;

            /* Could be a Hindu-Arabic number. */
            if (isdigit(input[0])) {
                try {
                    result = stringToInteger(input);
                } catch (const exception &) {
                    cerr << "Please enter a number." << endl;
                    continue;
                }
            }
            /* Could be Roman numerals. */
            else if (isRomanNumeral(input)) {
                result = romanToInt(input);
            }
            /* Or it's just bad. :-) */
            else {
                cerr << "Please enter a number." << endl;
                continue;
            }

            /* Range check. */
            if (result >= low && result <= high) {
                return result;
            } else {
                cerr << "Please enter a number between " << low << " and " << high << endl;
            }
        }
    }
}
