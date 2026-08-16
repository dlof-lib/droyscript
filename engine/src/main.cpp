// droy script x — command line runner
// Usage: droyc path/to/script.droy
#include "droy/interpreter.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: droyc <file.droy>\n";
        return 1;
    }
    std::ifstream f(argv[1]);
    if (!f) {
        std::cerr << "droyc: cannot open " << argv[1] << "\n";
        return 1;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    try {
        std::string output = droy::run(ss.str());
        std::cout << output;
    } catch (const droy::DroyError& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
