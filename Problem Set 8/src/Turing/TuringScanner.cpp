#include "TuringScanner.h"
#include "Utilities/Unicode.h"
#include "StrUtils/StrUtils.h"
#include <unordered_map>
#include <sstream>
using namespace std;

namespace Turing {
    namespace {
        const unordered_map<string, TokenType> kTokens = {
            { "Move",         TokenType::MOVE   },
            { "Left",         TokenType::LEFT   },
            { "Right",        TokenType::RIGHT  },
            { "Goto",         TokenType::GOTO   },
            { "Write",        TokenType::PRINT  },
            { "Blank",        TokenType::BLANK  },
            { "True",         TokenType::TRUE   },
            { "False",        TokenType::FALSE  },
            { "Return",       TokenType::RETURN },
            { "If",           TokenType::IF     },
            { "Not",          TokenType::NOT    },
            { ":",            TokenType::COLON  },
        };

        /* Replacements for <cctype>, given that we're working with
         * Unicode characters.
         */
        bool isAlphabetic(char32_t ch) {
            return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
        }
        bool isAlphanumeric(char32_t ch) {
            return isAlphabetic(ch) || (ch >= '0' && ch <= '9');
        }
        bool isASCII(char32_t ch) {
            return 0 <= static_cast<int>(ch) && ch <= 127;
        }
        bool isSpace(char32_t ch) {
            return isASCII(ch) && isspace(static_cast<int>(ch));
        }
        bool isQuote(char32_t ch) {
            return ch == '\'' ||
                   ch == fromUTF8("‘") ||
                   ch == fromUTF8("’");
        }

        /* Given a character, could it start an identifier?
         *
         * Identifiers must start with a letter or underscore.
         */
        bool isPossibleIdentifier(char32_t ch) {
            return ch == '_' || isAlphabetic(ch);
        }

        /* Scans a character. We should see '[ch]'. */
        void scanCharacter(queue<Token>& result, istream& input) {
            if (!isQuote(readChar(input))) throw runtime_error("Expected a single quote.");
            char32_t payload = readChar(input);
            if (!isQuote(readChar(input))) throw runtime_error("Expected a single quote.");

            result.push({ TokenType::CHAR, toUTF8(payload) });
        }

        /* Scans something from the input that may be an identifier. It's possible that
         * we aren't looking at an identifier here and that we have a word like "move"
         * or "left." Therefore, we'll use max-munch to pull out the longest identifier
         * name we can get, then do some context-dependent transforms on it.
         */
        void scanPossibleIdentifier(queue<Token>& result, istream& input) {
            /* Grab the first character, which we know starts an identifier. */
            string token = toUTF8(readChar(input));

            while (input.peek() != EOF) {
                char32_t next = peekChar(input);

                /* If this can extend our identifier, do so. */
                if (isAlphanumeric(next) || next == '_') {
                    token += toUTF8(readChar(input));
                }

                /* Otherwise, we're done. */
                else break;
            }

            /* Decode what we have. That is, if it's a known token identifier, use that,
             * and otherwise make it an identifier.
             */
            auto itr = kTokens.find(token);
            if (itr != kTokens.end()) {
                result.push( { itr->second, token } );
            } else {
                result.push( { TokenType::LABEL, token } );
            }
        }

        /* Returns whether any token begins with the specified pattern. */
        bool someTokenStartsWith(const string& soFar) {
            for (const auto& token: kTokens) {
                if (Utilities::startsWith(token.first, soFar)) return true;
            }
            return false;
        }

        /* Scans until either (1) a complete symbol is found or (2) the input is
         * consumed. In the first case, great! We return what token we got. In the
         * second case, we report an error, because we don't know what to do.
         */
        void scanSymbol(queue<Token>& result, istream& input) {
            /* Grab the first character, which we know exists. */
            string token = toUTF8(readChar(input));

            /* Keep extending until we find something that works. */
            while (!kTokens.count(token)) {
                /* If no token starts with the characters we have seen so far, we can
                 * stop scanning and report an error. Similarly, if we're out of
                 * character, we should stop.
                 */
                if (!someTokenStartsWith(token) || input.peek() == EOF) {
                    throw runtime_error("Unexpected character sequence: " + token);
                }

                /* Otherwise, extend and hope we'll get to something later. */
                token += toUTF8(readChar(input));
            }

            /* Now, use maximal munch. Keep appending until we find something that doesn't
             * work or until we run out of characters.
             */
            while (input.peek() != EOF && someTokenStartsWith(token + toUTF8(peekChar(input)))) {
                token += toUTF8(readChar(input));
            }

            /* One of two things happened at this point.
             *
             * 1. We successfully read a token. If so, great! Return it.
             * 2. We didn't read a token. That means a prefix of what we read is a token and
             *    the longest one is what we want to return.
             */
            while (!kTokens.count(token)) {
                if (!input.unget()) throw runtime_error("Can't rewind stream.");
                token.pop_back();
            }

            result.push({ kTokens.at(token), token });
        }
    }

    queue<Token> scan(istream& input) {
        queue<Token> result;
        while (input.peek() != EOF) {
            /* Grab the next character to see what to do with it. */
            char32_t next = peekChar(input);

            /* Skip whitespace. */
            if (isSpace(next)) {
                (void) readChar(input);
            }
            /* See quotes? That means it's a character name. */
            else if (isQuote(next)) {
                scanCharacter(result, input);
            }
            /* See something alphanumeric? It might be a label, or it might be
             * the name of a keyword.
             */
            else if (isPossibleIdentifier(next)) {
                scanPossibleIdentifier(result, input);
            }
            /* Otherwise, scan it as though it's a collection of symbols. */
            else {
                scanSymbol(result, input);
            }
        }

        /* Tack on an EOF marker. */
        result.push({ TokenType::SCAN_EOF, "(EOF)" });
        return result;
    }

    queue<Token> scan(const string& sourceText) {
        istringstream extractor(sourceText);
        return scan(extractor);
    }

    string to_string(const Token& t) {
        return t.data;
    }
}
