#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include "slpy-lex.hh"
#include "slpy-ast.hh"
#include "slpy-parse.hh"
#include "slpy-util.hh"

//
// slpy-parse.cc
//
// A recursive descent parser for the SLPY programming language.
//
// The code below forms a collection of mutually recursive procedures that
// work together to parse a stream of tokens from the SLPY (Straight-Line
// PYthon) programming language. The language has the following grammar:
//
//     <prgm> ::= <blck>
//     <blck> ::= <stmt> EOLN <stmt> EOLN ... <stmt> EOLN
//     <stmt> ::= <name> = <expn>
//              | pass
//              | print ( <expn> )
//
//     <expn> ::= <addn>
//     <addn> ::= <mult> <pm> <mult> <pm> ... <pm> <mult>
//     <pm>   ::= + | -
//     <mult> ::= <leaf>  <leaf>  ...  <leaf>
//     <md>   ::= * | //
//     <leaf> ::= <name> | <nmbr> 
//              | input ( <strg> ) | int( <expn> ) | ( <expn> )
//     <name> ::= x | count | _special | y0 | camelWalk | snake_slither | ...
//     <nmbr> ::= 0 | 1 | 2 | 3 | ...
//     <strg> ::= "hello" | "" | "say \"yo!\n\tyo.\"" | ...
//
// In the method of recursive descent, we invent a procedure for handling
// each of these grammar productions. So, in the code below, we've defined
// the procedures:
//
//     parsePrgm, parseBlck, parseExpn, parseAddn, parseMult, parseLeaf
//
// and then also we rely on other methods to handle variable names,
// integer literals, and string literals within the token stream.  At
// the top level, we process a <prgm>. This in turn processes a <blck>
// And that involves processing a series of EOLN-seperated <stmt>s.
//
// The prototypes for all these are defined in the associated .hh file.
// They all take a `TokenStream` object (by reference) and consume the
// tokens, from first to last, parsing the entire source program. Along
// the way, should errors be encountered (because of, say, syntax errors)
// a SlpyError exception might get thrown with info about the location of
// the problem in the source code, and the problem that was discovered.
//
// The code is heavily reliant on TokenStream methods that `advance` a
// "cursor" through the stream of tokens. At any moment, the parser makes
// decisions based on the `current()` token. In most situations a particular
// token is expected to appear next. In those cases, we use `eat*` methods
// to check whether the next token matches what we expect, and then advance
// the cursor to the next token. An error is thrown when a mismatch occurs.
//

//
// parseLeaf - parse tokens that form a "leaf" in the AST. This is typically
//             literals (integers, strings, etc.) but also function calls and
//              sub-expressions nested within parentheses.
//
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
//
Expn_ptr parseLeaf(TokenStream& tks) {
    if (tks.at("(")) {

        //
        // <leaf> ::= ( <expn> ) 
        //
        tks.eat("(");
        Expn_ptr expn = parseExpn(tks);
        tks.eat(")");

        return expn;

    } else if (tks.at("input")) {

        //
        // <leaf> ::= input ( <strg> )
        //
        Locn locn = tks.locate();
        tks.eat("input");
        tks.eat("(");
        //
        std::string strg = tks.eat_string();
        Inpt_ptr inpt = std::shared_ptr<Inpt> { new Inpt {strg,locn} };
        //
        tks.eat(")");

        return inpt;

    } else if (tks.at("int")) {

        //
        // <leaf> ::= int ( <expn> )
        //
        tks.eat("int");
        tks.eat("(");
        //
        Expn_ptr expn = parseExpn(tks);
        //
        tks.eat(")");

        return expn;

    } else if (tks.at_number()) {

        //
        // <leaf> ::= <nmbr> 
        //
        Locn locn = tks.locate();
        int valu = tks.eat_number();
        
        return std::shared_ptr<Nmbr> { new Nmbr {valu, locn} };

    } else if (tks.at_name()) {

        //
        // <leaf> ::= <name> 
        //
        Locn locn = tks.locate();
        std::string name = tks.eat_name();
    
        return std::shared_ptr<Lkup> { new Lkup {name, locn} };

    } else {

        Locn locn = tks.locate();
        Token tkn = tks.current();
        std::string msg = "";
        msg += "Syntax error: unexpected '" + tkn.token;
        msg += "' seen when parsing a leaf expression.\n";
        throw SlpyError { locn, msg };
        
    }
}

//
// parseMult - parse tokens that form a series of multiplications and
//             divisions of sub-expressions.
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
//
Expn_ptr parseMult(TokenStream& tks) {
    //
    // <mult> ::= <leaf> * <leaf> * ... * <leaf>
    //
    Expn_ptr expn1 = parseLeaf(tks);
    while (tks.at("*") || tks.at("//")) {
        Locn locn = tks.locate();
        if (tks.at("*")) {
            tks.eat("*");       
            Expn_ptr expn2 = parseLeaf(tks);
            expn1 = std::shared_ptr<Tmes> { new Tmes {expn1, expn2, locn} };
        } else {
            tks.eat("//");       
            Expn_ptr expn2 = parseLeaf(tks);
            expn1 = std::shared_ptr<IDiv> { new IDiv {expn1, expn2, locn} };
        }
    }

    return expn1;
}

//
// parseAddn - parse tokens that form a series of additions and subtractions
//             of sub-expressions.
//
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
//
Expn_ptr parseAddn(TokenStream& tks) {
    //
    // <addn> ::= <mult> +/- <mult> +/- ... +/- <mult>
    //
    Expn_ptr expn1 = parseMult(tks);
    while (tks.at("+") || tks.at("-")) {
        Locn locn = tks.locate();
        if (tks.at("+")) {
            tks.eat("+");
            Expn_ptr expn2 = parseMult(tks);
            expn1 = std::shared_ptr<Plus> { new Plus {expn1, expn2, locn} };
        } else {
            tks.eat("-");
            Expn_ptr expn2 = parseMult(tks);
            expn1 = std::shared_ptr<Mnus> { new Mnus {expn1, expn2, locn} };
        }
    }

    return expn1;
}

//
// parseExpn - parse tokens that form an evaluatable sub-expression of SLPY.
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
//
Expn_ptr parseExpn(TokenStream& tks) {
    //
    // <expn> ::= <addn>
    //
    return parseAddn(tks);
}

//
// parseStmt - parse tokens that form an executable SLPY statement line.
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
//
Stmt_ptr parseStmt(TokenStream& tks) {
    if (tks.at("print")) {

        //
        // <stmt> ::= print ( <expn> )
        //
        Locn locn = tks.locate();
        tks.eat("print");
        tks.eat("(");
        Expn_ptr expn = parseExpn(tks);
        tks.eat(")");

        return std::shared_ptr<Prnt> { new Prnt { expn, locn } };

    } else if (tks.at("pass")) {

        //
        // <stmt> ::= pass
        //
        Locn locn = tks.locate();
        tks.eat("pass");

        return std::shared_ptr<Pass> { new Pass { locn } };

    } else {
        
        //
        // <stmt> ::= <name> = <expn>
        //
        std::string name = tks.eat_name();
        Locn locn = tks.locate();
        tks.eat("=");
        Expn_ptr expn = parseExpn(tks);

        return std::shared_ptr<Asgn> { new Asgn { name, expn, locn } };
    }
}

//
// parseBlck - parse tokens that form an block of executable SLPY statements,
//             with each ending with a new line.
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
//
Blck_ptr parseBlck(TokenStream& tks) {

    //
    // <blck> ::= <stmt> EOLN <stmt> EOLN ... <stmt> EOLN
    // 
    Stmt_vec stms { };
    Locn locn = tks.locate();
    do {
        Stmt_ptr stmt = parseStmt(tks);
        tks.eat_EOLN();
        stms.push_back(stmt);
    } while (!tks.at_EOF());

    return std::shared_ptr<Blck> { new Blck { stms, locn } };
}

//
// parsePrgm - parse tokens that form a runnable SLPY program.
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
//
Prgm_ptr parsePrgm(TokenStream& tks) {

    //
    // <prgm> ::= <blck>
    //
    Locn locn = tks.locate();
    Prgm* prgm = new Prgm {parseBlck(tks), locn};

    return std::shared_ptr<Prgm> { prgm };
}

//
// parse - top-level call to the SLPY parser.
//
// Modifies the TokenStream `tks` by consuming token and advancing its cursor.
// This procedure should advance it all the way to the end of the stream,
// consuming all the tokens.
//
Prgm_ptr parse(TokenStream& tks) {
    return parsePrgm(tks);
}
