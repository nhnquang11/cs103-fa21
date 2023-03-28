#include "LogicScanner.h"
#include "Utilities/Unicode.h"
#include "StrUtils/StrUtils.h"
#include <unordered_map>
#include <sstream>
using namespace std;

namespace Logic {
    namespace {
        const unordered_map<string, TokenType> kTokens = {
            { "~",                TokenType::NOT       },
            { "!",                TokenType::NOT       },
            { "¬",                TokenType::NOT       },
            { "\\lnot",           TokenType::NOT       },
            { "\\neg",            TokenType::NOT       },
            { "not",              TokenType::NOT       },
            { "/\\",              TokenType::AND       },
            { "&&",               TokenType::AND       },
            { "and",              TokenType::AND       },
            { "\\land",           TokenType::AND       },
            { "\\wedge",          TokenType::AND       },
            { "∧",                TokenType::AND       },
            { "^",                TokenType::AND       },
            { "\\/",              TokenType::OR        },
            { "||",               TokenType::OR        },
            { "or",               TokenType::OR        },
            { "\\lor",            TokenType::OR        },
            { "\\vee",            TokenType::OR        },
            { "∨",                TokenType::OR        },
            { "->",               TokenType::IMPLIES   },
            { "=>",               TokenType::IMPLIES   },
            { "implies",          TokenType::IMPLIES   },
            { "\\to",             TokenType::IMPLIES   },
            { "\\rightarrow",     TokenType::IMPLIES   },
            { "\\Rightarrow",     TokenType::IMPLIES   },
            { "→",                TokenType::IMPLIES   },
            { "<->",              TokenType::IFF       },
            { "<=>",              TokenType::IFF       },
            { "iff",              TokenType::IFF       },
            { "\\iff",            TokenType::IFF       },
            { "\\leftrightarrow", TokenType::IFF       },
            { "\\Leftrightarrow", TokenType::IFF       },
            { "↔",                TokenType::IFF       },
            { "T",                TokenType::TRUE      },
            { "true",             TokenType::TRUE      },
            { "True",             TokenType::TRUE      },
            { "\\top",            TokenType::TRUE      },
            { "⊤",                TokenType::TRUE      },
            { "F",                TokenType::FALSE     },
            { "false",            TokenType::FALSE     },
            { "False",            TokenType::FALSE     },
            { "\\bot",            TokenType::FALSE     },
            { "⊥",                TokenType::FALSE     },
            { "(",                TokenType::LPAREN    },
            { ")",                TokenType::RPAREN    },
            { ".",                TokenType::DOT       },
            { ",",                TokenType::COMMA     },
            { "\\forall",         TokenType::FORALL    },
            { "A",                TokenType::FORALL    },
            { "forall",           TokenType::FORALL    },
            { "∀",                TokenType::FORALL    },
            { "E",                TokenType::EXISTS    },
            { "\\exists",         TokenType::EXISTS    },
            { "exists",           TokenType::EXISTS    },
            { "∃",                TokenType::EXISTS    },
            { "=",                TokenType::EQUALS    },
            { "==",               TokenType::EQUALS    },
            { "!=",               TokenType::NOTEQUALS },
            { "\\ne",             TokenType::NOTEQUALS },
            { "≠",                TokenType::NOTEQUALS },
            { "\\neq",            TokenType::NOTEQUALS },
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

        /* Given a character, could it start an identifier?
         *
         * Identifiers must start with a letter or underscore.
         */
        bool isPossibleIdentifier(char32_t ch) {
            return ch == '_' || isAlphabetic(ch);
        }

        /* Scans something from the input that may be an identifier. It's possible that
         * we aren't looking at an identifier here and that we have something like
         *
         *    true
         *    Ax.
         *
         * Therefore, we'll use max-munch to pull out the longest identifier name we
         * could get, then do some context-dependent transforms on it.
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

            /* Special-case logic applies here: if the next token is a dot, then we
             * need to see whether we're looking at something of the form "Ax" or "Ex".
             */
            if (token.size() > 1 && (token[0] == 'A' || token[0] == 'E')) {
                /* Scan forward until we either hit EOF or encounter non-whitespace. */
                while (input.peek() != EOF) {
                    char32_t next = peekChar(input);

                    /* Skip whitespace. */
                    if (isSpace(next)) {
                        (void) readChar(input);
                    }

                    /* Found a dot? */
                    else if (next == '.') {
                        if (token[0] == 'A') {
                            result.push({ TokenType::FORALL, "A" });
                            token = token.substr(1);
                        } else if (token[0] == 'E') {
                            result.push({ TokenType::EXISTS, "E" });
                            token = token.substr(1);
                        }
                        break;
                    }
                    /* Otherwise, we don't care. */
                    else break;
                }
            }

            /* Decode what we have. That is, if it's a known token identifier, use that,
             * and otherwise make it an identifier.
             */
            auto itr = kTokens.find(token);
            if (itr != kTokens.end()) {
                result.push( { itr->second, token } );
            } else {
                result.push( { TokenType::IDENTIFIER, token } );
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
            /* If this is a "variable-like" token, scan it as such. */
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
