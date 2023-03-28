#pragma once

#include <string>
#include <queue>
#include <istream>

namespace Logic {
    /* Enumerated type representing all the tokens we might expect to see. */
    enum class TokenType {
      TRUE = 'T',
      FALSE = 'F',
      AND = '&',
      OR = '|',
      IMPLIES = '>',
      IFF = '#',
      NOT = '~',
      LPAREN = '(',
      RPAREN = ')',
      FORALL = 'A',
      EXISTS = 'E',
      EQUALS = '=',
      NOTEQUALS = 'Z',
      IDENTIFIER = 'I',
      DOT = '.',
      COMMA = ',',
      SCAN_EOF = '$'
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
