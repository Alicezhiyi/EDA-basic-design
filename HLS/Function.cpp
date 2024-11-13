#include "parser.h"
#include "Head.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cstdlib>

Function::Function(const std::string& fileName) {
    using namespace std;

    // Parse LLVM IR
    cout << "Parsing..." << endl;
    parser::parser p(fileName);
    if(p.parse() != 0) {
        cout << "Parsing error!" << endl;
        exit(EXIT_FAILURE);
    }
    // Add branch / return statement
    int blockNum = p.get_basic_blocks().size();
    for(int i = 0; i < blockNum; i++) {
        auto& block = p.get_basic_blocks()[i];
        if(block.get_statements().size() != 0) {
            auto& lastStatement = block.get_statements()[block.get_statements().size() - 1];
            if(lastStatement.get_type() == OP_BR || lastStatement.get_type() == OP_RET)
                continue;
        }
        parser::statement branch;
        int branchBlockID = (i == blockNum - 1) ? 0 : (i + 1);
        branch.set_type(OP_BR);
        branch.set_num_oprands(1);
        branch.add_oprand(p.get_basic_blocks()[branchBlockID].get_label_name());
        block.add_statement(branch);
    }

    _name = p.get_function_name();
    _retType = p.get_ret_type();

    vector<parser::statement> blockEndStatement;
    // Function parameters
    for(auto& param : p.get_function_params()) {
        _statements.push_back(Statement(param._name, -1, param._array_flag));
    }
    int inputNum = _statements.size();
    // Push statements
    for(int i = 0; i < blockNum; i++) {
        auto& block = p.get_basic_blocks()[i];
        for(int j = 0; j < block.get_statements().size() - 1; j++) {
            auto& s = block.get_statements()[j];
            Statement temp(s.get_var(), i, false, s.get_type());
            for(int k = 0; k < s.get_num_oprands(); k++) {
                temp._operands.push_back(Operand(s.get_oprand(k), isdigit(s.get_oprand(k)[0])));
            }
            _statements.push_back(temp);
        }
        // Push branch / return
        blockEndStatement.push_back(block.get_statements()[block.get_statements().size() - 1]);
    }

    // name -> statementID
    map<string, int> statementID;
    for(int i = 0; i < _statements.size(); i++) {
        statementID[_statements[i]._name] = i;
    }
    // block label -> blockID
    map<string, int> blockID;
    for(int i = 0; i < blockNum; i++) {
        blockID[p.get_basic_blocks()[i].get_label_name()] = i;
    }

    // Block end information (branch/return)
    for(int i = 0; i < blockNum; i++) {
        _blockEnd.push_back(BlockEnd());
        switch(blockEndStatement[i].get_type()) {
            case OP_RET:
                _blockEnd[i]._isReturn = true;
                if(p.get_ret_type() == RET_INT) {
                    bool isConst = isdigit(blockEndStatement[i].get_oprand(0)[0]);
                    string returnValueName = isConst ? blockEndStatement[i].get_oprand(0) : to_string(statementID[blockEndStatement[i].get_oprand(0)]);
                    _blockEnd[i]._returnValue = Operand(returnValueName, isConst);
                }
                _blockEnd[i]._branchCond = -1;
                _blockEnd[i]._branchTrue = -1;
                _blockEnd[i]._branchFalse = -1;
                break;
            case OP_BR:
                if(blockEndStatement[i].get_num_oprands() == 1) {
                    _blockEnd[i]._isReturn = false;
                    _blockEnd[i]._branchCond = -1;
                    _blockEnd[i]._branchTrue = blockID[blockEndStatement[i].get_oprand(0)];
                    _blockEnd[i]._branchFalse = -1;
                }
                else {
                    _blockEnd[i]._isReturn = false;
                    _blockEnd[i]._branchCond = statementID[blockEndStatement[i].get_oprand(0)];
                    _blockEnd[i]._branchTrue = blockID[blockEndStatement[i].get_oprand(1)];
                    _blockEnd[i]._branchFalse = blockID[blockEndStatement[i].get_oprand(2)];
                }
                break;
        }
    }

    // Operand name -> statementID/blockID
    for(auto& s : _statements) {
        switch(s._type){
            case OP_ASSIGN:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_LOAD:
            case OP_STORE:
            case OP_LT:
            case OP_GT:
            case OP_LE:
            case OP_GE:
            case OP_EQ:
                for(auto &o : s._operands) {
                    if(!o._isConst) {
                        o._name = to_string(statementID[o._name]);
                    }
                }
                break;
            case OP_PHI:
                for(int i = 0; 2 * i < s._operands.size(); i++) {
                    if(!s._operands[2 * i]._isConst) {
                        s._operands[2 * i]._name = to_string(statementID[s._operands[2 * i]._name]);
                    }
                    s._operands[2 * i + 1]._name = to_string(blockID[s._operands[2 * i + 1]._name]);
                    s._operands[2 * i + 1]._isConst = false;
                }
                break;
            case OP_BR:
            case OP_RET:
                break;
        }
    }

    // Generate graph
    cout << "Generating graph..." << endl;
    for(int i = 0; i < _statements.size(); i++) {
        if(_statements[i]._blockID >= 0) {
            Vertex *v = new Vertex(_statements[i]._type, i);
            v->_blockID = _statements[i]._blockID;
            _graph.getVertexList().push_back(v);
        }
    }
    for(auto& v : _graph.getVertexList()) {
        switch(_statements[v->_identifier]._type){
            case OP_ASSIGN:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_LOAD:
            case OP_STORE:
            case OP_LT:
            case OP_GT:
            case OP_LE:
            case OP_GE:
            case OP_EQ:
                for(auto &o : _statements[v->_identifier]._operands) {
                    if(!o._isConst && std::stoi(o._name) >= inputNum) {
                        Edge *e = new Edge();
                        e->addFromVertex(_graph.getVertexList()[std::stoi(o._name) - inputNum]);
                        e->addToVertex(v);
                        _graph.getEdgeList().push_back(e);
                        _graph.getVertexList()[std::stoi(o._name) - inputNum]->addOutEdge(e);
                        v->addInEdge(e);
                    }
                }
                break;
            case OP_PHI:
                for(int i = 0; 2 * i < _statements[v->_identifier]._operands.size(); i++) {
                    Operand &o = _statements[v->_identifier]._operands[2 * i];
                    if(!o._isConst && std::stoi(o._name) >= inputNum) {
                        Edge *e = new Edge();
                        e->addFromVertex(_graph.getVertexList()[std::stoi(o._name) - inputNum]);
                        e->addToVertex(v);
                        _graph.getEdgeList().push_back(e);
                        _graph.getVertexList()[std::stoi(o._name) - inputNum]->addOutEdge(e);
                        v->addInEdge(e);
                    }
                }
                break;
            case OP_BR:
            case OP_RET:
                break;
        }
    }
    _graph.setBlocknum(blockNum);
}

void Function::schedule() {
    std::cout << "Scheduling..." << std::endl;
    _graph.list_schedule();
}

void Function::bind() {
    std::cout << "Binding..." << std::endl;
    _graph.bind();
    for(int block = 0; block < _graph.getRegisters().size(); block++) {
        for(int reg = 0; reg < _graph.getRegisters()[block].size(); reg++) {
            for(int time = 0; time < _graph.getRegisters()[block][reg].size(); time++) {
                int statementID = _graph.getRegisters()[block][reg][time];
                if(statementID >= 0) {
                    _statements[statementID]._reg = reg;
                }
            }
        }
    }
    for(int type = 0; type < NUM_OP_TYPE; type++) {
        for(int block = 0; block < _graph.getUnits(type).size(); block++) {
            for(int unit = 0; unit < _graph.getUnits(type)[block].size(); unit++) {
                for(int time = 0; time < _graph.getUnits(type)[block][unit].size(); time++) {
                    int statementID = _graph.getUnits(type)[block][unit][time];
                    if(statementID >= 0) {
                        _statements[statementID]._opUnit = unit;
                    }
                }
            }
        }
    }
}