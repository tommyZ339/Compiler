#include "vector"
#include "Token.h"
#include "fstream"
#include "map"
#include "ast.h"

#ifndef COMPILERP4_SYNTAX_H
#define COMPILERP4_SYNTAX_H


class Syntax {
private:
    int pointer = 0;
    std::vector<Token> tokens;
    bool compUnit();
    void constDecl(std::vector<Decl *> &declarations);
    void varDecl(std::vector<Decl *> &declarations);
    Decl *constDef();
    void constInitVal(std::vector<Expression *> &args);
    Decl *varDef();
    void initVal(std::vector<Expression *> &args);
    bool funcDef();
    bool mainFuncDef();
    bool funcType();
    bool funcFParams(std::vector<Decl *> &args);
    bool funcFParam(std::vector<Decl *> &args);
    BlockTK *block();
    Statement *stmt();
    Expression *expression();
    Expression *cond();
    Expression *lVal(bool isLeft);
    Expression *primaryExp();
    Expression *number();
    Expression *unaryExp();
    void funcRParams(std::vector<Expression *> &args);
    Expression *mulExp();
    Expression *addExp();
    Expression *relExp();
    Expression *eqExp();
    Expression *lAndExp();
    Expression *lOrExp();
    int constExp();
    bool isSameType(int, const std::string&);
    void moveOn();

public:
    explicit Syntax(std::vector<Token>);
    void start();
    void vmRun();
    //void end();
    std::map<std::string, Function *> functions;
    //符号表应该怎么存？first:怎么保存每一层的信息？在decl里记录属于的上一层？全局变量之类的置一张表。
    // 这个可以用一张符号表。对于每个函数，都单独设置一张符号表。包括main也要设置一张符号表。
    // 每进入一个block，需要一个计数器 但是应该怎么记录从哪里开始找呢？
    std::vector<Decl *> decls;
    std::vector<int> indexes;
    int curIndex;
    Function *curFunc;
    //Decl *curDecl;
    CompUnit comp;
    VirtualMachinary *vm;
    int isNextLevel = 0;//只有funcdef时可以让他置为1，意思是读到block的时候符号表并不会进入到下一层。
    bool isComp = true;
    int offset = 0;
    //void update();
};


#endif //COMPILERP4_SYNTAX_H
