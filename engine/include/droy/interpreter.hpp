#pragma once
// droy script x — parser + tree-walking interpreter, combined for simplicity.
#include "droy/lexer.hpp"
#include "droy/model.hpp"
#include <string>
#include <sstream>

namespace droy {

// Runs a droy script program and returns everything it printed via ~print
// (and a couple of diagnostic built-ins) as a single string, one line per
// call. Throws DroyError on malformed input.
std::string run(const std::string& source, Environment* envOut = nullptr);

} // namespace droy
