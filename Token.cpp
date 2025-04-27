#include <iostream>
#include <string>
#include <cctype>

#include "Token.h"

const std::string allTypeCodes[38] = { "IDENFR","INTCON","STRCON","MAINTK","CONSTTK",
                            "INTTK","BREAKTK","CONTINUETK","IFTK","ELSETK","NOT","AND","OR",
                            "WHILETK","GETINTTK","PRINTFTK","RETURNTK","PLUS","MINU","VOIDTK",
                            "MULT","DIV","MOD","LSS","LEQ","GRE","GEQ","EQL","NEQ" ,
                            "ASSIGN","SEMICN","COMMA","LPARENT","RPARENT",
                            "LBRACK","RBRACK","LBRACE","RBRACE" };
const std::string accordinglyWords[38] = { "VAR1","VAR2","VAR3","main",
                                "const","int","break","continue","if",
                                "else","!","&&","||","while",
                                "getint","printf","return","+",
                                "-","void","*","/","%","<","<=",
                                ">",">=","==","!=",
                                "=",";",",","(",")",
                                "[","]","{","}" };
Token::Token(std::string str, int line) {
    name = str;
    lineNum = line;
    type = getType();
}

int Token::getType() {
    for (int i = 3; i < 38; i++) {
        if (name.compare(accordinglyWords[i]) == 0) return i;
    }
    char c = name[0];
    if (std::isalpha(c) || c == '_') return 0;//ident
    else if (std::isdigit(c)) return 1;//intconst
    else if (c == '\"') return 2;//format strings
    return  -1;//wrong
}

std::string Token::getTypeStr() {
    if (type != -1) return allTypeCodes[type];
    return "wrong stuff";
}

std::string Token::getName() {
    return name;
}

std::string Token::outputTypeNWord() {
    if (type < 0) return name;
    return allTypeCodes[type] + " " + name;
}