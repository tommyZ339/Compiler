#include <iostream>
#include <fstream>

#include "Token.h"
#include "Lexicality.h"
#include "Syntax.h"
using  namespace  std;
int main() {
    ifstream fp("testfile.txt" );
    string line,str;
    char *c;
    if (!fp) {
        cout << "cannot open the file" << endl;
        return -1;
    }
    while (getline(fp, line)) {
        line += '\n';
        str += line;
    } //make file into string
    //cout << str;
    fp.close();
    c = &str[0];
    Lexicality lexicality(0);
    lexicality.lexAnalyze(c);
    //lexicality.printTokens();
    Syntax syntax(lexicality.getTokens());
    syntax.start();
    //printf("hello in syntax start!");
    syntax.vmRun();
    return 0;
}
