#include "ast.h"
#include "iostream"
#include "vector"
#include "fstream"


std::ofstream out("pcoderesult.txt");

VirtualMachinary::VirtualMachinary() = default;

void VirtualMachinary::update() {
    out << std::endl;
    int i = 0;
    for (auto *frame : frames) {
        out <<"frame index: "<< i << std::endl;
        i++;
        for (auto value : frame->values) out << " "<< value <<std::endl;
    }
}

int *VirtualMachinary::resolve_va(int va) {
    //out << "Areuhere in resolve?" << std::endl;
    int fno = va >> 20;
    //out << " va fno= "<< fno<<std::endl;
    int offset = va & ((1 << 20) - 1);
    //out << " va off= "<<offset<<std::endl;
    return &frames[fno]->values[offset];
}

Decl::Decl(std::string namee, bool iscon) {
    name = std::move(namee);
    isConst = iscon;
}

void Decl::setConst(bool x) {
    isConst = x;
}

void Decl::addDim() {
    dim++;
}

Decl::Decl() = default;

Function::Function() = default;

LVal::LVal(bool reallyLeft) {
    isLeft = reallyLeft;
}

int LVal::calculate(VirtualMachinary *vm) {
    // out << "Areuhere?" << std::endl;
   // out <<"the ident is:"<< ident <<std::endl;
    int sizeLVal = int(dims.size());
    int sizeDecl = aim->dim;
   // out << "sizelval " <<sizeLVal<< " sizedecl "<<sizeDecl<<std::endl;
    int fno;
    int address;
    //build virtual address in the frame
    if (aim->isGlobal) {
        fno = 0;
    } else {
        fno = (int(vm->frames.size()) - 1) << 20;
    }
    //out << "fno "<<(fno>>20)<<std::endl;
    if (aim->isParam) {
        //out <<"fno: " <<fno<<"  offset: "<<aim->offset<<std::endl;
        //out << "sizedecl!"<<sizeDecl<<std::endl;
        //out<<"aereuhere in size?"<<std::endl;
        if (sizeDecl == 0) {
            //out << "areuhere in lc!!!a fku"<<std::endl;
            address= aim->offset + fno;
        } else {
            int valueInFrame = vm->frames[fno>>20]->values[aim->offset];
            //out << "valueinframe:"<<valueInFrame<<std::endl;
            int actualFno = (valueInFrame >>20) << 20;
            int actualOffset = (valueInFrame << 20) >> 20;
            if (sizeDecl == 1) {
                if (sizeLVal == 1)
                {
                    address= actualFno+actualOffset+dims.front()->calculate(vm);
                    //out<<"areuhere in addr sizelval1  "<<address<<" actualfno="<<(actualFno>>20)<<" actualoffset=" << actualOffset<<" value:"<<*(vm->resolve_va(address))<<std::endl;
                    //out << (address>>20)<< "<-fno    offset->"<<((address<<20)>>20) << std::endl;
                    //out<<"asdfghj  "<<vm->frames[1]->values[17]<<std::endl;
                }
                else {
                    address=  actualFno+actualOffset;
                    return address;
                }
            } else {
                if (sizeLVal == 2) address= actualFno+actualOffset+dims.front()->calculate(vm)*aim->dimValues.front()+dims.back()->calculate(vm);
                else if (sizeLVal == 0) {
                    address = actualFno+actualOffset;
                    return address;
                } else {
                    address = actualFno+actualOffset + dims.front()->calculate(vm)*aim->dimValues.front();
                    return address;
                }
            }
        }
    } else {
        //out<<"areuhere? in not param" << std::endl;
        if (sizeDecl == 0) address= aim->offset + fno;
        else if (sizeDecl == 1) {
            if (sizeLVal == 0) {
                address= aim->offset + fno;
                return address;// int a[]; f(a)
            }
            else {
                address= aim->offset + dims.front()->calculate(vm) + fno;
            }
        } else {
            int dim2 = aim->dimValues.at(1);
            //  out << "dim2! "<<dim2<<std::endl;
            if (sizeLVal == 0) {
                address= aim->offset + fno;
                return address;// int a[][]; f(a)
            }
            else if (sizeLVal == 1) {
                //out << "areuhere? arrays2" <<std::endl;
                address= aim->offset + dim2*dims.front()->calculate(vm) + fno;
                //out<<"did the calculation work?" <<std::endl;
                return address; // int a[][]; f(a[])
            }
            else address= aim->offset + dim2 * dims.front()->calculate(vm) + dims.back()->calculate(vm) + fno;
        }
    }
    //virtualAddr = address;
    if (isLeft) return address;
    else return *(vm->resolve_va(address));
}

void LVal::findAim(std::vector<Decl *> &decls) {
    int i = int(decls.size()) - 1;
    for (;i>=0;i--) {
        if(decls.at(i)->name == ident) {
            aim = decls.at(i);
            return;
        }
    }
}

int LVal::initDim(VirtualMachinary *vm) {
    if (dims.empty()) return aim->initVal.front();
    else if (dims.size() == 1){
        int index = dims[0]->initDim(vm);
        return aim->initVal[index];
    } else {
        int dim2 = aim->dimValues[1];
        int index = dims[0]->initDim(vm) * dim2 + dims[1]->initDim(vm);
        return aim->initVal[index];
    }
}

int Binary::initDim(VirtualMachinary *vm) {
    if (op == plus) return (lc->initDim(vm) + rc->initDim(vm));
    if (op == minus) return (lc->initDim(vm) - rc->initDim(vm));
    if (op == mul) return (lc->initDim(vm) * rc->initDim(vm));
    if (op == divv) return (lc->initDim(vm) / rc->initDim(vm));
    if (op == mod) return (lc->initDim(vm) % rc->initDim(vm));
    if (op == lss) return (lc->initDim(vm)) < (rc->initDim(vm)) ? 1 : 0;
    if (op == gre) return (lc->initDim(vm)) > (rc->initDim(vm)) ? 1 : 0;
    if (op == leq) return (lc->initDim(vm)) <= (rc->initDim(vm)) ? 1 : 0;
    if (op == geq) return (lc->initDim(vm)) >= (rc->initDim(vm)) ? 1 : 0;
    if (op == eql) return  (lc->initDim(vm)) == (rc->initDim(vm)) ? 1 : 0;
    if (op == neq) return  (lc->initDim(vm)) == (rc->initDim(vm)) ? 0 : 1;
    if (op == andd) {
        if (lc->initDim(vm) != 0  && rc->initDim(vm) != 0) return 1;
        else return 0;
    }
    if (op == orr) {
        if (lc->initDim(vm) != 0 || rc->initDim(vm) != 0) return 1;
        else return 0;
    }
    std::cout << std::endl << "wrong in calculating!" << std::endl;
    return 0;
}

int Call::initDim(VirtualMachinary *) {
    out<<"wrong dimension initialization!"<<std::endl;
    return -1; //按理来说不能到这里
}

int Number::initDim(VirtualMachinary *vm) {
    return num;
}

LVal::LVal() = default;

Call::Call() = default;

int Call::calculate(VirtualMachinary *vm) {
    //out << "are you here in call"<<std::endl;
    auto *frame = new Frame;
    int size = int(aim->frame->values.size());
    frame->values.resize(size);
    //out <<std::endl<< "framesize of "<<aim->name<<" is: "<<size<<std::endl<<std::endl;
    int returnValue = 0;
    int argsSize = int(args.size());
    for (int i = 0;i<argsSize;i++) {
            frame->values[i] = args[i]->calculate(vm);
    }
    vm->frames.push_back(frame);
/*
    out << std::endl;
    int i = 0;
    for (auto *frame : vm->frames) {
        out <<"frame index: "<< i << std::endl;
        i++;
        for (auto value : frame->values) out << " "<< value <<std::endl;
    }
*/

    try{
        aim->block->execute(vm);
    } catch (ReturnException &e) {
        returnValue = e.getValue();
        //out << "did u catch?"<<returnValue<<std::endl;
    }
    vm->frames.pop_back();
    return returnValue;
}

void Call::findAim(std::map<std::string, Function *> &functions) {
    aim = functions[ident];
}

Binary::Binary(Expression *lefthand, Expression *righthand, operatorType oper) {
    lc = lefthand;
    rc = righthand;
    op = oper;
}

Binary::Binary() {
    lc = nullptr;
    rc = nullptr;
    op = andd;//randomly initialized!!
}

int Binary::calculate(VirtualMachinary *vm) {
    //out << "areuhere in binary?" << std::endl;
    // << "lc! "<< lc->calculate(vm);
    //out << " rc! "<< rc->calculate(vm)<<std::endl;
    //out << "oper! "<<op<<std::endl;
    if (op == plus) return (lc->calculate(vm) + rc->calculate(vm));
    if (op == minus) return (lc->calculate(vm) - rc->calculate(vm));
    if (op == mul) return (lc->calculate(vm) * rc->calculate(vm));
    if (op == divv) return (lc->calculate(vm) / rc->calculate(vm));
    if (op == mod) return (lc->calculate(vm) % rc->calculate(vm));
    if (op == lss) return (lc->calculate(vm)) < (rc->calculate(vm)) ? 1 : 0;
    if (op == gre) return (lc->calculate(vm)) > (rc->calculate(vm)) ? 1 : 0;
    if (op == leq) return (lc->calculate(vm)) <= (rc->calculate(vm)) ? 1 : 0;
    if (op == geq) return (lc->calculate(vm)) >= (rc->calculate(vm)) ? 1 : 0;
    if (op == eql) return  (lc->calculate(vm)) == (rc->calculate(vm)) ? 1 : 0;
    if (op == neq) return  (lc->calculate(vm)) == (rc->calculate(vm)) ? 0 : 1;
    if (op == andd) {
        if (lc->calculate(vm) != 0 && rc->calculate(vm) != 0) return 1;
        else return 0;
    }
    if (op == orr) {
        if (lc->calculate(vm) != 0 || rc->calculate(vm) != 0) return 1;
        else return 0;
    }
    std::cout << std::endl << "wrong in calculating!" << std::endl;
    return 0;
}

Call::Call(std::string namee, std::vector<Expression *> argss) {
    ident = std::move(namee);
    args = std::move(argss);
}

Number::Number(int number) {
    num = number;
}

int Number::calculate(VirtualMachinary *vm) {
    return num;
}

void AssignTK::execute(VirtualMachinary *vm) {
     int value = exp->calculate(vm);
     auto *actualAddr = vm->resolve_va(lval->calculate(vm));
     *actualAddr = value;
}

void WhileTK::execute(VirtualMachinary *vm) {
    //out<<"areuhere in while?"<<std::endl;
    while (true) {
        if (condition->calculate(vm) != 0) {
            try {
                stmt->execute(vm);
            } catch (BreakOrContinueException &e) {
                if (e.getIsContinue()) continue;
                else break;
            }
        } else break;
    }
}

void DeclTK::execute(VirtualMachinary *vm) {
    //out << "areuhere?" << decl->name << std::endl;
    if (!decl->values.empty()) {
        for (int i = 0; i < int(decl->values.size()); i++) {
            vm->frames.back()->values[decl->offset + i] = decl->values[i]->calculate(vm);
        }
    }
/*
    out << std::endl;
    int i = 0;
    for (auto *frame : vm->frames) {
        out <<"frame index: "<< i << std::endl;
        i++;
        for (auto value : frame->values) out << " "<< value <<std::endl;
    }
*/
}

void IfTK::execute(VirtualMachinary *vm) {
    //out << "areuhere in if?" << std::endl;
    //out <<"condition!!"<< condition->calculate(vm)<<std::endl;
    if (condition->calculate(vm) != 0) {
        stmts.front()->execute(vm);
    } else {
        //out<< "are you here in else?"<<std::endl;
        if (stmts.size() > 1) stmts.back()->execute(vm);
    }
}

void ExpTK::execute(VirtualMachinary *vm) {
    if (hasExp) exp->calculate(vm);
}

void ReturnTK::execute(VirtualMachinary *vm) {
    if (hasReturnExp) {
        //out << "are uhere? in return"<<std::endl;
        int returnValue = exp->calculate(vm);
        throw ReturnException(returnValue);
    } else
        throw ReturnException(0);
}

void GetintTK::execute(VirtualMachinary *vm) {
    //out << "Areuhere in getint?"<< std::endl;
   // out<<lval->calculate(vm)<<std::endl;
    int *addr = vm->resolve_va(lval->calculate(vm));
    //out << "actual addr = "<<addr<<std::endl;
    //out << "value "<<*addr<<std::endl;
    int n;
    std::cin >> n;
    //out << "input: "<<n<<std::endl;
    *addr = n;

    //out << "value "<<*addr<<std::endl;
    //out <<std::endl<< "here in getint!"<<std::endl;
    /*
    out << std::endl;
    int i = 0;
    for (auto *frame : vm->frames) {
        out <<"frame index: "<< i << std::endl;
        i++;
        for (auto value : frame->values) out << "value "<< value <<std::endl;
    }
*/
}

void BlockTK::execute(VirtualMachinary *vm) {
    for (auto *stmt : statements) stmt->execute(vm);
}

void PrintTK::execute(VirtualMachinary *vm) {
    //out << "areuhere in print?" << formatString<< std::endl;
    int i = 0;
    int stringSize = int(formatString.size());
    for (int j = 1;j<stringSize - 1;j++) { // cut "..." to ...
        if (formatString[j] == '"') continue;
        if (formatString[j] == '%') {
            out << exps[i]->calculate(vm);
            i++;
            j++;
            continue;
        }
        if (formatString[j] == '\\') {
            out << std::endl;
            j++;
            continue;
        }
        out << formatString[j];
    }
}

void BrkOrConTK::execute(VirtualMachinary *) {
    throw BreakOrContinueException(isContinue);
}

ReturnException::ReturnException(int val) {
    value = val;
}

int ReturnException::getValue() const {
    return value;
}

BreakOrContinueException::BreakOrContinueException(bool bl) {
    isContinue = bl;
}

bool BreakOrContinueException::getIsContinue() const {
    return isContinue;
}


