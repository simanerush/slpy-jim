#include <iostream>
#include <cstring>

#include "slpy-lex.hh"
#include "slpy-ast.hh"
#include "slpy-parse.hh"

bool check_debug_lex(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (strcmp("--tokens",argv[i]) == 0) return true;
    }
    return false;
}

bool check_pprint(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (strcmp("--pprint",argv[i]) == 0) return true;
    }
    return false;
}

char* check_filename(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') return argv[i];
    }
    return nullptr;
}
int main(int argc, char** argv) {
    bool debug_lex = check_debug_lex(argc,argv);
    bool pprint    = check_pprint(argc,argv);
    char* filename = check_filename(argc,argv);
    if (filename) {
        Tokenizer lexer { filename };
        TokenStream tks = lexer.lex();
        if (debug_lex) {
            // Output the tokens.
            std::cout << "----------------------------------" << std::endl;
            std::cout << "#";
            while (!tks.at_EOF()) {
                std::cout << tks.current() << "#";
                tks.advance();
            }
            std::cout << std::endl;
            std::cout << "----------------------------------" << std::endl;
        } 
        tks.reset();
        Prgm_ptr prgm = parse(tks);
        assert(tks.at_EOF());
        if (pprint) {
            prgm->output(std::cout);
        } else {
            prgm->run();
        }
    } else {
        std::cout << "usage: " << argv[0] << " [-tokens] [-pprint] file" << std::endl;
    }
}
