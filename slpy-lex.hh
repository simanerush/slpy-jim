#ifndef __slpy_lex_
#define __slpy_lex_

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <optional>
#include "slpy-util.hh"

//
// Token
//
// Represents a chunk of consecutive characters in the source code.
// It contains the following information:
//  * `token`: the string that represents the chunk of characters
//  * `row`: the line where the chunk begins
//  * `column`: the column within the line where the chunk begins
//
class Token {
public:
    std::string token;
    int row;
    int column;
    Token(std::string tkn, int row, int col);
    friend std::ostream& operator<<(std::ostream& outs, Token tkn);
};

//
// TokenStream
//
// Represents a sequence of Token objects. It can be built by a tokenizer
// with a series of calls to `append`.
//
// After beiung `reset`, it can be parsed (and consumed) with a combination of:
//    * advance, eat (the latter looking for a match)
//    * eat_name, eat_number, eat_string, eat_EOF, eat_EOLN
// The current token (next to be consumed) can be queried with
//    * current, at (to look for a match)
//    * at_name, at_number, at_string, at_EOF, at_EOLN
//
class TokenStream {    
private:
    std::string src_name;
    std::vector<Token> tokens;
    int where;

public:
    TokenStream(std::string filename);
    std::string source_name(void);
    Locn locate(void);
    //
    void append(Token tkn);
    //
    void reset(void);
    Token current(void);
    void advance(void);
    //
    bool at(std::string match);
    bool at_number(void);
    bool at_string(void);
    bool at_name(void);
    bool at_EOLN(void);
    bool at_EOF(void);
    //
    void eat(std::string match);
    int eat_number(void);
    std::string eat_string(void);
    std::string eat_name(void);
    void eat_EOLN();
    void eat_EOF();
};

//
// Tokenizer
//
// Constructs a TokenStream from a SLPy source file using method `lex`.
//
class Tokenizer {
private:
    int state;
    //
    char* src_name;         
    std::ifstream src_stream;
    //
    char curr_char;
    int row;
    int column;
    //
    std::stringstream curr_token;
    int start_row;
    int start_column;
    TokenStream tokenstream;
    
    //
    void start_fresh_token();
    void issue_token(void);
    //
    void advance_char(void);
    void consume_char(void);
    void consume_char_as(std::string s);
    void consume_then_issue(void);
    void consume_then_start_fresh(void);
    void bail_with_error(std::string message);
    void bail_with_char(std::string message);
    
public:
    Tokenizer(char* src_file_name);
    TokenStream lex();
};

#endif
