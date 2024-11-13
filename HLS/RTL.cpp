#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "parser.h"
#include "Head.h"

int RTL::blockCountWidth(int cycle) {
    int width = 0;
    if(cycle == 0) {
        // 位宽最小为1
        width = 1;
    }
    else {
        // 位宽为log2(cycle)向上取整
        int temp = cycle;
        while(temp != 0){
            temp = temp >> 1;
            width++;
        }
    }
    return width;
}
std::string RTL::port(int portType, int width, const std::string& name) {
    std::string out;
    // 端口类型
    switch (portType)
    {
        case PORT_INPUT:
            out = "input";
            break;
        case PORT_OUTPUT:
            out = "output";
            break;
        default:
            break;
    }
    out += " ";
    // 端口位宽
    if(width != 1) {
        out += "[" + to_string(width - 1) + ":0] ";
    }
    // 端口名
    out += name;
    return out;
}
std::string RTL::var(int varType, int width, const std::string& name) {
    std::string out;
    // 变量类型
    switch (varType)
    {
        case VAR_WIRE:
            out = "wire";
            break;
        case VAR_REG:
            out = "reg";
            break;
        default:
            break;
    }
    out += " ";
    // 变量位宽
    if(width != 1) {
        out += "[" + to_string(width - 1) + ":0] ";
    }
    // 变量名
    out += name;
    return out;
}
std::string RTL::reg(const Function& f, int statementID) {
    std::string out;
    // 判断输入变量还是基本块内
    if(f.getStatements()[statementID]._blockID < 0) {
        out = f.getStatements()[statementID]._name + "_reg";
    }
    else {
        out = "bb" + to_string(f.getStatements()[statementID]._blockID) + "_reg" + to_string(f.getStatements()[statementID]._reg);
    }
    return out;
}
std::string RTL::operandReg(const Function &f, const Operand& a) {
    if(a._isConst) {
        return a._name;
    }
    else {
        return reg(f, std::stoi(a._name));
    }
}
std::string RTL::opName(const Function& f, int statementID) {
    int blockID = f.getStatements()[statementID]._blockID;
    int type = f.getStatements()[statementID]._type;
    int unitID = f.getStatements()[statementID]._opUnit;
    return opName(blockID, type, unitID);
}
std::string RTL::opName(int blockID, int type, int unitID) {
    std::string out;
    // 块序号
    out = "bb" + to_string(blockID) + "_";
    // 操作类型
    switch(type) {
        case OP_ASSIGN: out += "assign"; break;
        case OP_ADD:    out += "add";    break;
        case OP_SUB:    out += "sub";    break;
        case OP_MUL:    out += "mul";    break;
        case OP_DIV:    out += "div";    break;
        case OP_LOAD:   out += "load";   break;
        case OP_STORE:  out += "store";  break;
        case OP_BR:     out += "br";     break;
        case OP_LT:     out += "lt";     break;
        case OP_GT:     out += "gt";     break;
        case OP_LE:     out += "le";     break;
        case OP_GE:     out += "ge";     break;
        case OP_EQ:     out += "eq";     break;
        case OP_PHI:    out += "phi";    break;
        case OP_RET:    out += "ret";    break;
    }
    out += to_string(unitID);
    return out;
}
std::string RTL::operatorStr(int type) {
    std::string out;
    switch(type) {
        case OP_ASSIGN: out += "=";     break;
        case OP_ADD:    out += "+";     break;
        case OP_SUB:    out += "-";     break;
        case OP_MUL:    out += "*";     break;
        case OP_DIV:    out += "/";     break;
        case OP_LOAD:   break;
        case OP_STORE:  break;
        case OP_BR:     break;
        case OP_LT:     out += "<";     break;
        case OP_GT:     out += ">";     break;
        case OP_LE:     out += "<=";    break;
        case OP_GE:     out += ">=";    break;
        case OP_EQ:     out += "==";    break;
        case OP_PHI:    break;
        case OP_RET:    break;
    }
    return out;
}
std::string RTL::state(int blockNum, int blockEnable, bool idle, bool complete) {
    std::string out;
    // 先生成(blockNum + 2)个0，再把对应的0替换成1
    out = std::string(blockNum + 2, '0');
    if(idle) {
        out[0] = '1';
    }
    else if(complete) {
        out[1] = '1';
    }
    else {
        out[blockNum + 1 - blockEnable] = '1';
    }
    // 最后标出位宽和进制
    out = to_string(blockNum + 2) + "'b" + out;
    return out;
}

RTL::Module::Module(const Function& function) : _function(function) {
    // 统计输入信号
    for(int i = 0; i < function.getStatements().size(); i++) {
        if(function.getStatements()[i]._blockID == -1) {
            _input.push_back(i);
        }
    }
}
void RTL::Module::generateRTL() {
    std::cout << "Generating RTL..." << std::endl;
    int blockNum = _function.getGraph().getBlocknum();
    moduleDeclaration();
    portsDeclaration();
    varsDeclararion();
    stateMachine();
    moduleIO();
    moduleReturn();
    for(int i = 0; i < blockNum; i++){
        blockCount(i);
        blockReadWrite(i);
        for(int j = 0; j < NUM_OP_TYPE; j++) {
            for(int k = 0; k < _function.getGraph().getUnits(j)[i].size(); k++){
                opUnit(i, j, k);
            }
        }
        for(int j = 0; j < _function.getGraph().getRegisters()[i].size(); j++) {
            blockReg(i, j);
        }
    }
    moduleEnd();

    std::cout << "Writing RTL file..." << std::endl;
    std::string outputFileName = _function.getName() + ".v";
    std::ofstream out;
    out.open(outputFileName, std::ios::out);
    out << _moduleRTL;
    out.close();
    std::cout << "Generated output file " << outputFileName << std::endl;
}
void RTL::Module::moduleDeclaration() {
    // 模块声明
    _moduleRTL += "module " + _function.getName() + "(\n";
    _moduleRTL += "    clk,\n";
    _moduleRTL += "    rst,\n";
    _moduleRTL += "    start,\n";
    _moduleRTL += "    idle,\n";
    _moduleRTL += "    complete,\n";
    for(int i = 0; i < _input.size(); i++) {
        if(_function.getStatements()[_input[i]]._isArray) {
            _moduleRTL += "    " + _function.getStatements()[_input[i]]._name + "_addr" + ",\n";
            _moduleRTL += "    " + _function.getStatements()[_input[i]]._name + "_w_en" + ",\n";
            _moduleRTL += "    " + _function.getStatements()[_input[i]]._name + "_w_data" + ",\n";
            _moduleRTL += "    " + _function.getStatements()[_input[i]]._name + "_r_en" + ",\n";
            _moduleRTL += "    " + _function.getStatements()[_input[i]]._name + "_r_data" + ",\n";
        }
        else {
            _moduleRTL += "    " + _function.getStatements()[i]._name + ",\n";
        }
    }
    if(_function.getRetType() == RET_INT) {
        _moduleRTL += "    return,\n";
    }
    // 最后一个','删掉
    _moduleRTL.erase(_moduleRTL.size() - 2);
    _moduleRTL += "\n);\n";
    _moduleRTL += "\n";
}
void RTL::Module::portsDeclaration() {
    int blockNum = _function.getGraph().getBlocknum();
    // 端口声明：控制信号
    _moduleRTL += port(PORT_INPUT, 1, "clk") + ";\n";
    _moduleRTL += port(PORT_INPUT, 1, "rst") + ";\n";
    _moduleRTL += port(PORT_INPUT, 1, "start") + ";\n";
    _moduleRTL += port(PORT_OUTPUT, 1, "idle") + ";\n";
    _moduleRTL += port(PORT_OUTPUT, 1, "complete") + ";\n";
    // 端口声明：入口参数
    for(int i = 0; i < _input.size(); i++) {
        if(_function.getStatements()[_input[i]]._isArray) {
            _moduleRTL += port(PORT_OUTPUT, RAM_WIDTH, _function.getStatements()[_input[i]]._name + "_addr") + ";\n";
            _moduleRTL += port(PORT_OUTPUT, 1, _function.getStatements()[_input[i]]._name + "_w_en") + ";\n";
            _moduleRTL += port(PORT_OUTPUT, 32, _function.getStatements()[_input[i]]._name + "_w_data") + ";\n";
            _moduleRTL += port(PORT_OUTPUT, 1, _function.getStatements()[_input[i]]._name + "_r_en") + ";\n";
            _moduleRTL += port(PORT_INPUT, 32, _function.getStatements()[_input[i]]._name + "_r_data") + ";\n";
        }
        else {
            _moduleRTL += port(PORT_INPUT, 32, _function.getStatements()[_input[i]]._name) + ";\n";
        }
    }
    // 端口声明：返回值
    if(_function.getRetType() == RET_INT) {
        _moduleRTL += port(PORT_OUTPUT, 32, "return") +";\n";
    }
    _moduleRTL += "\n";
}
void RTL::Module::varsDeclararion() {
    int blockNum = _function.getGraph().getBlocknum();
    // 变量声明：状态变量
    _moduleRTL += var(VAR_WIRE, 1, "idle") + ";\n";
    _moduleRTL += var(VAR_WIRE, 1, "complete") + ";\n";
    for(int i = 0; i < blockNum; i++) {
        _moduleRTL += var(VAR_WIRE, 1, "bb" + to_string(i) + "_en") + ";\n";
    }
    _moduleRTL += var(VAR_REG, blockNum + 2, "state") + ";\n";
    _moduleRTL += var(VAR_REG, blockNum + 2, "last_state") + ";\n";
    _moduleRTL += var(VAR_REG, blockNum + 2, "next_state") + ";\n";
    // 变量声明：入口参数
    for(int i = 0; i < _input.size(); i++) {
        if(_function.getStatements()[_input[i]]._isArray) {
            _moduleRTL += var(VAR_REG, RAM_WIDTH, _function.getStatements()[_input[i]]._name + "_addr") + ";\n";
            _moduleRTL += var(VAR_REG, 1, _function.getStatements()[_input[i]]._name + "_w_en") + ";\n";
            _moduleRTL += var(VAR_REG, 32, _function.getStatements()[_input[i]]._name + "_w_data") + ";\n";
            _moduleRTL += var(VAR_REG, 1, _function.getStatements()[_input[i]]._name + "_r_en") + ";\n";
        }
        else {
            _moduleRTL += var(VAR_REG, 32, reg(_function, _input[i])) + ";\n";
        }
    }
    // 变量声明：返回值
    if(_function.getRetType() == RET_INT) {
        _moduleRTL += var(VAR_WIRE, 32, "return") + ";\n";
        _moduleRTL += var(VAR_REG, 32, "return_reg") + ";\n";
        _moduleRTL += var(VAR_REG, 32, "next_return") + ";\n";
    }
    // 变量声明：基本块的计数器
    for(int i = 0; i < blockNum; i++) {
        // 计数器
        _moduleRTL += var(VAR_REG, blockCountWidth(_function.getGraph().getBlockMaxTime()[i] + 1), "bb" + to_string(i) + "_count") + ";\n";
        _moduleRTL += var(VAR_WIRE, 1, "bb" + to_string(i) + "_complete") + ";\n";
    }
    // 变量声明：基本块对函数入口参数（数组）的读写
    for(int i = 0; i < blockNum; i++) {
        for(int j = 0; j < _input.size(); j++) {
            if(_function.getStatements()[_input[j]]._isArray) {
                _moduleRTL += var(VAR_REG, RAM_WIDTH, "bb" + to_string(i) + "_" + _function.getStatements()[_input[j]]._name + "_addr") + ";\n";
                _moduleRTL += var(VAR_REG, 1, "bb" + to_string(i) + "_" + _function.getStatements()[_input[j]]._name + "_w_en") + ";\n";
                _moduleRTL += var(VAR_REG, 32, "bb" + to_string(i) + "_" + _function.getStatements()[_input[j]]._name + "_w_data") + ";\n";
                _moduleRTL += var(VAR_REG, 1, "bb" + to_string(i) + "_" + _function.getStatements()[_input[j]]._name + "_r_en") + ";\n";
            }
        }
    }
    // 变量声明：基本块计算单元
    for(int i = 0; i < blockNum; i++) {
        for(int j = 0; j < NUM_OP_TYPE; j++) {
            for(int k = 0; k < _function.getGraph().getUnits(j)[i].size(); k++) {
                switch(j) {
                    case OP_ASSIGN:
                        _moduleRTL += var(VAR_WIRE, 32, opName(i, j, k) + "_out") + ";\n";
                        break;
                    case OP_ADD:
                    case OP_SUB:
                    case OP_MUL:
                    case OP_DIV:
                        _moduleRTL += var(VAR_REG, 32, opName(i, j, k) + "_op0") + ";\n";
                        _moduleRTL += var(VAR_REG, 32, opName(i, j, k) + "_op1") + ";\n";
                        _moduleRTL += var(VAR_WIRE, 32, opName(i, j, k) + "_out") + ";\n";
                        break;
                    case OP_LOAD:
                        _moduleRTL += var(VAR_WIRE, RAM_WIDTH, opName(i, j, k) + "_addr") + ";\n";
                        _moduleRTL += var(VAR_WIRE, 32, opName(i, j, k) + "_out") + ";\n";
                        break;
                    case OP_STORE:
                        _moduleRTL += var(VAR_WIRE, RAM_WIDTH, opName(i, j, k) + "_addr") + ";\n";
                        _moduleRTL += var(VAR_WIRE, 32, opName(i, j, k) + "_data") + ";\n";
                        break;
                    case OP_BR:
                        break;
                    case OP_LT:
                    case OP_GT:
                    case OP_LE:
                    case OP_GE:
                    case OP_EQ:
                        _moduleRTL += var(VAR_REG, 32, opName(i, j, k) + "_op0") + ";\n";
                        _moduleRTL += var(VAR_REG, 32, opName(i, j, k) + "_op1") + ";\n";
                        _moduleRTL += var(VAR_WIRE, 32, opName(i, j, k) + "_out") + ";\n";
                        break;
                    case OP_PHI:
                        {
                            // 先找到对应的算子
                            int statement;
                            for(int cycle = 0; _function.getGraph().getUnits(j)[i][k][cycle] > 0; cycle++) {
                                if(_function.getGraph().getUnits(j)[i][k][cycle] > 0) {
                                    statement = _function.getGraph().getUnits(j)[i][k][cycle];
                                    break;
                                }
                            }
                            // 再找到对应的操作数
                            const std::vector<Operand>& operands = _function.getStatements()[statement]._operands;
                            // 对于每一个phi操作相关的block，生成对应变量
                            for(int phiBlock = 0; 2 * phiBlock < operands.size(); phiBlock++) {
                                _moduleRTL += var(VAR_WIRE, 32, opName(i, j, k) + "_bb" + operands[2 * phiBlock + 1]._name) + ";\n";
                            }
                            _moduleRTL += var(VAR_REG, 32, opName(i, j, k) + "_out") + ";\n";
                        }
                        break;
                    case OP_RET:
                        break;
                }
            }
        }
    }
    // 变量声明：基本块寄存器
    for(int i = 0; i < blockNum; i++) {
        for(int j = 0; j < _function.getGraph().getRegisters()[i].size(); j++) {
            _moduleRTL += var(VAR_REG, 32, "bb" + to_string(i) + "_reg" + to_string(j)) + ";\n";
        }
    }
    _moduleRTL += "\n";
}
void RTL::Module::stateMachine() {
    _moduleRTL += "// State machine\n";
    int blockNum = _function.getGraph().getBlocknum();

    std::string state_idle = RTL::state(blockNum, -1, true, false);
    std::string state_complete = RTL::state(blockNum, -1, false, true);
    // 将状态state连接到输出信号和各个block的激活信号
    _moduleRTL += "assign {idle, complete";
    for(int i = blockNum - 1; i >= 0; i--) {
        _moduleRTL += ", bb" + to_string(i) + "_en";
    }
    _moduleRTL += "} = state;\n";
    // 状态转移
    _moduleRTL += "always @(posedge clk) begin\n";
    _moduleRTL += "    if(rst)\n";
    _moduleRTL += "        state <= " + state_idle + ";\n";
    _moduleRTL += "    else\n";
    _moduleRTL += "        state <= next_state;\n";
    _moduleRTL += "end\n";
    // 保留上一状态（用于phi操作）
    _moduleRTL += "always @(posedge clk) begin\n";
    _moduleRTL += "    if(rst)\n";
    _moduleRTL += "        last_state <= " + RTL::state(blockNum, 0) + ";\n";
    _moduleRTL += "    else if(next_state == " + state_idle + ")\n";
    _moduleRTL += "        last_state <= " + RTL::state(blockNum, 0) + ";\n";
    _moduleRTL += "    else if(state != next_state)\n";
    _moduleRTL += "        last_state <= state;\n";
    _moduleRTL += "    else\n";
    _moduleRTL += "        last_state <= last_state;\n";
    _moduleRTL += "end\n";
    // 次态
    _moduleRTL += "always @(*) begin\n";
    _moduleRTL += "    case(state)\n";
    for(int i = 0; i < blockNum; i++) {  // block激活的次态：有3种情况：返回、有条件跳转、无条件跳转
        _moduleRTL += "    " + RTL::state(blockNum, i) + ":\n";
        _moduleRTL += "        if(bb" + to_string(i) + "_complete)\n";
        if(_function.getBlockEnd()[i]._isReturn) {
            _moduleRTL += "            next_state = " + state_complete + ";\n";
        }
        else if(_function.getBlockEnd()[i]._branchCond >= 0) {
            _moduleRTL += "            next_state = " + RTL::reg(_function, _function.getBlockEnd()[i]._branchCond) \
                                                    + " ? " + RTL::state(blockNum, _function.getBlockEnd()[i]._branchTrue) \
                                                    + " : " + RTL::state(blockNum, _function.getBlockEnd()[i]._branchFalse) + ";\n";
        }
        else {
            _moduleRTL += "            next_state = " + RTL::state(blockNum, _function.getBlockEnd()[i]._branchTrue) + ";\n";
        }
        _moduleRTL += "        else\n";
        _moduleRTL += "            next_state = state;\n";
    }
    _moduleRTL += "    " + state_complete + ":\n";  // complete的次态
    _moduleRTL += "        next_state = " + state_idle + ";\n";
    _moduleRTL += "    " + state_idle + ":\n";  // idle的次态
    _moduleRTL += "        if(start)\n";
    _moduleRTL += "            next_state = " + RTL::state(blockNum, 0) + ";\n";
    _moduleRTL += "        else\n";
    _moduleRTL += "            next_state = state;\n";
    _moduleRTL += "    default:\n";
    _moduleRTL += "        next_state = " + state_idle + ";\n";
    _moduleRTL += "    endcase\n";
    _moduleRTL += "end\n";

    _moduleRTL += "\n";
}
void RTL::Module::moduleIO() {
    _moduleRTL += "// Function paramaters\n";
    // 根据输入类型生成对应信号
    // 数组：输出地址等信号
    // 非数组：将输入信号寄存
    int blockNum = _function.getGraph().getBlocknum();
    for(int i = 0; i < _input.size(); i++) {
        if(_function.getStatements()[_input[i]]._isArray) {
            _moduleRTL += "always @(*) begin\n";
            _moduleRTL += "    case(state)\n";
            for(int j = 0; j < blockNum; j++) {
                _moduleRTL += "    " + state(blockNum, j) + ": begin\n";
                _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_addr = bb" + to_string(j) + "_" + _function.getStatements()[_input[i]]._name + "_addr;\n";
                _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_w_en = bb" + to_string(j) + "_" + _function.getStatements()[_input[i]]._name + "_w_en;\n";
                _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_w_data = bb" + to_string(j) + "_" + _function.getStatements()[_input[i]]._name + "_w_data;\n";
                _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_r_en = bb" + to_string(j) + "_" + _function.getStatements()[_input[i]]._name + "_r_en;\n";
                _moduleRTL += "    end\n";
            }
            _moduleRTL += "    default: begin\n";
            _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_addr = 0;\n";
            _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_w_en = 0;\n";
            _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_w_data = 0;\n";
            _moduleRTL += "        " + _function.getStatements()[_input[i]]._name + "_r_en = 0;\n";
            _moduleRTL += "    end\n";
            _moduleRTL += "    endcase\n";
            _moduleRTL += "end\n";
        }
        else {
            _moduleRTL += "always @(posedge clk) begin\n";
            _moduleRTL += "    if(rst)\n";
            _moduleRTL += "        " + reg(_function, _input[i]) + " <= 0;\n";
            _moduleRTL += "    else if(start && state == " + state(blockNum, -1, true, false) + ")\n";
            _moduleRTL += "        " + reg(_function, _input[i]) + " <= " + _function.getStatements()[_input[i]]._name + ";\n";
            _moduleRTL += "    else\n";
            _moduleRTL += "        " + reg(_function, _input[i]) + " <= " + reg(_function, _input[i]) + ";\n";
            _moduleRTL += "end\n";
        }
    }
    _moduleRTL += "\n";
}
void RTL::Module::moduleReturn() {
    if(_function.getRetType() == RET_INT) {
        int blockNum = _function.getGraph().getBlocknum();
        _moduleRTL += "// Return value\n";
        _moduleRTL += "assign return = return_reg;\n";
        _moduleRTL += "always @(posedge clk) begin\n";
        _moduleRTL += "    if(rst)\n";
        _moduleRTL += "        return_reg <= 0;\n";
        _moduleRTL += "    else if(start && state == " + state(blockNum, -1, true, false) + ")\n";
        _moduleRTL += "        return_reg <= 0;\n";
        _moduleRTL += "    else\n";
        _moduleRTL += "        return_reg <= next_return;\n";
        _moduleRTL += "end\n";
        _moduleRTL += "always @(*) begin\n";
        _moduleRTL += "    case(state)\n";
        for(int i = 0; i < blockNum; i++) {
            if(_function.getBlockEnd()[i]._isReturn) {
                _moduleRTL += "    " + state(blockNum, i) + ":\n";
                _moduleRTL += "        if(bb" + to_string(i) + "_complete)\n";
                if(_function.getBlockEnd()[i]._returnValue._isConst) {
                    _moduleRTL += "            next_return = " + _function.getBlockEnd()[i]._returnValue._name + ";\n";
                }
                else {
                    _moduleRTL += "            next_return = " + reg(_function, std::stoi(_function.getBlockEnd()[i]._returnValue._name)) + ";\n";
                }
                _moduleRTL += "        else\n";
                _moduleRTL += "            next_return = return_reg;\n";
            }
        }
        _moduleRTL += "    default:\n";
        _moduleRTL += "        next_return = return_reg;\n";
        _moduleRTL += "    endcase\n";
        _moduleRTL += "end\n";
        _moduleRTL += "\n";
    }
}
void RTL::Module::blockCount(int blockID) {
    _moduleRTL += "// Basic block " + to_string(blockID) + "\n";
    int cycle = _function.getGraph().getBlockMaxTime()[blockID] + 1;
    _moduleRTL += "always @(posedge clk) begin\n";
    _moduleRTL += "    if(rst)\n";
    _moduleRTL += "        bb" + to_string(blockID) + "_count <= 0;\n";
    _moduleRTL += "    else if(bb" + to_string(blockID) + "_en)\n";
    _moduleRTL += "        bb" + to_string(blockID) + "_count <= (bb" + to_string(blockID) + "_count == " + to_string(cycle) + ") ? 0 : " + "bb" + to_string(blockID) + "_count + 1;\n";
    _moduleRTL += "    else\n";
    _moduleRTL += "        bb" + to_string(blockID) + "_count <= 0;\n";
    _moduleRTL += "end\n";
    _moduleRTL += "assign bb" + to_string(blockID) + "_complete = bb" + to_string(blockID) + "_en && (bb" + to_string(blockID) + "_count == " + to_string(cycle) + ");\n";
}
void RTL::Module::blockReadWrite(int blockID) {
    // 统计寄存器和运算单元（读写）
    const std::vector<std::vector<int>>& registers = _function.getGraph().getRegisters()[blockID];
    const std::vector<std::vector<int>>& loadUnits = _function.getGraph().getUnits(OP_LOAD)[blockID];
    const std::vector<std::vector<int>>& storeUnits = _function.getGraph().getUnits(OP_STORE)[blockID];

    int cycle = _function.getGraph().getBlockMaxTime()[blockID] + 1;

    // opReadWrite[Function的输入的编号][周期数]=算子编号
    std::vector<std::vector<int>> opReadWrite(_input.size(), std::vector<int>(cycle, -1));
    // 对于每个输入，遍历运算单元，判断每个周期是否有读写
    for(int i = 0; i < _input.size(); i++) {
        for(int j = 0; j < loadUnits.size(); j++) {
            for(int k = 0; k < cycle; k++) {
                if(loadUnits[j][k] > 0 && std::stoi(_function.getStatements()[loadUnits[j][k]]._operands[0]._name) == _input[i])
                    opReadWrite[i][k] = loadUnits[j][k];
            }
        }
    }
    for(int i = 0; i < _input.size(); i++) {
        for(int j = 0; j < storeUnits.size(); j++) {
            for(int k = 0; k < cycle; k++) {
                if(storeUnits[j][k] > 0 && std::stoi(_function.getStatements()[storeUnits[j][k]]._operands[0]._name) == _input[i])
                    opReadWrite[i][k] = storeUnits[j][k];
            }
        }
    }
    // 将整个基本块的RAM读写连接到对应的读写算子
    for(int i = 0; i < _input.size(); i++) {
        if(_function.getStatements()[_input[i]]._isArray == true) {
            std::string input_name = _function.getStatements()[_input[i]]._name;
            _moduleRTL += "always @(*) begin\n";
            _moduleRTL += "    case(bb" + to_string(blockID) + "_count)\n";
            for(int j = 0; j < cycle; j++) {
                if(opReadWrite[i][j] > 0 && _function.getStatements()[opReadWrite[i][j]]._type == OP_LOAD) {         // 读取
                    _moduleRTL += "    " + to_string(j) + ": begin\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_addr = " + opName(_function, opReadWrite[i][j]) + "_addr;\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_w_en = 0;\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_w_data = 0;\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_r_en = 1;\n";
                    _moduleRTL += "    end\n";
                }
                else if(opReadWrite[i][j] > 0 && _function.getStatements()[opReadWrite[i][j]]._type == OP_STORE) {   // 写入
                    _moduleRTL += "    " + to_string(j) + ": begin\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_addr = " + opName(_function, opReadWrite[i][j]) + "_addr;\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_w_en = 1;\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_w_data = " + opName(_function, opReadWrite[i][j]) + "_data;\n";
                    _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_r_en = 0;\n";
                    _moduleRTL += "    end\n";
                }
            }
            _moduleRTL += "    default: begin\n";
            _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_addr = 0;\n";
            _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_w_en = 0;\n";
            _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_w_data = 0;\n";
            _moduleRTL += "        bb" + to_string(blockID) + "_" + input_name + "_r_en = 0;\n";
            _moduleRTL += "    end\n";
            _moduleRTL += "    endcase\n";
            _moduleRTL += "end\n";
        }
    }
}
void RTL::Module::opUnit(int blockID, int type, int unitID) {
    int cycle = _function.getGraph().getBlockMaxTime()[blockID] + 1;

    // 统计计算单元
    const std::vector<std::vector<int>>& units = _function.getGraph().getUnits(type)[blockID];

    // 统计共享的算子和周期
    std::vector<int> sharedStatement;
    std::vector<int> sharedCycle;
    for(int i = 0; i < cycle; i++) {
        if(units[unitID][i] > 0) {
            sharedStatement.push_back(units[unitID][i]);
            sharedCycle.push_back(i);
        }
    }

    switch(type) {
        case OP_ASSIGN:
            _moduleRTL += "assign " + opName(blockID, type, unitID) + "_out = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[0]) + ";\n";
            break;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            // 加减乘除
            // 两个操作数
            if(sharedStatement.size() > 1) {
                _moduleRTL += "always @(*) begin\n";
                _moduleRTL += "    case(bb" + to_string(blockID) + "_count)\n";
                for(int i = 0; i < sharedCycle.size(); i++) {
                    _moduleRTL += "    " + to_string(sharedCycle[i]) + ": begin\n";
                    _moduleRTL += "        " + opName(blockID, type, unitID) + "_op0 = " + operandReg(_function, _function.getStatements()[sharedStatement[i]]._operands[0]) + ";\n";
                    _moduleRTL += "        " + opName(blockID, type, unitID) + "_op1 = " + operandReg(_function, _function.getStatements()[sharedStatement[i]]._operands[1]) + ";\n";
                    _moduleRTL += "    end\n";
                }
                _moduleRTL += "    default: begin\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op0 = 0;\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op1 = 0;\n";
                _moduleRTL += "    end\n";
                _moduleRTL += "    endcase\n";
                _moduleRTL += "end\n";
            }
            else {
                _moduleRTL += "always @(*) begin\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op0 = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[0]) + ";\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op1 = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[1]) + ";\n";
                _moduleRTL += "end\n";
            }
            // 计算结果
            _moduleRTL += "assign " + opName(blockID, type, unitID) + "_out = $signed(" + opName(blockID, type, unitID) + "_op0) " + operatorStr(type) + " $signed(" + opName(blockID, type, unitID) + "_op1);\n";
            break;
        case OP_LOAD:
            // 地址：operands[1]
            // 数组名：operands[0]对应的statement的name
            _moduleRTL += "assign " + opName(blockID, type, unitID) + "_addr = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[1]) + ";\n";
            _moduleRTL += "assign " + opName(blockID, type, unitID) + "_out = " + _function.getStatements()[std::stoi(_function.getStatements()[sharedStatement[0]]._operands[0]._name)]._name + "_r_data;\n";
            break;
        case OP_STORE:
            // 地址：operands[1]
            // 写入数据：operands[2]
            _moduleRTL += "assign " + opName(blockID, type, unitID) + "_addr = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[1]) + ";\n";
            _moduleRTL += "assign " + opName(blockID, type, unitID) + "_data = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[2]) + ";\n";
            break;
        case OP_BR:
            break;
        case OP_LT:
        case OP_GT:
        case OP_LE:
        case OP_GE:
        case OP_EQ:
            // 比较运算
            // 两个操作数
            if(sharedStatement.size() > 1) {
                _moduleRTL += "always @(*) begin\n";
                _moduleRTL += "    case(bb" + to_string(blockID) + "_count)\n";
                for(int i = 0; i < sharedCycle.size(); i++) {
                    _moduleRTL += "    " + to_string(i) + ": begin\n";
                    _moduleRTL += "        " + opName(blockID, type, unitID) + "_op0 = " + operandReg(_function, _function.getStatements()[sharedStatement[i]]._operands[0]) + ";\n";
                    _moduleRTL += "        " + opName(blockID, type, unitID) + "_op1 = " + operandReg(_function, _function.getStatements()[sharedStatement[i]]._operands[1]) + ";\n";
                    _moduleRTL += "    end\n";
                }
                _moduleRTL += "    default: begin\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op0 = 0;\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op1 = 0;\n";
                _moduleRTL += "    end\n";
                _moduleRTL += "    endcase\n";
                _moduleRTL += "end\n";
            }
            else {
                _moduleRTL += "always @(*) begin\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op0 = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[0]) + ";\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_op1 = " + operandReg(_function, _function.getStatements()[sharedStatement[0]]._operands[1]) + ";\n";
                _moduleRTL += "end\n";
            }
            // 计算结果
            _moduleRTL += "assign " + opName(blockID, type, unitID) + "_out = ($signed(" + opName(blockID, type, unitID) + "_op0) " + operatorStr(type) + " $signed(" + opName(blockID, type, unitID) + "_op1)) ? 1 : 0;\n";
            break;
        case OP_PHI:
            {
                const std::vector<Operand>& operands = _function.getStatements()[sharedStatement[0]]._operands;
                for(int j = 0; 2 * j < operands.size(); j++) {
                    _moduleRTL += "assign " + opName(blockID, type, unitID) + "_bb" + operands[2 * j + 1]._name + " = " + operandReg(_function, operands[2 * j]) + ";\n";
                }
                _moduleRTL += "always @(*) begin\n";
                for(int j = 0; 2 * j < operands.size(); j++) {
                    if(j == 0) {
                        _moduleRTL += "    if(";
                    }
                    else {
                        _moduleRTL += "    else if(";
                    }
                    _moduleRTL += "last_state[" + operands[2 * j + 1]._name + "])\n";
                    _moduleRTL += "        " + opName(blockID, type, unitID) + "_out = " + opName(blockID, type, unitID) + "_bb" + operands[2 * j + 1]._name + ";\n";
                }
                _moduleRTL += "    else\n";
                _moduleRTL += "        " + opName(blockID, type, unitID) + "_out = 0;\n";
                _moduleRTL += "end\n";
            }
            break;
        case OP_RET:
            break;
    }
}
void RTL::Module::blockReg(int blockID, int regID) {
    // 统计寄存器
    const std::vector<std::vector<int>>& registers = _function.getGraph().getRegisters()[blockID];

    _moduleRTL += "always @(posedge clk) begin\n";
    _moduleRTL += "    if(rst)\n";
    _moduleRTL += "        bb" + to_string(blockID) + "_reg" + to_string(regID) + " <= 0;\n";
    _moduleRTL += "    else if(bb" + to_string(blockID) + "_en)\n";
    _moduleRTL += "        case(bb" + to_string(blockID) + "_count)\n";

    int cycle = _function.getGraph().getBlockMaxTime()[blockID] + 1;
    int last_statement = -1;    // 记录上一个周期的算子，如果当前周期和上一周期算子相同，则寄存器数据保持（当作该周期没有绑定）
    for(int i = 0; i < cycle; i++) {
        if(registers[regID][i] > 0 && registers[regID][i] != last_statement) {  // 寄存器需要有效，且当前周期和上一周期算子不同
            _moduleRTL += "        " + to_string(i) + ":\n";
            _moduleRTL += "            bb" + to_string(blockID) + "_reg" + to_string(regID) + " <= " + opName(_function, registers[regID][i]) + "_out;\n";
            last_statement = registers[regID][i];
        }
    }
    _moduleRTL += "        default:\n";
    _moduleRTL += "            bb" + to_string(blockID) + "_reg" + to_string(regID) + " <= bb" + to_string(blockID) + "_reg" + to_string(regID) + ";\n";
    _moduleRTL += "        endcase\n";
    _moduleRTL += "    else\n";
    _moduleRTL += "        bb" + to_string(blockID) + "_reg" + to_string(regID) + " <= bb" + to_string(blockID) + "_reg" + to_string(regID) + ";\n";
    _moduleRTL += "end\n";
}
void RTL::Module::moduleEnd() {
    _moduleRTL += "\nendmodule\n";
}
