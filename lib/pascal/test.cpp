#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include "tokenizer.h"


int main(int argc, char** argv) {

    if (argc < 2) return 0;

    std::ifstream ifile;
    ifile.open(argv[1]);

    std::stringstream buf;
    buf << ifile.rdbuf();
    std::string_view str = buf.str();
    
    pascal::Tokenizer tok(str);
    tok.tokenize();
    tok.print();

    return 0;
}
