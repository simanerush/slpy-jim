#include <sstream>
#include "slpy-util.hh"

//
// slpy-util.cc
//
// Implementation of some utilities for the SLPY interpreter.
//
// See the header (.hh) file for details.
//

//
// s = slpy_message(lo,ms);
//
// Builds and returns a string `s` that gives a SLPY error message `ms`
// along with information `lo` about the place in the source file where
// the error occurs.
//
const std::string slpy_message(Locn lo, std::string ms) {
    std::stringstream ss { };
    ss << lo.source_name << ":";
    if (lo.column > 0 && lo.row > 0) {
        ss << lo.row << ":" << lo.column << ":";
    }
    ss << "\n\t" << ms << "\n";
    return ss.str();
}

//
// class SlpyError
//
//  - the type of exceptions thrown by the SLPY interpreter
//
SlpyError::SlpyError(Locn lo, std::string ms) :
    std::exception { },
    message { slpy_message (lo, ms) }
{ }

const char* SlpyError::what() {
    return message.c_str();
}

//
// de_escape(s)
//
// Builds a string `t` from string `s` where all the escape sequences
// (e.g. `\\`, `\n`) have been replaced by their actual characters.
//
// Returns that de-escaped string.
//
std::string de_escape(std::string s) {
    std::stringstream de_s;
    bool escape = false;
    for (char c: s) {
        if (escape) {
            if (c == 'n') {
                de_s << '\n';
            } else if (c == 't') {
                de_s << '\t';
            } else if (c == '\\') {
                de_s << '\\';
            } else if (c == '"') {
                de_s << '"';
            }
            escape = false;        
        } else if (c == '\\') {
            escape = true;
        } else {
            de_s << c;
        }
    }
    return de_s.str();
}

//
// re_escape(s)
//
// Builds a string `t` from string `s` where all special characters
// (e.g tab, end of line, etc) are replaced by their escape sequences
// (e.g. `\t`, `\n`, etc).
//
// Returns that escaped string.
//
std::string re_escape(std::string s) {
    std::stringstream re_s;
    for (char c: s) {
        if (c == '\n') {
            re_s << "\\n";
        } else if (c == '\t') {
            re_s << "\\t";
        } else if (c == '\\') {
            re_s << "\\\\";
        } else if (c == '"') {
            re_s << "\\\"";
        } else {
            re_s << c;
        }
    }
    return re_s.str();
}


