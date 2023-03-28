#pragma once

#include <string>
#include <queue>
#include <istream>

namespace Turing {
    /* Enumerated type representing all the tokens we might expect to see. */
    enum class TokenType {
        MOVE      = 'M',
        LEFT      = 'L',
        RIGHT     = 'R',
        GOTO      = 'G',
        PRINT     = 'P',
        BLANK     = '_',
        CHAR      = 'a',
        COLON     = ':',
        LABEL     = 'L',
        TRUE      = 'Y',
        FALSE     = 'N',
        RETURN    = 'r',
        IF        = '?',
        NOT       = '!',
        SCAN_EOF  = '$'
    };

    /* Type representing a single token. */
    struct Token {
        TokenType   type;
        std::string data;
    };

    std::string to_string(const Token& t);

    /* Scans the input stream, producing a queue of tokens. If the scan fails, an
     * exception is generateed.
     *
     * The last token produced will always be a SCAN_EOF token.
     */
    std::queue<Token> scan(std::istream& input);
    std::queue<Token> scan(const std::string& sourceText);
}
