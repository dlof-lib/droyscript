#pragma once
// droy script x — lexer
#include <string>
#include <vector>

namespace droy {

enum class TokType {
    At,           // @
    Dollar,       // $
    Dot,          // .
    Tilde,        // ~
    Arrow,        // =>
    BackArrow,    // <=
    Equals,       // =
    LParen, RParen,
    Comma,
    Ident,
    Number,
    String,
    End
};

struct Token {
    TokType type;
    std::string text;
    double num = 0.0;
    int line = 0;
};

class Lexer {
public:
    explicit Lexer(std::string src) : src_(std::move(src)) {}
    std::vector<Token> tokenize();

private:
    std::string src_;
    size_t pos_ = 0;
    int line_ = 1;

    char peek(int off = 0) const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespaceAndComments();
    Token lexIdentOrKeyword();
    Token lexNumber();
    Token lexString();
};

} // namespace droy
