#pragma once
// droy script x — data model
// Groups/Rows/Collections/Bridges/Maps — the concepts of the language.

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace droy {

// A scalar value: string, number, or bare identifier (treated as string).
struct Value {
    enum class Kind { String, Number, Ident } kind = Kind::String;
    std::string s;
    double n = 0.0;

    static Value str(std::string v) { Value x; x.kind = Kind::String; x.s = std::move(v); return x; }
    static Value num(double v) { Value x; x.kind = Kind::Number; x.n = v; return x; }
    static Value ident(std::string v) { Value x; x.kind = Kind::Ident; x.s = std::move(v); return x; }

    bool isEmptyString() const { return kind == Kind::String && s.empty(); }

    std::string toString() const {
        switch (kind) {
            case Kind::Number: {
                std::ostringstream o;
                if (n == (long long)n) o << (long long)n; else o << n;
                return o.str();
            }
            case Kind::Ident: return s;
            case Kind::String: default: return s;
        }
    }

    bool equals(const Value& o) const {
        if (kind == Kind::Number || o.kind == Kind::Number) {
            double a = kind == Kind::Number ? n : atof(s.c_str());
            double b = o.kind == Kind::Number ? o.n : atof(o.s.c_str());
            return a == b;
        }
        return s == o.s;
    }
};

// A row is an ordered set of named fields.
struct Row {
    std::vector<std::pair<std::string, Value>> fields;

    bool has(const std::string& key) const {
        for (auto& f : fields) if (f.first == key) return true;
        return false;
    }
    const Value* get(const std::string& key) const {
        for (auto& f : fields) if (f.first == key) return &f.second;
        return nullptr;
    }
    void set(const std::string& key, Value v) {
        for (auto& f : fields) if (f.first == key) { f.second = std::move(v); return; }
        fields.emplace_back(key, std::move(v));
    }
};

struct Group {
    std::string name;
    std::vector<Row> rows;
};

struct Collection {
    std::string name;
    std::vector<Row> rows;
    std::string derivedFrom; // group name, if derived via .equals(...)
};

struct Edge {
    Value from;
    Value to;
};

struct Bridge {
    std::string name;
    std::vector<Edge> edges;
};

struct MapEntry {
    Value key;
    Value value;
};

struct DroyMap {
    std::string name;
    std::vector<MapEntry> entries;
};

// Runtime environment: every named entity droy script knows about.
struct Environment {
    std::map<std::string, Group> groups;
    std::map<std::string, Collection> collections;
    std::map<std::string, Bridge> bridges;
    std::map<std::string, DroyMap> maps;

    bool exists(const std::string& name) const {
        return groups.count(name) || collections.count(name) || bridges.count(name) || maps.count(name);
    }
};

struct DroyError : std::runtime_error {
    explicit DroyError(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace droy
