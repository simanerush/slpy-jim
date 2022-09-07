#ifndef __slpy_util_
#define __slpy_util_

//
// slpy-util.hh
//
// Some useful utilities for the SLPY interpreter. Some are for error
// reporting, namely
//
//  * SlpyError    - an exception for reporting SLPY errors
//  * Locn         - a (filename, line number, column number) for an error
//  * slpy_message - builds an error string 
//
// Some are for converting string literals to their actual strings, and
// back, namely
//
//   * de_escape, re_escape
//

//
// class Locn
//
// Houses information about the SLPY source file's name, along with a
// line (the "row") and column within that source file.
//
// This is typically used to report errors in the SLPY source code.
//
class Locn {
public:
    std::string source_name;
    int row;
    int column;
    //
    Locn(std::string fn, int rw, int co)
        : source_name {fn}, row {rw}, column {co} { }
    Locn(void) : Locn {"",0,0} { }
};
    
//
// s = slpy_message(lo,ms);
//
// Builds and returns a string `s` that gives a SLPY error message
// `ms` along with information `lo` about the place in the source file
// where the error occurs.
//
const std::string slpy_message(Locn lo, std::string ms);

//
// class SlpyError
//
// Thrown when an error is discovered while processing a SLPY source
// file.
//
class SlpyError: public std::exception {
private:
    Locn location;
    const std::string message;
    
public:    
    SlpyError(Locn lo, std::string ms);
    const char* what();
};

//
// Utility functions for dealing with string literals.
//
std::string re_escape(std::string s); // Replace special chars with \d ones.
std::string de_escape(std::string s); // Replace \d sequences with actuals.

#endif
