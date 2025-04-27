#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <fstream>

#include "Lexicality.h"

void Lexicality::lexAnalyze(char *s) {

    while (true) {
       // std::cout << (void *) (s) << " : "<<*s<< std::endl;
        while (*s && std::isspace(*s)) {
            if (*s == '\n') {
                lines++;
            }// change line
            ++s;
        }// blanks

        if (!*s)
            return;

        if (*s == '/' && s[1] == '/') {
            *s = ' ';
            s[1] = ' ';
            s += 2;
            while (*s != '\n') {
                ++s;
            }
            ++s;
            continue;
        } //single line annotation(//)
        if (*s == '/' && s[1] == '*') {
            *s = ' ';
            s[1] = ' ';
            s +=2;
            while (!(*s == '*' && s[1] == '/')) {
                if (*s == '\n') lines++;
                *s = ' ';//replace
                ++s;
            }
            s += 2;
            continue;
        }// /* ....*/

        std::string tempStr;
        if (std::isalpha(*s) || *s == '_') {
            while (std::isalpha(*s) || std::isdigit(*s) || *s == '_') {
                tempStr += s[0];
                ++s;
            }
        } else if (std::isdigit(*s)) {
            while (std::isdigit(*s)) {
                tempStr += s[0];
                ++s;
            }
        } else if (*s == '!'||*s == '<'||*s == '>'||*s == '=') {
            tempStr += s[0];
            if (s[1] == '=') tempStr += s[1], ++s;
            ++s;
        } else if (*s == '&' || *s == '|') {
            tempStr += s[0];
            if (s[1] == s[0]) tempStr += s[1], ++s;
            ++s;
        } else if (*s == '"') {
            tempStr += s[0];
            while (*(++s) != '"') {
                tempStr += s[0];
            }
            tempStr += '"';
            ++s;
        } else {
            tempStr += *(s++);
        }
        allTokens.emplace_back(tempStr, lines);
    }
}

std::vector<Token> Lexicality::getTokens() {
    return allTokens;
}

int Lexicality::getLines() {
    return lines;
}

void Lexicality::printTokens() {
    std::ofstream out("output.txt");
    for (auto &token : allTokens) {
        out << token.outputTypeNWord() << std::endl;
    }
}

Lexicality::Lexicality(int x) {
    lines = x;
}
