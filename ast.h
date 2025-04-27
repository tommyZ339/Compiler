#include "string"
#include "vector"
#include "map"
#include "stack"
#include "fstream"

#ifndef COMPILERP4_AST_H
#define COMPILERP4_AST_H

enum operatorType {
    plus, //0
    minus, //1
    mul, //2
    divv, //3
    mod, //4
    lss, //5
    gre, //6
    leq, //7
    geq, //8
    eql, //9
    neq, //10
    andd, //11
    orr //12
};


struct Frame {
    std::vector<int> values; // values in frame
};

struct CompUnit;

struct VirtualMachinary {
    std::vector<Frame *> frames;
    int *resolve_va(int va);
    CompUnit *compUnit = nullptr;
    std::ofstream out;
    VirtualMachinary();
    void update();
};

struct Expression {
   // virtual ~expression() = default;
    virtual int calculate(VirtualMachinary *vm) = 0;
    virtual int initDim(VirtualMachinary *vm) = 0;
};

struct Decl {
    std::string name;
    std::vector<int> dimValues;
    std::vector<Expression *> values;
    bool isConst = false; //can't be changed in lval = ...
    int dim=0;
    Decl(std::string, bool); // simple value
    void setConst(bool );
    void addDim();
    Decl();
    int offset = 0;
    bool isGlobal = false;
    bool isParam = false;
    std::vector<int> initVal;
};

struct LVal : Expression {
    std::string ident;
    Decl *aim = nullptr;
    std::vector<Expression *> dims;
    bool isLeft = false;
    LVal(bool isRight);
    LVal();
    void findAim(std::vector<Decl *> &decls);
    int calculate(VirtualMachinary *vm) override;
    int initDim(VirtualMachinary *vm) override;
};

struct Binary : Expression {
    Expression *lc;
    Expression *rc;
    operatorType op;
    int calculate(VirtualMachinary *vm) override;
    Binary(Expression *lefthand, Expression *righthand, operatorType oper);
    Binary();
    int initDim(VirtualMachinary *vm) override;
};

struct Function;

struct Call : Expression {
    std::string ident;
    Function *aim = nullptr;
    std::vector<Expression *> args;
    Call(std::string namee, std::vector<Expression *> argss);
    Call();
    void findAim(std::map<std::string, Function *> &functions);
    int calculate(VirtualMachinary *vm) override;
    int initDim(VirtualMachinary *vm) override;
};

struct Number : Expression {
    int num;
    int calculate(VirtualMachinary *vm) override;
    explicit Number(int number);
    int initDim(VirtualMachinary *vm) override;
};

struct Statement {
    virtual ~Statement() = default;
    virtual void execute(VirtualMachinary *vm) = 0;
};

struct IfTK : Statement {
    Expression *condition = nullptr;
    std::vector<Statement *> stmts;
    void execute(VirtualMachinary *vm) override;
};

struct WhileTK : Statement {
    Expression *condition = nullptr;
    Statement *stmt = nullptr;
    void execute(VirtualMachinary *vm) override;
};

struct AssignTK : Statement {
    Expression *lval = nullptr;
    Expression *exp = nullptr;
    void execute(VirtualMachinary *vm) override;
};

struct ExpTK :Statement {
    bool hasExp = false;
    Expression *exp = nullptr;
    void execute(VirtualMachinary *vm) override;
};

struct BlockTK : Statement {
    std::vector<Decl *> declarations;//temporarily store things
    std::vector<Statement *> statements;
    void execute(VirtualMachinary *vm) override;
};

struct DeclTK : Statement {
    Decl *decl;
    void execute(VirtualMachinary *vm) override;
};

struct BrkOrConTK : Statement {
    bool isContinue = false; // false MEANS BREAK; true MEANS CONTINUE
    void execute(VirtualMachinary *vm) override;
};

struct ReturnTK : Statement {
    bool hasReturnExp = false;
    Expression *exp = nullptr;
    void execute(VirtualMachinary *vm) override;
};

struct GetintTK : Statement {
    Expression *lval = nullptr;
    void execute(VirtualMachinary *vm) override;
};

struct PrintTK : Statement {
    std::string formatString;
    std::vector<Expression *> exps;
    void execute(VirtualMachinary *vm) override;
};


struct Function {
    std::string name;
    std::vector<Decl *> params;
    BlockTK *block = nullptr;
    Frame *frame = new Frame;
    Function();
};

struct CompUnit {
    std::vector<Decl *> decls;
    std::vector<Function *> funcs;
    Frame *frame = new Frame;
    //VirtualMachinary *vm = new VirtualMachinary;
};

class ReturnException : std::exception {
    int value;

public:
    int getValue() const;
    explicit ReturnException(int val);
};

class BreakOrContinueException : std::exception {
    bool isContinue;

public:
    bool getIsContinue() const;
    explicit BreakOrContinueException(bool bl);
};

#endif //COMPILERP4_AST_H
