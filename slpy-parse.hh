#ifndef _slpy_parse_
#define _slpy_parse_

#include <vector>
#include "slpy-lex.hh"
#include "slpy-ast.hh"

Expn_ptr parseLeaf(TokenStream& tks);
Expn_ptr parseMult(TokenStream& tks);
Expn_ptr parseAddn(TokenStream& tks);
Expn_ptr parseExpn(TokenStream& tks);
Stmt_ptr parseStmt(TokenStream& tks);
Blck_ptr parseBlck(TokenStream& tks);
Prgm_ptr parsePrgm(TokenStream& tks);
Prgm_ptr parse(TokenStream& tks);

#endif
