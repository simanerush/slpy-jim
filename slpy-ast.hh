#ifndef __SLPY_AST_H_
#define __SLPY_AST_H_

// slpy-ast.hh
//
// Object classes used by the parser to represent the syntax trees of
// Slpy programs.
//
// It defines a class for a Slpy program and its block of statements,
// and also the two abstract classes whose subclasses form the syntax
// trees of a program's code.
//
//  * Prgm - a Slpy program that consists of a block of statements.
//
//  * Blck - a series of Slpy statements.
//
//  * Stmt - subclasses for the various statments that can occur
//           block of code, namely assignment, print, and pass.
//           These get executed when the program runs.
//
//  * Expn - subclasses for the various integer-valued expressions
//           than can occur on the right-hand side of an assignment
//           statement. These get evaluated to compute a value.
//

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <iostream>
#include "slpy-util.hh"

//
// We "pre-declare" each AST subclass for mutually recursive definitions.
//

class Prgm;
class Blck;
//
class Stmt;
class Pass;
class Asgn;
class Prnt;
//
class Expn;
class Plus;
class Mnus;
class Tmes;
class IDiv;
class Inpt;
class IntC;
class Lkup;
class Nmbr;

//
// We alias some types, including pointers and vectors.
//

typedef std::string Name;
typedef std::unordered_map<Name,int> Ctxt;
//
typedef std::shared_ptr<Lkup> Lkup_ptr; 
typedef std::shared_ptr<Nmbr> Nmbr_ptr; 
typedef std::shared_ptr<Inpt> Inpt_ptr; 
typedef std::shared_ptr<Plus> Plus_ptr; 
typedef std::shared_ptr<Mnus> Mnus_ptr; 
typedef std::shared_ptr<Tmes> Tmes_ptr;
typedef std::shared_ptr<IDiv> IDiv_ptr;
//
typedef std::shared_ptr<Pass> Pass_ptr; 
typedef std::shared_ptr<Prnt> Prnt_ptr; 
typedef std::shared_ptr<Asgn> Asgn_ptr;
//
typedef std::shared_ptr<Prgm> Prgm_ptr; 
typedef std::shared_ptr<Blck> Blck_ptr; 
typedef std::shared_ptr<Stmt> Stmt_ptr; 
typedef std::shared_ptr<Expn> Expn_ptr;
//
typedef std::vector<Stmt_ptr> Stmt_vec;
typedef std::vector<Expn_ptr> Expn_vec;


//
//
// ************************************************************

// Syntax tree classes used to represent a Slpy program's code.
//
// The classes Blck, Stmt, Expn are all subclasses of AST.
//

//
// class AST 
//
// Cover type for all "abstract" syntax tree classes.
//

class AST {
private:
    Locn locn; // Location of construct in source code (for reporting errors).
public:
    AST(Locn lo) : locn {lo} { }
    virtual void output(std::ostream& os) const = 0;
    Locn where(void) const { return locn; }
};


//
// class Prgm
//
// An object in this class holds all the information gained
// from parsing the source code of a Slpy program. A program
// is a series of Slpy statements organized as a block.
//
// The method Prgm::run implements the Slpy interpreter. This runs the
// Slpy program, executing its statments, updating the state of
// program variables as a result, getting user input from the console,
// and outputting results to the console.  The interpreter relies on
// the Blck::exec, Stmt::exec, and Expn::eval methods of the various
// syntactic components that constitute the Prgm object.
//

class Prgm : public AST {
public:
    //
    Blck_ptr main;
    //
    Prgm(Blck_ptr mn, Locn lo) : main {mn}, AST {lo} { }

    //
    void run(void) const; // Execute the program by interpreting its code.
    void output(std::ostream& os) const; // Output formatted code.
};

//
// class Stmt
//
// Abstract class for program statment syntax trees,
//
// Subclasses are
//
//   Asgn - assignment statement "v = e"
//   Prnt - output statement "print(e1,e2,...,ek)"
//   Pass - statement that does nothing
//
// These each support the methods:
//
//  * exec(ctxt): execute the statement within the stack frame
//
//  * output(os), output(os,indent): output formatted Slpy code of
//        the statement to the output stream `os`. The `indent` string
//        gives us a string of spaces for indenting the lines of its
//        code.
//
//
class Stmt : public AST {
public:
    Stmt(Locn lo) : AST {lo} { }
    virtual void exec(Ctxt& ctxt) const = 0;
    virtual void output(std::ostream& os, std::string indent) const = 0;
    virtual void output(std::ostream& os) const;
};

//
// Asgn - assignment statement AST node
//
class Asgn : public Stmt {
public:
    Name     name;
    Expn_ptr expn;
    Asgn(Name x, Expn_ptr e, Locn l) : name {x}, expn {e}, Stmt {l}  { }
    void exec(Ctxt& ctxt) const;
    void output(std::ostream& os, std::string indent) const;
};

//
// Prnt - print statement AST node
//
class Prnt : public Stmt {
public:
    Expn_ptr expn;
    Prnt(Expn_ptr e, Locn l) : expn {e}, Stmt {l} { }
    void exec(Ctxt& ctxt) const;
    void output(std::ostream& os, std::string indent) const;
};


//
// Pass - pass statement AST node
//
class Pass : public Stmt {
public:
    Pass(Locn l) : Stmt {l} { }
    void exec(Ctxt& ctxt) const;
    void output(std::ostream& os, std::string indent) const;
};

//
// class Blck
//
// Represents a sequence of statements.
//
class Blck : public AST {
public:
    Stmt_vec stmts;
    Blck(Stmt_vec ss, Locn lo) : stmts {ss}, AST {lo}  { }
    void exec(Ctxt& ctxt) const;
    void output(std::ostream& os, std::string indent) const;
    void output(std::ostream& os) const;
};


//
// class Expn
//
// Abstract class for integer expression syntax trees,
//
// Subclasses are
//
//   Plus - binary operation + applied to two sub-expressions
//   Mnus - binary operation - applied to two sub-expressions
//   Tmes - binary operation * applied to two sub-expressions
//   Nmbr - integer constant expression
//   Lkup - variable access (i.e. "look-up") within function frame
//   Inpt - obtains an integer input (after output of a prompt)
//
// These each support the methods:
//
//  * eval(ctxt): evaluate the expression; return its result
//  * output(os): output formatted Slpy code of the expression.
//
class Expn : public AST {
public:
    Expn(Locn lo) : AST {lo} { }
    virtual int eval(const Ctxt& ctxt) const = 0;
};

//
// Plus - addition binary operation's AST node
//
class Plus : public Expn {
public:
    Expn_ptr left;
    Expn_ptr rght;
    Plus(Expn_ptr lf, Expn_ptr rg, Locn lo)
        : left {lf}, rght {rg}, Expn {lo}  { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

//
// Mnus - subtraction binary operation's AST node
//
class Mnus : public Expn {
public:
    Expn_ptr left;
    Expn_ptr rght;
    Mnus(Expn_ptr lf, Expn_ptr rg, Locn lo)
        : left {lf}, rght {rg}, Expn {lo}  { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

//
// Tmes - multiplication binary operation's AST node
//
class Tmes : public Expn {
public:
    Expn_ptr left;
    Expn_ptr rght;
    Tmes(Expn_ptr lf, Expn_ptr rg, Locn lo)
        : left {lf}, rght {rg}, Expn {lo}  { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

//
// IDiv - quotient binary operation's AST node
//
class IDiv : public Expn {
public:
    Expn_ptr left;
    Expn_ptr rght;
    IDiv(Expn_ptr lf, Expn_ptr rg, Locn lo)
        : left {lf}, rght {rg}, Expn {lo}  { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

//
// Nmbr - integer literal AST node
//
class Nmbr : public Expn {
public:
    int valu;
    Nmbr(int vl, Locn lo) : valu {vl}, Expn {lo} { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

//
// Lkup - variable use/look-up AST node
//
class Lkup : public Expn {
public:
    Name name;
    Lkup(Name nm, Locn lo) : name {nm}, Expn {lo} { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

//
// Inpt - input expression AST node
//
class Inpt : public Expn {
public:
    std::string prpt;
    Inpt(std::string pr, Locn lo) : prpt {pr}, Expn {lo} { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

//
// IntC - int conversion expression AST node
//
class IntC : public Expn {
public:
    Expn_ptr expn;
    IntC(Expn_ptr e, Locn l) : expn {e}, Expn {l} { }
    int eval(const Ctxt& ctxt) const;
    void output(std::ostream& os) const;
};

#endif
