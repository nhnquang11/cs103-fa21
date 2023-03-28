/* String utility functions used in several different places. These are all
 * taken from StanfordCPPLib and enclosed in their own namespace so that
 * there aren't any conflicts.
 */
#pragma once

#include <string>
#include <vector>

#include <cctype>
#include <algorithm>

/* All code here, except for the code for error, stolen or adapted from strlib.cpp. */

namespace Utilities {
    namespace Impl {
        inline void trimEndInPlace(std::string& str) {
            int end = (int) str.length();
            int finish = end;
            while (finish > 0 && isspace(str[finish - 1])) {
                finish--;
            }
            if (finish < end) {
                str.erase(finish, end - finish);
            }
        }

        inline void trimStartInPlace(std::string& str) {
            int start = 0;
            int finish = (int) str.length() - 1;
            while (start <= finish && isspace(str[start])) {
                start++;
            }
            if (start > 0) {
                str.erase(0, start);
            }
        }

        inline void trimInPlace(std::string& str) {
            trimEndInPlace(str);
            trimStartInPlace(str);
        }
    }

    inline bool startsWith(const std::string& str, char prefix) {
        return str.length() > 0 && str[0] == prefix;
    }

    inline bool startsWith(const std::string& str, const std::string& prefix) {
        if (str.length() < prefix.length()) return false;
        int nChars = prefix.length();
        for (int i = 0; i < nChars; i++) {
            if (str[i] != prefix[i]) return false;
        }
        return true;
    }

    inline bool endsWith(const std::string& str, const std::string& suffix) {
        /* This string ends with this suffix if the reverse of the string
         * ends with the reverse of the suffix.
         */
        std::string revStr(str.rbegin(), str.rend());
        std::string revSuffix(suffix.rbegin(), suffix.rend());
        return startsWith(revStr, revSuffix);
    }

    inline std::string trim(const std::string& str) {
        std::string str2 = str;
        Impl::trimInPlace(str2);
        return str2;
    }

    inline std::vector<std::string> stringSplit(const std::string& str, char delimiter) {
        std::string str2 = str;
        std::vector<std::string> result;

        size_t index = 0;
        while (true) {
            index = str2.find(delimiter);
            if (index == std::string::npos) {
                break;
            }
            result.push_back(str2.substr(0, index));
            str2.erase(str2.begin(), str2.begin() + index + 1);
        }
        if (!str2.empty()) {
            result.push_back(str2);
        }

        return result;
    }
}
