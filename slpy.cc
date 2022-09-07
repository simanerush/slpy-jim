#include <iostream>
#include <cstring>

#include "slpy-lex.hh"
#include "slpy-ast.hh"
#include "slpy-parse.hh"
#include "slpy-util.hh"

//
// slpy.cc - a SLPY ("Straight-Line PYthon") interpreter.
//
// Usage: ./slpy [--test] [--tokens] [--pprint] <SLPY source file name>
//
// This implements a Unix command for processing a SLPY program.  By
// default, it executes a SLPY program. There are command-line flags
// for other activities, namely:
//
//    --pprint - echo back the (parsed) source code instead of running it.
//
//    --tokens - also report the tokens processed by the lexer.
//
//    --test - give a simple ERROR message when an error occurs.
//
// The code is heavily reliant upon:
//
//   slpy-ast.{cc,hh} - defines the AST for SLPY programs
//   slpy-lex.{cc,hh} - converts the source into a token stream
//   slpy-parse.{cc,hh} - parse a SLPY token stream
//
// The interpreter works as a result of the `run` method of `Prgm` AST
// nodes, the `exec` method of `Stmt` AST nodes, and the `eval` method
// of `Expn` AST nodes. See `slpy-ast.cc` for details on each of these
// methods.
//


// * * * * * 
//
// Some utilities for extracting information from the command line.
//

bool check_flag(int argc, char** argv, std::string flag) {
    for (int i=1; i<argc; i++) {
        if (strcmp(flag.c_str(),argv[i]) == 0) return true;
    }
    return false;
}

char* extract_filename(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') return argv[i];
    }
    return nullptr;
}

// * * * * * 
//
// main - the SLPY interpreter
//
int main(int argc, char** argv) {
    
    //
    // Process the command-line, including any flags.
    //
    bool  debug_lex = check_flag(argc,argv,"--tokens");
    bool  pprint    = check_flag(argc,argv,"--pprint");
    bool  testing   = check_flag(argc,argv,"--test");
    char* filename  = extract_filename(argc,argv);
    
    if (filename) {
        //
        // Catch SLPY errors.
        //
        try {
            
            //
            // Process file to create a stream of tokens.
            //
            Tokenizer lexer { filename };
            TokenStream tks = lexer.lex();

            //
            // Report the tokens.
            //
            if (debug_lex) {
                std::cout << "----------------------------------" << std::endl;
                std::cout << "#";
                while (!tks.at_EOF()) {
                    std::cout << tks.current() << "#";
                    tks.advance();
                }
                std::cout << std::endl;
                std::cout << "----------------------------------" << std::endl;
            }

            //
            // Parse.
            //
            tks.reset();
            Prgm_ptr prgm = parse(tks);

            //
            // Make sure the whole source code was parsed.
            //
            if (!tks.at_EOF()) {
                //
                // Untested because of how parseBlck works.
                //
                throw SlpyError { tks.locate(), "Error: extra unparsed characters.\n" };
            }

            //
            // Either pretty print or run the parsed code.
            //
            if (pprint) {
                prgm->output(std::cout);
            } else {
                prgm->run();
            }
            
        } catch (SlpyError se) {
            
            //
            // If --test then just give "ERROR" message
            //
            if (testing) {
                std::cout << "ERROR" << std::endl;
            } else {
                std::cerr << se.what();
            }
        }
    } else {
        std::cerr << "usage: "
                  << argv[0]
                  << " [--tokens] [--pprint] [--test] file"
                  << std::endl;
    }
}
