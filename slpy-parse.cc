#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include "slpy-lex.hh"
#include "slpy-ast.hh"
#include "slpy-parse.hh"

void bail_parse() {
    exit(-1);
}
    
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
        tks.eat("input");
        tks.eat("(");
        //
        if (!tks.at_string()) {
            Token tkn = tks.current();
            std::cerr << "Syntax error expected string literal, saw\"" << tkn.token;
            std::cerr << "\" at " << tkn.row << ":" << tkn.column;
            std::cerr << ", while parsing an input expression." << std::endl;
            bail_parse();
        }
        std::string strg = tks.eat_string();
        Inpt_ptr inpt = std::shared_ptr<Inpt> { new Inpt {strg} };
        //
        tks.eat(")");

        return inpt;

    } else if (tks.at_number()) {

        //
        // <leaf> ::= <nmbr> 
        //
        int valu = tks.eat_number();
        
        return std::shared_ptr<Nmbr> { new Nmbr {valu} };

    } else if (tks.at_name()) {

        //
        // <leaf> ::= <name> 
        //
        std::string name = tks.eat_name();
    
        return std::shared_ptr<Lkup> { new Lkup {name} };

    } else {

        Token tkn = tks.current();
        std::cerr << "Syntax error unexpected \"" << tkn.token;
        std::cerr << "\" at " << tkn.row << ":" << tkn.column;
        std::cerr << " while parsing a leaf expression." << std::endl;
        bail_parse();

        return nullptr;
    }
}

Expn_ptr parseMult(TokenStream& tks) {
    //
    // <mult> ::= <leaf> * <leaf> * ... * <leaf>
    //
    Expn_ptr expn1 = parseLeaf(tks);
    while (tks.at("*") || tks.at("//")) {
        if (tks.at("*")) {
            tks.eat("*");       
            Expn_ptr expn2 = parseLeaf(tks);
            expn1 = std::shared_ptr<Tmes> { new Tmes {expn1, expn2} };
        } else {
            tks.eat("//");       
            Expn_ptr expn2 = parseLeaf(tks);
            expn1 = std::shared_ptr<IDiv> { new IDiv {expn1, expn2} };
        }
    }

    return expn1;
}

Expn_ptr parseAddn(TokenStream& tks) {
    //
    // <addn> ::= <mult> +/- <mult> +/- ... +/- <mult>
    //
    Expn_ptr expn1 = parseMult(tks);
    while (tks.at("+") || tks.at("-")) {
        if (tks.at("+")) {
            tks.eat("+");
            Expn_ptr expn2 = parseMult(tks);
            expn1 = std::shared_ptr<Plus> { new Plus {expn1, expn2} };
        } else {
            tks.eat("-");
            Expn_ptr expn2 = parseMult(tks);
            expn1 = std::shared_ptr<Mnus> { new Mnus {expn1, expn2} };
        }
    }

    return expn1;
}

Expn_ptr parseExpn(TokenStream& tks) {
    //
    // <expn> ::= <addn>
    //
    return parseAddn(tks);
}

Stmt_ptr parseStmt(TokenStream& tks) {
    if (tks.at("print")) {

        //
        // <stmt> ::= print ( <expn> )
        //
        tks.eat("print");
        tks.eat("(");
        Expn_ptr expn = parseExpn(tks);
        tks.eat(")");

        return std::shared_ptr<Prnt> { new Prnt { expn } };

    } else if (tks.at("pass")) {

        //
        // <stmt> ::= pass
        //
        tks.eat("pass");

        return std::shared_ptr<Pass> { new Pass { } };

    } else {
        
        //
        // <stmt> ::= <name> = <expn>
        //
        std::string name = tks.eat_name();
        tks.eat("=");
        Expn_ptr expn = parseExpn(tks);

        return std::shared_ptr<Asgn> { new Asgn { name, expn } };
    }
}

Blck_ptr parseBlck(TokenStream& tks) {

    //
    // <blck> ::= <stmt> EOLN <stmt> EOLN ... <stmt> EOLN
    // 
    Stmt_vec stms { };
    do {
        Stmt_ptr stmt = parseStmt(tks);
        tks.eat_EOLN();
        stms.push_back(stmt);
    } while (!tks.at_EOF());

    return std::shared_ptr<Blck> { new Blck { stms } };
}

Prgm_ptr parsePrgm(TokenStream& tks) {

    //
    // <prgm> ::= <blck>
    //
    Prgm* prgm = new Prgm {parseBlck(tks)};

    return std::shared_ptr<Prgm> { prgm };
}

Prgm_ptr parse(TokenStream& tks) {
    return parsePrgm(tks);
}
