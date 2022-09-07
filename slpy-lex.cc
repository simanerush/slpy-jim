#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

#include "slpy-util.hh"
#include "slpy-lex.hh"

//
// slpy-lex.cc
//
// An implementation of a lexical analysis tool for the SLPY language
// and also the classes that support its work. This includes
//
// class Token: strings that appear in the source, along with their extent
// class TokenStream: a sequence of tokens resulting from the analysis
// class Tokenizer: the wrapper class for the analysis. 
//
// A Tokenizer instance reads the text of SLPY source code, consumes it
// character by character, breaking it up into chunks (and ignoring things
// like whitespace), to produce a TokenStream instance.
//
// The analysis is defined under the method TokenStream::lex.
//
// Typical use of this module is:
//
//     Tokenizer   tz { "my_program.slpy" };
//     TokenStream ts = tz.lex();
//

// * * * * *
//
// Some utility functions' prototypes. Defined at the bottom.
//
// * * * * * 
bool is_name(std::string s);   // Is the token string a name?
bool is_number(std::string s); // Is the token string an integer literal?
bool is_string(std::string s); // Is the token string an string literal?

// * * * * *
//
// class Token
//
// Defines a chunk of characters at a location in a SLPY source file.
//

//
// Token { s, r, c }
//
// Constructs a Token representing a string s and its start position in the
// source file at (r,c).
//
Token::Token(std::string tkn, int row, int col)
    : token  {tkn},
      row    {row},
      column {col}
{ }

//
// outs << t;
//
// Pretty prints a description of a Token t.
//
std::ostream& operator<<(std::ostream& outs, Token tkn) {
    if (tkn.token == "\n") {
        outs << "[NEWLINE]";
    } else if (tkn.token == "\377") {
        outs << "[EOF]";
    } else if (tkn.token[0] == '\t' || tkn.token[0] == ' ') {
        int tabbing = 0;
        for (char c: tkn.token) {
            if (c == ' ') {
                tabbing += 1;
            } else if (c == '\t'){
                tabbing += 8 - (tabbing % 8);
            } else {
                std::cerr << c << std::endl;
                std::cerr << "Error within tab token." << std::endl;
            }
        }
        outs << "[INDENT-" << tabbing << "]";
    } else {
        outs << tkn.token;
    }
    outs << ":" << tkn.row << ":" << tkn.column;
    return outs;
}

// * * * * *
// class TokenStream
//
// Provides a sequence of SLPY tokens to be parsed.
//

//
// ts = TokenStream { } 
//
// constructs an empty sequence.
//
TokenStream::TokenStream(std::string filename) :
    src_name {filename},
    tokens {},
    where {0}
{ }

//
// ts.source_name();
//
// Gets the name of the SLPY source file for this token stream.
//
std::string TokenStream::source_name(void) {
    return src_name;
}

//
// ts.locate();
//
// Gets info about the current token's place in the code.
//
Locn TokenStream::locate(void) {
    return Locn { src_name, current().row, current().column };
}

// ts.append(t) 
//
// During construction of the sequence, appends token t to the end.
//
void TokenStream::append(Token tkn) {
    tokens.push_back(tkn);
}

// ts.reset();
//
// Puts the "cursor" at the front of the sequence for parsing.
//
void TokenStream::reset(void) {
    where = 0;
}

// t = ts.current();
//
// Gives the token at the "cursor", next to be processed by the parser.
//
Token TokenStream::current(void) {
    return tokens[where];
}

// ts.advance();
//
// Advances the cursor to the next token.
//
void TokenStream::advance(void) {
    assert(where < tokens.size());
    where++;
}

// b = ts.at(s)
//
// Returns true if the next token's string matches s.
//
bool TokenStream::at(std::string match) {
    return (current().token == match);
}

// b = ts.at_name()
//
// Returns true if the next token's string is an identifying name.
//
bool TokenStream::at_name(void) {
    return is_name(current().token);
}

// b = ts.at_number()
//
// Returns true if the next token's string is an decimal encoding of an int.
//
bool TokenStream::at_number(void) {
    return is_number(current().token);
}

// b = ts.at_string()
//
// Returns true if the next token is for a string literal.
//
bool TokenStream::at_string(void) {
    return is_string(current().token);
}

// b = ts.at_EOLN()
//
// Returns true if the next token is an end-of-line.
//
bool TokenStream::at_EOLN(void) {
    return current().token == "\n";
}

// b = ts.at_EOF()
//
// Returns true if the next token is the sequence terminator.
//
bool TokenStream::at_EOF(void) {
    return (where == tokens.size());
}

//
// ts.eat(s);
//
// Consumes the next token if it matches s.
// Raises an error if it doesn't.
//
void TokenStream::eat(std::string match) {
    if (match == current().token) {
        advance();
    } else {
        std::string msg = "Syntax error: expected '";
        msg += match;
        msg += "' but saw '";
        msg += current().token;
        msg += "' instead.";
        throw SlpyError { locate(), msg };
    }
}

//
// ts.eat_EOLN();
//
// Consumes the next token if it is an end of line.
// Raises an error if it doesn't.
//
void TokenStream::eat_EOLN(void) {
    if (at_EOLN()) {
        advance();
    } else {
        std::string msg = "Syntax error: expected end-of-line ";
        msg += "but saw '";
        msg += current().token;
        msg += "' instead.";
        throw SlpyError { locate(), msg };
    }
}

//
// x = ts.eat_name();
//
// Consumes the next token if it is an identifying name.
// Raises an error if it isn't.
//
// Returns the name as a string.
//
std::string TokenStream::eat_name(void) {
    if (at_name()) {
        std::string x = current().token;
        advance();
        return x;
    } else {
        std::string msg = "Syntax error: expected an identifier ";
        msg += "but saw '";
        msg += current().token;
        msg += "' instead.";
        throw SlpyError { locate(), msg };
    }
}

//
// n = ts.eat_number();
//
// Consumes the next token if it is the decimal encoding of an integer.
// Raises an error if it isn't.
//
// Returns the integer value.
//
int TokenStream::eat_number(void) {
    if (at_number()) {
        int nmbr = std::stoi(current().token);
        advance();
        return nmbr;
    } else {
        std::string msg = "Syntax error: expected an integer constant ";
        msg += "but saw '";
        msg += current().token;
        msg += "' instead.";
        throw SlpyError { locate(), msg };
    }
}

//
// s = ts.eat_string();
//
// Consumes the next token if it is a string literal.
// Raises an error if it isn't.
//
// Returns the string (transforming it by removing escaped characters).
//
std::string TokenStream::eat_string(void) {
    if (at_string()) {
        std::string strg = current().token;
        advance();
        return de_escape(strg.substr(1,strg.length()-2));
    } else {
        std::string msg = "Syntax error: expected a string literal ";
        msg += "but saw '";
        msg += current().token;
        msg += "' instead.";
        throw SlpyError { locate(), msg };
    }
}

/* * * * *
 *
 * class Tokenizer
 *
 * This implements a state-machine based lexical analyzer. When `lex`
 * is called, it repeatedly calls `advance_char` to process a SLPY
 * source file, building up a potential token as `curr_token`. When a
 * legal token has been processed, it "issues it" as a Token (the
 * string along with the start/end locations of it within the source
 * code) using `issue_token`. Issuing a token appends it to the in-progress
 * TokenStream being built. 
 * 
 * See TokenStream::lex for details.
 *
 * * * * */

//
// These are the states of the state machine used for lex analysis.
//
enum TokenizerState { INIT, // At the start of a line, haven't seen a token.
                      WTHN, // Within a line, have seen at least one token.
                      TABS, // At the start of a line, processing indentation.
                      CMMT_INIT, // Processing an end-of-line comment.
                      CMMT_WTHN,
                      NMBR, // Processing decimal digits starting with 1-9.
                      ZERO, // Processing a 0 literal.
                      STRG, // Processing a string literal.
                      ESCP, // Processing `\n`, `\t` etc within string literal.
                      SLSH, // Processing `//` token.
                      IDEN_RSRV, // Processing an identifier name.
                      HALT  // Done! Saw EOF.
};

//
// tz = Tokenizer {f};
//
// Constructs a Tokenizer when given the name of a .slpy file.
// 
// Typical use of this module is:
//
//     Tokenizer   tz { "my_program.slpy" };
//     TokenStream ts = tz.lex();
//
Tokenizer::Tokenizer(char* src_file_name) : 
    src_name     { src_file_name },
    src_stream   { src_file_name },
    state        { INIT },
    curr_token   { },
    tokenstream  { src_file_name },
    row          { 1 },
    column       { 1 },
    start_row    { 1 },
    start_column { 1 }
{
    if (src_stream) {
        curr_char = src_stream.get();
        start_fresh_token();
    } else {
        throw SlpyError { Locn { src_file_name, -1, -1 },
                             "File not found." };
    }
}

// tz.advance_char()
//
// Moves the "cursor" of the file processing to the next character, updating
// the location of it.
//
void Tokenizer::advance_char(void) {
    if (curr_char == '\n') {
        row++;
        column = 1;
    } else if (curr_char == '\t') {
        column = column + (8 - (column - 1) % 8);
    } else {
        column++;
    }
    curr_char = src_stream.get();
}

// tz.start_fresh_token()
//
// Marks the position of a token that is about to be processed.
// Throws out any consumed characters that didn't yield a token,
// or were from the last token processed.
//
void Tokenizer::start_fresh_token(void) {
    start_row = row;
    start_column = column;
    curr_token = std::stringstream { };
}

//
// tz.issue_token()
//
// Append a token to the TokenStream being built. That token's string
// is held in curr_token, and the start of the token within the source
// file was marked with start_row/column.
//
void Tokenizer::issue_token() {
    Token tkn { curr_token.str(), start_row, start_column };
    tokenstream.append(tkn);
    start_fresh_token(); // Shouldn't be necessary, but probably doesn't hurt.
}

//
// tz.consume_char()
//
// Advances the cursor by one character, appending the current one to
// the token being constructed.
//
void Tokenizer::consume_char() {
    curr_token << curr_char;
    advance_char();
}

//
// tz.consume_char_as(s);
//
// Advances the cursor by one character, appending the string to
// the token being constructed instead.
//
void Tokenizer::consume_char_as(std::string s) {
    curr_token << s;
    advance_char();
}

//
// tz.consume_then_start_fresh();
//
// Advances the cursor by one character but then tosses out the
// consumed characters from the token being constructed.
// The tokenizer can now construct a fresh token.
//
void Tokenizer::consume_then_start_fresh(void) {
    consume_char();
    start_fresh_token();
}

//
// tz.consume_then_start_fresh();
//
// Advances the cursor by one character, adding it to the token being
// constructed.  Then issue that token.
//
void Tokenizer::consume_then_issue(void) {
    consume_char();
    issue_token();
}

//
// bail_with_error
//
// Need to make this an exception.
//
void Tokenizer::bail_with_error(std::string message) {
    throw SlpyError { Locn {src_name, row, column}, message };
}

//
// bail_with_error
//
// Need to make this an exception.
//
void Tokenizer::bail_with_char(std::string message) {
    throw SlpyError { Locn {src_name, row, column },
                         message + "'" + curr_char + "'" };
}

//
// ts = tz.lex();
//    
// This runs the lexical analyzer on its SLPY source.
// It returns the TokenStream resulting from processing that file.
//
// Typical use of this class is:
//
//     Tokenizer   tz { "my_program.slpy" };
//     TokenStream ts = tz.lex();
// 
TokenStream Tokenizer::lex(void) {
    do {
        
        switch (state) {
            
        case WTHN:
        case INIT:
            if (curr_char >= '1' && curr_char <= '9') {
                //
                // Is it a positive number?
                start_fresh_token();
                state = NMBR;  // => Process it.

            } else if (curr_char == '0') {
                //
                // Is it zero?
                start_fresh_token();
                consume_char();
                state = ZERO;  // => Process it.
            } else if (curr_char == '"') {
                //
                // Is it a string literal?
                start_fresh_token();
                consume_char();
                state = STRG;  // => Process until the end quote.
                
            } else if (curr_char >= 'a' && curr_char <= 'z') {
                //
                // Is it an identifier or a reserved word?
                start_fresh_token();
                state = IDEN_RSRV;
            } else if (curr_char >= 'A' && curr_char <= 'Z') {
                start_fresh_token();
                state = IDEN_RSRV;
            } else if (curr_char == '_') {
                start_fresh_token();
                state = IDEN_RSRV;  // => Process alpha-numeric-underscore.
                
            } else if (curr_char == '\n') {
                //
                // Is it an end-of-line?
                if (state == WTHN) {
                    start_fresh_token();
                    consume_then_issue();
                } else if (state == INIT) {
                    advance_char();
                } else {
                    bail_with_error("Bad state at newline.");
                }
                state = INIT; // => Start of a new line.
                
            } else if (curr_char == '#') {
                //
                // Is it an end comment?
                if (state == INIT) {
                    state = CMMT_INIT; // => Process the comment.
                } else if (state == WTHN) {
                    state = CMMT_WTHN;
                } else {
                    bail_with_error("Bad state within comment.");
                }
                
            } else if (curr_char == ' ' || curr_char == '\t') {
                //
                // Is it space or tab?
                if (state == INIT) {
                    // At the line start => Treat as tab.
                    start_fresh_token();
                    state = TABS;
                    
                } else {
                    // Otherwise ignore.
                    advance_char();
                    
                }

            } else {
                switch (curr_char) {
                    
                //
                // Is it a single-character operator or delimiter?
                case '=':
                case '+':
                case '-':
                case '*':
                case '(':
                case ')':
                    start_fresh_token();
                    consume_then_issue(); // => Issue it as a token.
                    break;
                    
                case '/':
                    start_fresh_token();
                    consume_char();
                    state = SLSH;
                    break;
                    
                case -1:
                //
                // Is it the end of the file?
                    state = HALT;         // => Stop.
                    break;
                    
                default:
                    bail_with_char("Unexpected token: ");
                    break;
                }
                break;
            }
            break;

        case SLSH:
            if (curr_char == '/') {
                consume_then_issue();
                state = WTHN;
            } else {
                bail_with_error("Expected a // operator.");
            }
            break;

        case ZERO:
            if (curr_char < '0' || curr_char > '9') {
                issue_token();
                state = WTHN;
            } else {
                bail_with_error("Non-zero integer literal starts with zero digit.");
            }
            break;
                
        case NMBR:
            //
            // Process the digits of a number.
            //
            if (curr_char >= '0' && curr_char <= '9') {
                consume_char();
            } else {
                //
                // When done => We're now within a line.
                //
                issue_token();
                state = WTHN;
            }
            break;
            
        case STRG:
            //
            // Process the rest of a string literal.
            //
            if (curr_char == '"') {
                //
                // When done => We're now within a line.
                //
                consume_then_issue();
                state = WTHN;
                
            } else if (curr_char == '\\') {
                //
                // Is it a special escaped character?
                //
                consume_char();
                state = ESCP;
                break;
                
            } else if (curr_char == '\n' || curr_char == '\t') {
                //
                // Hit tab or newline while processing string literal.
                // ERROR!
                //
                if (curr_char == '\n') {
                    bail_with_error("Line ended within string literal.");
                } else {
                    bail_with_error("Tab seen within string literal.");
                }
            } else {
                //
                // Other chars are just part of the string.
                //
                consume_char();
            }
            break;

        case ESCP:
            consume_char();
            state = STRG;
            break;

        case TABS:
            if (curr_char == ' ' || curr_char == '\t') {
                consume_char();
            } else if (curr_char == '#' || curr_char == '\n') {
                start_fresh_token();
                state = INIT;
            } else {
                issue_token();
                state = WTHN;
            }
            break;
            
        case IDEN_RSRV:
            //
            // Is it an alpha-numeric-underscore character?
            //
            if ((curr_char >= 'a' && curr_char <= 'z')
                || (curr_char >= 'A' && curr_char <= 'Z')
                || (curr_char >= '_')
                || (curr_char >= '0' && curr_char <= '9')) {

                //
                // It is part of the identifier or word.
                //
                consume_char();
            } else {
                issue_token();
                state = WTHN; // => Within a line.
            }
            break;

        case CMMT_INIT:
            //
            // Is it the end-of-line character?
            //
            if (curr_char == '\n') {
                
                state = INIT; // Process the newline as if alone.

            } else {
                //
                // Skip it.
                //
                advance_char();
            }
            break;
            
        case CMMT_WTHN:
            //
            // Is it the end-of-line character?
            //
            if (curr_char == '\n') {
                
                state = WTHN; // Process the newline as within a line.

            } else {
                //
                // Skip it.
                //
                advance_char();
            }
            break;
            
        case HALT:
            //
            // We're done!
            //
            break;
        }
        
    } while (state != HALT);
    
    return tokenstream;
}
    
/* * * * * 
 *
 * UTILITY FUNCTIONS for lexical analysis.
 *
 * * * * */

//
// is_string(s)
//
// Determines whether string `s` is a string literal. This simply
// checks whether the first and last characters are double-quotes.
//
// Returns `true` or `false` accordingly.
//
bool is_string(std::string s) {
    int slen = s.length();
    if (slen < 2) {
        return false;
    }
    return (s[0] == '\"' && s[slen-1] == '\"');
}

//
// is_number(s)
//
// Determines whether string `s` contains the decimal digits of an integer.
//
// Returns `true` or `false` accordingly.
//
bool is_number(std::string s) {
    if (s.length() == 0) {
        return false;
    }
    if (s.length() == 1 && s[0] == '0') {
        return true;
    }
    if (s[0] <= '0' || s[0] > '9') {
        return false;
    }
    for (char c : s) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

//
// is_name(s)
//
// Determines whether string `s` describes a valid identifier or reserved word.
//
// Returns `true` or `false` accordingly.
//
bool is_name(std::string s) {
    if (s.length() == 0) {
        return false;
    }
    if ((s[0] >= 'a' && s[0] <= 'z')
        || (s[0] >= 'A' && s[0] <= 'Z')
        || (s[0] == '_')) {
        for (char c : s) {
            if ((c >= 'a' && c <= 'z')
                || (c >= 'A' && c <= 'Z')
                || (c == '_')
                || (c >= '0' && c <= '9')) {
                continue;
            }
            return false;
        }
        return true;
    }
    return false;
}
