#include "string"
#include "Token.h"
#include "Lexicality.h"
#include "vector"

#include "Syntax.h"
#include "ast.h"


//std::ofstream outt("update.txt");

Syntax::Syntax(std::vector<Token> _tokens) {
    tokens = std::move(_tokens);
    curIndex = 0;
    curFunc = nullptr;
    vm = new VirtualMachinary;
    //curDecl = nullptr;
}

void Syntax::moveOn() {
    //out << tokens[pointer++].outputTypeNWord() << std::endl;
    pointer++;
}

void Syntax::start() {
    compUnit();
}

bool Syntax::isSameType(int pointerPlace, const std::string& strr) {
    return tokens[pointerPlace].getTypeStr() == strr;
}

bool Syntax::compUnit() {
    indexes.push_back(0);
    while (isSameType(pointer, "CONSTTK")
        || (isSameType(pointer, "INTTK")
            && isSameType(pointer+1, "IDENFR")
            && !isSameType(pointer+2, "LPARENT"))
            ) {
        //out << "compunit1" << std::endl;
        if (isSameType(pointer, "CONSTTK")) {
            constDecl(comp.decls);
        } else {
            varDecl(comp.decls);
        }
    }
    comp.frame->values.resize(offset);
    offset = 0;
    isComp = false;
    while (isSameType(pointer, "VOIDTK")
        || (isSameType(pointer, "INTTK")
            && isSameType(pointer+1, "IDENFR"))
            ) {
        //out << "compunit2" <<std::endl;
        if (!funcDef()) return false;
    }
    if (!mainFuncDef()) return false;
    vm->compUnit = &comp;
    //outt << "<CompUnit>" << std::endl;
    return true;
}

void Syntax::constDecl(std::vector<Decl *> &declarations) {
    moveOn();
    //if (!isSameType(pointer, "INTTK")) return false;
    moveOn();
    declarations.push_back(constDef());
    while (isSameType(pointer, "COMMA")) {
        moveOn();
        declarations.push_back(constDef());
    }
    //if (!isSameType(pointer, "SEMICN")) return false;
    moveOn();
   // out << "<ConstDecl>" << std::endl;
    //return true;
}

Decl *Syntax::constDef() {
    auto *decl = new Decl;
    //curDecl = decl;
    //if(!isSameType(pointer, "IDENFR")) return false;
    std::string name = tokens[pointer].getName();
    decl->name = name;
    decl->setConst(true);
    moveOn();
    while(isSameType(pointer, "LBRACK")) {
        moveOn();
        decl->addDim();
        decl->dimValues.push_back(constExp());
        moveOn();
    }
    if (decl->dim == 0) {
        decl->offset = offset;
        offset++;
    } else {
        int size = 1;
        for (auto value : decl->dimValues) {
            size *= value;
        }
        decl->offset = offset;
        offset += size;
    }
    //if (!isSameType(pointer, "ASSIGN")) return false;
    moveOn();
    constInitVal(decl->values);
    for (auto *expr: decl->values) {
        decl->initVal.push_back(expr->initDim(vm));
    }
    decls.push_back(decl); //add to symboltable
    curIndex++;
    if (isComp) decl->isGlobal = true;
    //curFunc->vars.push_back(decl);
    //comp.addDecl(decl);
    //out << "<ConstDef>" << std::endl;
    return decl;
}

void Syntax::constInitVal(std::vector<Expression *> &args) {
    if (isSameType(pointer, "LBRACE")) {
        //out << "constinit1" <<std::endl;
        moveOn();
        if (!isSameType(pointer, "RBRACE")) {
            //out << "constinit2" << std::endl;
            constInitVal(args);
           // out << "constinit3" << std::endl;
            while (isSameType(pointer, "COMMA")) {
                moveOn();
                constInitVal(args);
            }
            //if (!isSameType(pointer, "RBRACE")) return false;
            moveOn();
        }
    } else {
        //out << "constinit4" <<std::endl;
        args.push_back(expression());
        //return true;
    }
    //out << "<ConstInitVal>" << std::endl;
}

void Syntax::varDecl(std::vector<Decl *> &declarations) {
    //if (!isSameType(pointer, "INTTK")) return false;
    moveOn();
    //out << "vardecl1" <<std::endl;
    declarations.push_back(varDef());
    while (isSameType(pointer, "COMMA")) {
        moveOn();
        declarations.push_back(varDef());
    }
    //if (!isSameType(pointer, "SEMICN")) return false;
    moveOn();
    //out << "<VarDecl>" << std::endl;
    //return true;
}

Decl *Syntax::varDef() {
    //if (!isSameType(pointer, "IDENFR")) return false;
    auto  *decl = new Decl;
    decl->name = tokens[pointer].getName();
    decl->setConst(false);
    moveOn();
    //out << "vardef1" <<std::endl;
    while (isSameType(pointer, "LBRACK")) {
        moveOn();
        decl->addDim();
        decl->dimValues.push_back(constExp());
        //if (!isSameType(pointer, "RBRACK")) return false;
        moveOn();
    }
    if (decl->dim == 0) {
        decl->offset = offset;
        offset++;
    } else {
        int size = 1;
        for (auto value : decl->dimValues) {
            size *= value;
        }
        decl->offset = offset;
        offset += size;
    }
    if (isSameType(pointer, "ASSIGN")) {
        moveOn();
        initVal(decl->values);
    } else { //全局变量未显示初始化，赋0.
        if (isComp) {
            if (decl->dim == 0) {
                auto *num = new Number(0);
                decl->values.push_back(num);
            } else {
                int size = 1;
                for (auto value : decl->dimValues) {
                    size *= value;
                }
                while (size--) {
                    auto *num = new Number(0);
                    decl->values.push_back(num);
                }
            }
        }
    }
    if (isComp) decl->isGlobal = true;
    //curFunc->vars.push_back(decl);
    decls.push_back(decl);
    curIndex++;
    //comp.addDecl(decl);
    //out << "<VarDef>" << std::endl;
    return decl;
}

void Syntax::initVal(std::vector<Expression *> &args) {
    if (isSameType(pointer, "LBRACE")) {
        moveOn();
        if (!isSameType(pointer, "RBRACE")) {
            initVal(args);
            while (isSameType(pointer, "COMMA")) {
                moveOn();
                initVal(args);
            }
            //if (!isSameType(pointer, "RBRACE")) return false;
            moveOn();
        }
    } else {
        args.push_back(expression());
    }
    //out << "<InitVal>" << std::endl;
}

bool Syntax::funcDef() {
    if (!funcType()) return false;
    auto *func = new Function;
    curFunc = func;
    curFunc->name = tokens[pointer].getName();
    if (!isSameType(pointer, "IDENFR")) return false;
    moveOn();
    if(!isSameType(pointer, "LPARENT")) return false;
    moveOn();
    int newIndex = curIndex;
    indexes.push_back(newIndex);
    if (!isSameType(pointer, "RPARENT")) {
        if (!funcFParams(curFunc->params)) return false;
    }
    if (!isSameType(pointer, "RPARENT")) return false;
    moveOn();
    functions.insert(std::pair<std::string, Function *>(curFunc->name, curFunc));
    comp.funcs.push_back(curFunc);
    isNextLevel = 1;
    curFunc->block = block();
    curFunc->frame->values.resize(offset);
    offset = 0;
    //out << "<FuncDef>" << std::endl;
    return true;
}

bool Syntax::mainFuncDef() {
    auto *func = new Function;
    curFunc = func;
    curFunc->name = "main";
    int newIndex = curIndex;
    indexes.push_back(newIndex);
    if (!isSameType(pointer, "INTTK")) return false;
    moveOn();
    if (!isSameType(pointer, "MAINTK")) return false;
    moveOn();
    if (!isSameType(pointer, "LPARENT")) return false;
    moveOn();
    if (!isSameType(pointer, "RPARENT")) return false;
    moveOn();
    isNextLevel = 1;
    curFunc->block = block();
    comp.funcs.push_back(curFunc);
    curFunc->frame->values.resize(offset);
    offset = 0;
    //out << "<MainFuncDef>" << std::endl;
    return true;
}

bool Syntax::funcType() {
    if (isSameType(pointer, "VOIDTK") || isSameType(pointer, "INTTK")) {
        moveOn();
        //out << "<FuncType>" << std::endl;
        return true;
    }
    return false;
}

bool Syntax::funcFParams(std::vector<Decl *> &args) {
    if (!funcFParam(args)) return false;
    while(isSameType(pointer, "COMMA")) {
        moveOn();
        if (!funcFParam(args)) return false;
    }
    //out << "<FuncFParams>" << std::endl;
    return true;
}

bool Syntax::funcFParam(std::vector<Decl *> &args) {
    auto *decl = new Decl;
    if (!isSameType(pointer, "INTTK")) return false;
    moveOn();
    if (!isSameType(pointer, "IDENFR")) return false;
    decl->name = tokens[pointer].getName();
    moveOn();
    if (isSameType(pointer, "LBRACK")) {
        moveOn();
        if (!isSameType(pointer, "RBRACK")) return false;
        moveOn();
        decl->addDim();
        while (isSameType(pointer, "LBRACK")) {
            moveOn();
            decl->addDim();
            decl->dimValues.push_back(constExp()); //this is not right?
            if (!isSameType(pointer, "RBRACK")) return false;
            moveOn();
        }
    }
    decl->offset = offset;
    offset++;
    args.push_back(decl);
    decls.push_back(decl);
    curIndex++;
    decl->isParam = true;
    //out << "<FuncFParam>" << std::endl;
    return true;
}

BlockTK *Syntax::block() {
    auto *blo = new BlockTK;
    if (isNextLevel == 0) {
        int newIndex = curIndex;
        indexes.push_back(newIndex);// update index and floor
    } else {
        isNextLevel = 0;
    }
    //if (!isSameType(pointer, "LBRACE")) return false;
    moveOn();
    while (!isSameType(pointer, "RBRACE")) {
        if (isSameType(pointer, "CONSTTK")) {
            constDecl(blo->declarations);
            for (auto declaration : blo->declarations) {
                auto *declTk = new DeclTK;
                declTk->decl = declaration;
                blo->statements.push_back(declTk);
            }
            blo->declarations.clear();
        } else if (isSameType(pointer, "INTTK")) {
            //out << "blockitem2" <<std::endl;
            varDecl(blo->declarations);
            for (auto declaration : blo->declarations) {
                auto *declTk = new DeclTK;
                declTk->decl = declaration;
                blo->statements.push_back(declTk);
                //outt << std::endl<<"blo stmt:"<<declTk->decl->name<<std::endl;
            }
            blo->declarations.clear();
        } else {
            blo->statements.push_back(stmt());
        }
    }
    int toDeleteNum = int(decls.size()) - indexes.back();
    for (int i = 0; i < toDeleteNum; i++) {
        decls.pop_back();
        curIndex--;
    }
    indexes.pop_back();
    moveOn();
    //out << "<Block>" << std::endl;
    return blo;
}

Statement *Syntax::stmt() {
    if (isSameType(pointer, "IFTK")) {
        auto *state = new IfTK;
        moveOn();
        //if (!isSameType(pointer, "LPARENT")) return false;
        moveOn();
        state->condition = cond();
        //if (!isSameType(pointer, "RPARENT")) return false;
        moveOn();
        state->stmts.push_back(stmt());
        if (isSameType(pointer, "ELSETK")) {
            moveOn();
            state->stmts.push_back(stmt());
        }
        return state;
    } else if (isSameType(pointer, "WHILETK")) {
        auto *state = new WhileTK;
        moveOn();
        //if (!isSameType(pointer, "LPARENT")) return false;
        moveOn();
        state->condition = cond();
        //if (!isSameType(pointer, "RPARENT")) return false;
        moveOn();
        state->stmt = stmt();
        return state;
    } else if (isSameType(pointer, "BREAKTK")
                || isSameType(pointer, "CONTINUETK")) {
        auto *state = new BrkOrConTK;
        state->isContinue = !isSameType(pointer, "BREAKTK");
        moveOn();
        //if (!isSameType(pointer, "SEMICN")) return false;
        moveOn();
        return state;
    } else if (isSameType(pointer, "RETURNTK")) {
        auto *state = new ReturnTK;
        moveOn();
        if (!isSameType(pointer, "SEMICN")) {
            state->hasReturnExp = true;
            state->exp = expression();
        }
        //if (!isSameType(pointer, "SEMICN")) return false;
        moveOn();
        return state;
    } else if (isSameType(pointer, "LBRACE")) {
        return block();
    } else if (isSameType(pointer, "PRINTFTK")) {
        auto *state = new PrintTK;
        moveOn();
        //if (!isSameType(pointer, "LPARENT")) return false;
        moveOn();
        //if (!isSameType(pointer, "STRCON")) return false;
        state->formatString = tokens[pointer].getName();
        moveOn();
        while (isSameType(pointer, "COMMA")) {
            moveOn();
            state->exps.push_back(expression());
        }
        //if (!isSameType(pointer, "RPARENT")) return false;
        moveOn();
        //if (!isSameType(pointer, "SEMICN")) return false;
        moveOn();
        return state;
    } else if (isSameType(pointer, "SEMICN")) {
        auto *state = new ExpTK;
        moveOn();
        return state;
    } else if (isSameType(pointer, "LPARENT") || isSameType(pointer, "INTCON")
                || isSameType(pointer, "PLUS") || isSameType(pointer, "MINU")
                || isSameType(pointer, "NOT")) {
        auto *state = new ExpTK;
        state->hasExp = true;
        state->exp = expression();
        return state;
    } else { //lval=exp; exp(start with lval); lval=getint();
        int flag = 0,place = 0, i;
        bool which_way = false;
        for (i = pointer; i < int(tokens.size()); i++) {
            if (isSameType(i, "SEMICN")) {
                flag = 1;
                place = i;
                break;
            }
        }
        if (flag == 1) {
            for (i = pointer; i < place; i++) {
                if (isSameType(i, "ASSIGN")) {
                    which_way = true;
                    break;
                }
            }
        }
        if (which_way) {
            auto *expr = lVal(true);
            //if (!isSameType(pointer, "ASSIGN")) return false;
            moveOn();
            if (isSameType(pointer, "GETINTTK")) {
                auto *state = new GetintTK;
                state->lval = expr;
                moveOn();
                //if (!isSameType(pointer, "LPARENT")) return false;
                moveOn();
                //if (!isSameType(pointer, "RPARENT")) return false;
                moveOn();
                //if (!isSameType(pointer, "SEMICN")) return false;
                moveOn();
                return state;
            } else {
                auto *state = new AssignTK;
                state->lval = expr;
                state->exp = expression();
                //if (!isSameType(pointer, "SEMICN")) return false;
                moveOn();
                return state;
            }
        } else {
            auto *state = new ExpTK;
            state->hasExp = true;
            state->exp = expression();
            //if (!isSameType(pointer, "SEMICN")) return false;
            moveOn();
            return state;
        }
    }
    //out << "<Stmt>" << std::endl;
    //return true;
}

Expression *Syntax::expression() {
    return addExp();
}

Expression *Syntax::cond() {
    return lOrExp();
}

Expression *Syntax::lVal(bool isLeft) {
    auto *expr = new LVal(isLeft);
    std::string name = tokens[pointer].getName();
    expr->ident = name;
    moveOn();
    while (isSameType(pointer, "LBRACK")) {
        moveOn();
        expr->dims.push_back(expression());
        moveOn();
    }
    expr->findAim(decls);
    //out << "<LVal>" << std::endl;
    return expr;
}

Expression *Syntax::primaryExp() {
    if (isSameType(pointer, "LPARENT")) {
        moveOn();
        auto *expr = expression();
        moveOn();
        return expr;
    } else if (isSameType(pointer, "IDENFR")) {
        return lVal(false);
    }
    return number();
    //out << "<PrimaryExp>" << std::endl;
}

Expression *Syntax::number() {
    int number = std::stoi(tokens[pointer].getName());
    auto *expr = new Number(number);
    moveOn();
    //out << "<Number>" << std::endl;
    return expr;
}

Expression *Syntax::unaryExp() {
    if (isSameType(pointer, "PLUS")) {
        moveOn();
        return unaryExp();
    } else if (isSameType(pointer, "MINU")) {
        auto numberTemp = new Number(0);
        moveOn();
        auto *expr = unaryExp();
        auto *lc = new Binary(numberTemp, expr, operatorType::minus);
        return lc;
    } else if (isSameType(pointer, "NOT")) {
        auto numberTemp = new Number(0);
        moveOn();
        auto *expr = unaryExp();
        auto *lc = new Binary(numberTemp, expr, operatorType::eql);
        return lc;
    } else if (!isSameType(pointer, "IDENFR")) {
        return primaryExp();
    } else {
        if (isSameType(pointer+1, "LPARENT")) {
            auto *expr = new Call;
            expr->ident = tokens[pointer].getName();
            moveOn();
            moveOn();
            if (!isSameType(pointer, "RPARENT")) {
                funcRParams(expr->args);
            }
            moveOn();
            expr->findAim(functions);
            return expr;
        } else {
            return primaryExp();
        }
    }
    //out << "<UnaryExp>" << std::endl;
}

void Syntax::funcRParams(std::vector<Expression *> &args) {
    args.push_back(expression());
    while (isSameType(pointer, "COMMA")) {
        moveOn();
        args.push_back(expression());
    }
    //out << "<FuncRParams>" << std::endl;
}

Expression *Syntax::mulExp() {
    auto *lc = unaryExp();
    while (isSameType(pointer, "MULT") || isSameType(pointer, "DIV")
           || isSameType(pointer, "MOD")) {
        //out << "<AddExp>" << std::endl;
        operatorType oper;
        if (isSameType(pointer, "MULT")) oper = mul;
        else if (isSameType(pointer, "DIV")) oper = divv;
        else oper = mod;
        moveOn();
        auto *rc = unaryExp();
        lc= new Binary(lc, rc, oper);
    }
    //out << "<AddExp>" << std::endl;
    return lc;
}

Expression *Syntax::addExp() {
    auto *lc = mulExp();
    while (isSameType(pointer, "PLUS") || isSameType(pointer, "MINU")) {
        //out << "<AddExp>" << std::endl;
        operatorType oper = isSameType(pointer, "PLUS") ? plus : minus;
        moveOn();
        auto *rc = mulExp();
        lc= new Binary(lc, rc, oper);
    }
    //out << "<AddExp>" << std::endl;
    return lc;
}

Expression *Syntax::relExp() {
    auto *lc = addExp();
    while (isSameType(pointer, "GRE") || isSameType(pointer, "LSS")
           || isSameType(pointer, "GEQ") || isSameType(pointer, "LEQ")) {
        //out << "<AddExp>" << std::endl;
        operatorType oper;
        if (isSameType(pointer, "GRE")) oper = gre;
        else if (isSameType(pointer, "LSS")) oper = lss;
        else if (isSameType(pointer, "GEQ")) oper = geq;
        else oper = leq;
        moveOn();
        auto *rc = addExp();
        lc= new Binary(lc, rc, oper);
    }
    //out << "<AddExp>" << std::endl;
    return lc;
}

Expression *Syntax::eqExp() {
    auto *lc = relExp();
    while (isSameType(pointer, "EQL") || isSameType(pointer, "NEQ")) {
        //out << "<AddExp>" << std::endl;
        operatorType oper = isSameType(pointer, "EQL") ? eql : neq;
        moveOn();
        auto *rc = relExp();
        lc= new Binary(lc, rc, oper);
    }
    //out << "<AddExp>" << std::endl;
    return lc;
}

Expression *Syntax::lAndExp() {
    auto *lc = eqExp();
    while (isSameType(pointer, "AND")) {
        //out << "<AddExp>" << std::endl;
        operatorType oper = andd;
        moveOn();
        auto *rc = eqExp();
        lc= new Binary(lc, rc, oper);
    }
    //out << "<AddExp>" << std::endl;
    return lc;
}

Expression *Syntax::lOrExp() {
    //outt<<tokens[pointer].getName()<<std::endl;
    auto *lc = lAndExp();
    while (isSameType(pointer, "OR")) {
        //out << "<AddExp>" << std::endl;
        operatorType oper = orr;
        moveOn();
        auto *rc = lAndExp();
        lc= new Binary(lc, rc, oper);
    }
    //out << "<AddExp>" << std::endl;
    return lc;
}

int Syntax::constExp() {
    return addExp()->initDim(vm);
}

void Syntax::vmRun() {
    vm->frames.push_back(vm->compUnit->frame);//总栈先放一个全局的栈帧
    for (auto *decl : vm->compUnit->decls) {
        //outt << decl->name << "'s index: "<<decl->offset<<std::endl;
        auto *decltk = new DeclTK;
        decltk->decl = decl;
        decltk->execute(vm);
    }
    //vm->compUnit->funcs.back()->block->execute(vm);*/
    auto *call = new Call;
    call->aim = vm->compUnit->funcs.back();
    //outt <<"framesizeofmain!"<< vm->compUnit->funcs.back()->frame->values.size()<<std::endl;
    call->calculate(vm);
}


/*
void Syntax::update() {
    outt << std::endl;
    int i = 0;
    for (auto *frame : vm->frames) {
        outt <<"frame index: "<< i << std::endl;
        for (auto value : frame->values) outt << " "<< value <<std::endl;
    }
}*/