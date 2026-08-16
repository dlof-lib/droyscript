#include "droy/interpreter.hpp"
#include <vector>
#include <cmath>

namespace droy {

namespace {

class Parser {
public:
    Parser(std::vector<Token> toks, Environment& env, std::ostringstream& out)
        : t_(std::move(toks)), env_(env), out_(out) {}

    void run() {
        expectSeeDroyPrefix();
        expect(TokType::Equals);
        expectIdentText("start");

        while (!atEndMarker()) {
            statement();
        }
        expectSeeDroyPrefix();
        expect(TokType::Equals);
        expectIdentText("end");
    }

private:
    std::vector<Token> t_;
    size_t p_ = 0;
    Environment& env_;
    std::ostringstream& out_;

    const Token& peek(int off = 0) const {
        size_t i = p_ + off;
        if (i >= t_.size()) return t_.back();
        return t_[i];
    }
    const Token& advance() { const Token& tok = peek(); if (p_ < t_.size() - 1) p_++; return tok; }
    bool check(TokType ty) const { return peek().type == ty; }
    bool checkIdent(const std::string& text) const { return peek().type == TokType::Ident && peek().text == text; }

    const Token& expect(TokType ty) {
        if (!check(ty)) fail("expected token type " + std::to_string((int)ty) + " but found '" + peek().text + "'");
        return advance();
    }
    void expectAt() { expect(TokType::At); }
    void expectIdentText(const std::string& text) {
        if (!checkIdent(text)) fail("expected '" + text + "' but found '" + peek().text + "'");
        advance();
    }
    [[noreturn]] void fail(const std::string& msg) {
        throw DroyError("droy: parse error at line " + std::to_string(peek().line) + ": " + msg);
    }

    void expectSeeDroyPrefix() {
        expectAt();
        expectIdentText("see");
        expect(TokType::Dot);
        expectIdentText("droy");
    }

    bool atEndMarker() const {
        return peek().type == TokType::At
            && peek(1).type == TokType::Ident && peek(1).text == "see"
            && peek(2).type == TokType::Dot
            && peek(3).type == TokType::Ident && peek(3).text == "droy";
    }

    Value parseValue() {
        if (check(TokType::String)) { return Value::str(advance().text); }
        if (check(TokType::Number)) { return Value::num(advance().num); }
        if (check(TokType::Ident)) { return Value::ident(advance().text); }
        fail("expected a value (string, number, or identifier)");
    }

    // key=value
    std::pair<std::string, Value> parseField() {
        std::string key = expect(TokType::Ident).text;
        expect(TokType::Equals);
        Value v = parseValue();
        return {key, v};
    }

    Row parseRow() {
        expect(TokType::Dollar);
        expectIdentText("row");
        Row r;
        while (check(TokType::Ident) && peek(1).type == TokType::Equals) {
            auto f = parseField();
            r.set(f.first, f.second);
        }
        return r;
    }

    void statement() {
        if (check(TokType::At) && checkIdentAhead("group")) { parseGroup(); return; }
        if (check(TokType::At) && checkIdentAhead("collection")) { parseCollection(); return; }
        if (check(TokType::At) && checkIdentAhead("bridge")) { parseBridge(); return; }
        if (check(TokType::At) && checkIdentAhead("map")) { parseMap(); return; }
        if (check(TokType::Tilde)) { parseBuiltinCall(); return; }
        fail("unexpected token '" + peek().text + "'");
    }

    bool checkIdentAhead(const std::string& text) const {
        return peek(1).type == TokType::Ident && peek(1).text == text;
    }

    void expectBlockEnd() {
        expectAt();
        expectIdentText("end");
    }

    void parseGroup() {
        expectAt(); expectIdentText("group");
        std::string name = expect(TokType::Ident).text;
        Group g; g.name = name;
        while (check(TokType::Dollar)) g.rows.push_back(parseRow());
        expectBlockEnd();
        env_.groups[name] = std::move(g);
    }

    void parseCollection() {
        expectAt(); expectIdentText("collection");
        std::string name = expect(TokType::Ident).text;
        Collection c; c.name = name;

        if (check(TokType::BackArrow)) {
            advance();
            std::string srcName = expect(TokType::Ident).text;
            expect(TokType::Dot);
            expectIdentText("equals");
            expect(TokType::LParen);
            auto field = parseField();
            expect(TokType::RParen);

            c.derivedFrom = srcName;
            auto it = env_.groups.find(srcName);
            if (it == env_.groups.end()) fail("unknown group '" + srcName + "' in collection derive");
            for (auto& row : it->second.rows) {
                const Value* v = row.get(field.first);
                if (v && v->equals(field.second)) c.rows.push_back(row);
            }
            if (check(TokType::At) && checkIdentAhead("end")) expectBlockEnd();
        } else {
            while (check(TokType::Dollar)) c.rows.push_back(parseRow());
            expectBlockEnd();
        }
        env_.collections[name] = std::move(c);
    }

    void parseBridge() {
        expectAt(); expectIdentText("bridge");
        std::string name = expect(TokType::Ident).text;
        Bridge b; b.name = name;
        while (check(TokType::Ident) && peek().text == "edge") {
            advance();
            auto from = parseField();
            expect(TokType::Arrow);
            auto to = parseField();
            b.edges.push_back({from.second, to.second});
        }
        expectBlockEnd();
        env_.bridges[name] = std::move(b);
    }

    void parseMap() {
        expectAt(); expectIdentText("map");
        std::string name = expect(TokType::Ident).text;
        DroyMap m; m.name = name;
        while (check(TokType::String)) {
            Value key = Value::str(advance().text);
            expect(TokType::Arrow);
            Value val = parseValue();
            m.entries.push_back({key, val});
        }
        expectBlockEnd();
        env_.maps[name] = std::move(m);
    }

    std::vector<Value> parseArgs() {
        expect(TokType::LParen);
        std::vector<Value> args;
        if (!check(TokType::RParen)) {
            args.push_back(parseValue());
            while (check(TokType::Comma)) { advance(); args.push_back(parseValue()); }
        }
        expect(TokType::RParen);
        return args;
    }

    void parseBuiltinCall() {
        expect(TokType::Tilde);
        std::string fn = expect(TokType::Ident).text;
        std::vector<Value> args = parseArgs();
        callBuiltin(fn, args);
    }

    void callBuiltin(const std::string& fn, const std::vector<Value>& args) {
        if (fn == "print") {
            if (args.empty()) fail("~print requires one argument");
            out_ << formatEntity(args[0].toString()) << "\n";
            return;
        }
        if (fn == "len") {
            if (args.empty()) fail("~len requires one argument");
            out_ << args[0].toString() << ".len => " << lengthOf(args[0].toString()) << "\n";
            return;
        }
        if (fn == "sum") {
            if (args.size() < 2) fail("~sum requires (group, field)");
            out_ << "sum(" << args[0].toString() << "." << args[1].toString() << ") => "
                 << sumOf(args[0].toString(), args[1].toString()) << "\n";
            return;
        }
        fail("unknown built-in '~" + fn + "'");
    }

    static std::string valStr(const Value& v) {
        if (v.kind == Value::Kind::String) return "\"" + v.s + "\"";
        return v.toString();
    }

    std::string formatEntity(const std::string& name) {
        std::ostringstream o;
        if (env_.groups.count(name)) {
            auto& g = env_.groups[name];
            o << "group " << name << " (" << g.rows.size() << " rows) {\n";
            for (auto& r : g.rows) {
                o << "  row";
                for (auto& f : r.fields) o << " " << f.first << "=" << valStr(f.second);
                o << "\n";
            }
            o << "}";
            return o.str();
        }
        if (env_.collections.count(name)) {
            auto& c = env_.collections[name];
            o << "collection " << name;
            if (!c.derivedFrom.empty()) o << " <= " << c.derivedFrom << ".equals(...)";
            o << " (" << c.rows.size() << " rows) {\n";
            for (auto& r : c.rows) {
                o << "  row";
                for (auto& f : r.fields) o << " " << f.first << "=" << valStr(f.second);
                o << "\n";
            }
            o << "}";
            return o.str();
        }
        if (env_.bridges.count(name)) {
            auto& b = env_.bridges[name];
            o << "bridge " << name << " (" << b.edges.size() << " edges) {\n";
            for (auto& e : b.edges) o << "  edge id=" << e.from.toString() << " => id=" << e.to.toString() << "\n";
            o << "}";
            return o.str();
        }
        if (env_.maps.count(name)) {
            auto& m = env_.maps[name];
            o << "map " << name << " (" << m.entries.size() << " entries) {\n";
            for (auto& e : m.entries) o << "  " << valStr(e.key) << " => " << valStr(e.value) << "\n";
            o << "}";
            return o.str();
        }
        fail("unknown entity '" + name + "'");
    }

    size_t lengthOf(const std::string& name) {
        if (env_.groups.count(name)) return env_.groups[name].rows.size();
        if (env_.collections.count(name)) return env_.collections[name].rows.size();
        if (env_.bridges.count(name)) return env_.bridges[name].edges.size();
        if (env_.maps.count(name)) return env_.maps[name].entries.size();
        fail("unknown entity '" + name + "'");
    }

    double sumOf(const std::string& groupName, const std::string& field) {
        auto it = env_.groups.find(groupName);
        if (it == env_.groups.end()) fail("unknown group '" + groupName + "'");
        double total = 0;
        for (auto& r : it->second.rows) {
            const Value* v = r.get(field);
            if (v) total += (v->kind == Value::Kind::Number) ? v->n : atof(v->s.c_str());
        }
        return total;
    }
};

} // namespace

std::string run(const std::string& source, Environment* envOut) {
    Lexer lex(source);
    auto tokens = lex.tokenize();
    Environment env;
    std::ostringstream out;
    Parser parser(std::move(tokens), env, out);
    parser.run();
    if (envOut) *envOut = env;
    return out.str();
}

} // namespace droy
