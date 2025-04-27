#include <iostream>
#include <string>
#include <vector>

#include "Token.h"
#ifndef COMPILERP4_LEXICALITY_H
#define COMPILERP4_LEXICALITY_H


class Lexicality {
private:
    int lines;
    std::vector<Token> allTokens;
public:
    void lexAnalyze(char* s);
    std::vector<Token> getTokens();
    int getLines();
    void printTokens();
    explicit Lexicality(int lines);
};


#endif //COMPILERP4_LEXICALITY_H
