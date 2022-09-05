#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <functional>
#include <iostream>
#include <exception>
#include <algorithm>
#include <sstream>

#include "slpy-ast.hh"

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

Prgm::Prgm(Blck_ptr mn) :
    main {mn}
{ }

void Stmt::output(std::ostream& os) const {
    output(os,"");
}

void Asgn::output(std::ostream& os, std::string indent) const {
    os << indent;
    os << name << " = ";
    expn->output(os);
    os << std::endl;
}

void Asgn::exec(Ctxt& ctxt) const {
    ctxt[name] = expn->eval(ctxt);
}

void Pass::output(std::ostream& os, std::string indent) const {
    os << indent << "pass" << std::endl;
}

void Pass::exec(Ctxt& ctxt) const {
    // does nothing!
}
  
void Prnt::output(std::ostream& os, std::string indent) const {
    os << indent;
    os << "print";
    os << "(";
    expn->output(os);
    os << ")";
    os << std::endl;
}

void Prnt::exec(Ctxt& ctxt) const {
    std::cout << expn->eval(ctxt) << std::endl;
}

void Blck::output(std::ostream& os, std::string indent) const {
    for (Stmt_ptr s : stmts) {
        s->output(os,indent);
    }
}

void Blck::output(std::ostream& os) const {
    for (Stmt_ptr s : stmts) {
        s->output(os);
    }
}

void Blck::exec(Ctxt& ctxt) const {
    for (Stmt_ptr s : stmts) {
        s->exec(ctxt);
    }
}

void Plus::output(std::ostream& os) const {
    os << "(";
    left->output(os);
    os << " + ";
    rght->output(os);
    os << ")";
}

int Plus::eval(const Ctxt& ctxt) const {
    int lv = left->eval(ctxt);
    int rv = rght->eval(ctxt);
    return (lv + rv);
}

void Mnus::output(std::ostream& os) const {
    os << "(";
    left->output(os);
    os << " - ";
    rght->output(os);
    os << ")";
}

int Mnus::eval(const Ctxt& ctxt) const {
    int lv = left->eval(ctxt);
    int rv = rght->eval(ctxt);
    return (lv - rv);
}

void Tmes::output(std::ostream& os) const {
    os << "(";
    left->output(os);
    os << " * ";
    rght->output(os);
    os << ")";
}

int Tmes::eval(const Ctxt& ctxt) const {
    int lv = left->eval(ctxt);
    int rv = rght->eval(ctxt);
    return (lv * rv);
}

void IDiv::output(std::ostream& os) const {
    os << "(";
    left->output(os);
    os << " // ";
    rght->output(os);
    os << ")";
}

int IDiv::eval(const Ctxt& ctxt) const {
    int lv = left->eval(ctxt);
    int rv = rght->eval(ctxt);
    return (lv / rv);
}

void Nmbr::output(std::ostream& os) const {
    os << std::to_string(valu);
}

int Nmbr::eval(const Ctxt& ctxt) const {
    return valu;
}

void Lkup::output(std::ostream& os) const {
    os << name;
}

int Lkup::eval(const Ctxt& ctxt) const {
    return ctxt.at(name);
}

void Inpt::output(std::ostream& os) const {
    os << "input(\"" << re_escape(prpt) << "\")";
}

int Inpt::eval(const Ctxt& ctxt) const {
    int vl;
    std::cout << prpt;
    std::cin >> vl;
    return vl;
}

void Prgm::output(std::ostream& os) const {
    main->output(os);
}

void Prgm::run(void) const {
    Ctxt main_ctxt { };
    main->exec(main_ctxt);
}
