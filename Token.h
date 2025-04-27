#pragma once
#include <string>

#ifndef COMPILERP4_TOKEN_H
#define COMPILERP4_TOKEN_H

class Token {
private:
    std::string name;
    int lineNum;
    int type;
    int getType();

public:
    Token(std::string str, int line);
    std::string outputTypeNWord();
    std::string getTypeStr();
    std::string getName();
};


#endif //COMPILERP4_TOKEN_H
