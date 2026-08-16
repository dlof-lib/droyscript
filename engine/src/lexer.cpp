#include "droy/lexer.hpp"
#include "droy/model.hpp"
#include <cctype>

namespace droy {

char Lexer::peek(int off) const {
    size_t p = pos_ + off;
    if (p >= src_.size()) return '\0';
    return src_[p];
}

char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') line_++;
    return c;
}

bool Lexer::isAtEnd() const { return pos_ >= src_.size(); }

void Lexer::skipWhitespaceAndComments() {
    for (;;) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { advance(); continue; }
        if (c == '/' && peek(1) == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
            continue;
        }
        if (c == '#') {
            while (!isAtEnd() && peek() != '\n') advance();
            continue;
        }
        break;
    }
}

Token Lexer::lexIdentOrKeyword() {
    Token t; t.line = line_; t.type = TokType::Ident;
    std::string s;
    while (!isAtEnd() && (isalnum((unsigned char)peek()) || peek() == '_')) {
        s += advance();
    }
    t.text = s;
    return t;
}

Token Lexer::lexNumber() {
    Token t; t.line = line_; t.type = TokType::Number;
    std::string s;
    while (!isAtEnd() && (isdigit((unsigned char)peek()) || peek() == '.')) s += advance();
    t.text = s;
    t.num = atof(s.c_str());
    return t;
}

Token Lexer::lexString() {
    Token t; t.line = line_; t.type = TokType::String;
    advance(); // opening quote
    std::string s;
    while (!isAtEnd() && peek() != '"') {
        char c = advance();
        if (c == '\\' && !isAtEnd()) {
            char n = advance();
            if (n == 'n') s += '\n';
            else if (n == 't') s += '\t';
            else s += n;
        } else {
            s += c;
        }
    }
    if (!isAtEnd()) advance(); // closing quote
    t.text = s;
    return t;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> out;
    for (;;) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;
        char c = peek();
        int ln = line_;

        if (c == '@') { advance(); out.push_back({TokType::At, "@", 0, ln}); continue; }
        if (c == '$') { advance(); out.push_back({TokType::Dollar, "$", 0, ln}); continue; }
        if (c == '~') { advance(); out.push_back({TokType::Tilde, "~", 0, ln}); continue; }
        if (c == '(') { advance(); out.push_back({TokType::LParen, "(", 0, ln}); continue; }
        if (c == ')') { advance(); out.push_back({TokType::RParen, ")", 0, ln}); continue; }
        if (c == ',') { advance(); out.push_back({TokType::Comma, ",", 0, ln}); continue; }
        if (c == '"') { Token t = lexString(); t.line = ln; out.push_back(t); continue; }

        if (c == '=' && peek(1) == '>') { advance(); advance(); out.push_back({TokType::Arrow, "=>", 0, ln}); continue; }
        if (c == '<' && peek(1) == '=') { advance(); advance(); out.push_back({TokType::BackArrow, "<=", 0, ln}); continue; }
        if (c == '=') { advance(); out.push_back({TokType::Equals, "=", 0, ln}); continue; }

        if (c == '.') {
            // a lone '.' used as a method-call sigil (e.g. Users.equals)
            advance();
            out.push_back({TokType::Dot, ".", 0, ln});
            continue;
        }

        if (isdigit((unsigned char)c) || (c == '-' && isdigit((unsigned char)peek(1)))) {
            bool neg = c == '-';
            if (neg) advance();
            Token t = lexNumber(); t.line = ln;
            if (neg) t.num = -t.num;
            out.push_back(t);
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            Token t = lexIdentOrKeyword(); t.line = ln;
            out.push_back(t);
            continue;
        }

        throw DroyError("droy: unexpected character '" + std::string(1, c) + "' at line " + std::to_string(ln));
    }
    out.push_back({TokType::End, "", 0, line_});
    return out;
}

} // namespace droy
