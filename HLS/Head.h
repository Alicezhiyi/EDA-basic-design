#ifndef HEAD_H
#define HEAD_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <queue>

#include "parser.h"

#define RAM_WIDTH 4
#define NUM_OP_TYPE 15

using namespace std;

struct Edge;

struct Vertex
{
public:
    Vertex(int type = -1, int identifier = -1, int time = -1) : _type(type), _identifier(identifier), _time(time) {}
    void addInEdge(Edge* edge) { _inEdge.push_back(edge); }
    void addOutEdge(Edge* edge) { _outEdge.push_back(edge); }
public:
    int _type;               // 操作类型
    int _identifier;         // 点的编号
    int _time;               // 点所处周期数
    int _visited = 0;        // 是否被访问过
    int _indegree = 0;       // 块内入度
    int _outdegree = 0;      // 块内出度
    int _blockID = 0;        // 属于子块的编号
    vector<Edge*> _inEdge;   // 入边集合
    vector<Edge*> _outEdge;  // 出边集合
};

struct Edge
{
public:
    Edge() {}
    void addFromVertex(Vertex* f) { _fromVertex = f; }
    void addToVertex(Vertex* t) { _toVertex = t; }
public:
    Vertex* _fromVertex;     // 起始点
    Vertex* _toVertex;       // 终止点
};

class Graph
{
public:
    Graph() {}
    bool bind();                // 绑定（寄存器和运算单元）
    void list_schedule();       // 调度
    void display() {            // 输出图的信息
        for (Vertex* v : _vertexList) {
            cout << v->_identifier << " to ";
            for (Edge* e : v->_outEdge) {
                cout << e->_toVertex->_identifier << " ";
            }
            cout << endl;
            //cout << "is visited "<< v->_visited << endl;
            cout << "indegree is " << v->_indegree << endl;
            cout << "blocknum is " << v->_blockID << endl;
            cout << "time is " << v->_time << endl;
        }
    }
    void cal_indegree() {       // 计算入度
        for (Vertex* v : _vertexList) {
            v->_indegree = 0;
            for (Edge* e : v->_inEdge) {
                Vertex* startvertext = e->_fromVertex;
                if (startvertext != nullptr && v->_blockID == startvertext->_blockID)
                    v->_indegree++;
            }
        }
    }
    int getBlocknum() const {return _blocknum;}                                         // 图内分块的数量
    void setBlocknum(int blocknum) {_blocknum = blocknum;}                              // 图内分块的数量
    const vector<Vertex*>& getVertexList() const {return _vertexList;}                  // 图中所有点
    vector<Vertex*>& getVertexList() {return _vertexList;}                              // 图中所有点
    const vector<Edge*>& getEdgeList() const {return _edgeList;}                        // 图中所有边
    vector<Edge*>& getEdgeList() {return _edgeList;}                                    // 图中所有边
    const vector<vector<vector<int>>>& getRegisters() const {return _registers;}        // 共享寄存器的连接信息,getRegisters()[块编号][寄存器编号][周期数]=输入算子编号
    vector<vector<vector<int>>>& getRegisters() {return _registers;}                    // 共享寄存器的连接信息,getRegisters()[块编号][寄存器编号][周期数]=输入算子编号
    const vector<vector<vector<int>>>& getUnits(int type) const {return _units[type];}  // 共享计算单元的连接信息，getUnits(计算类型)[块编号][计算单元编号][周期数]=算子编号
    vector<vector<vector<int>>>& getUnits(int type) {return _units[type];}              // 共享计算单元的连接信息，getUnits(计算类型)[块编号][计算单元编号][周期数]=算子编号
    const vector<int>& getBlockMaxTime() const {return _blockMaxTime;}                  // 基本块执行时间的最大值（时间从0开始。若总周期数为1，则时间最大值为0；若基本块内无计算单元，则时间最大值为-1）
    vector<int>& getBlockMaxTime() {return _blockMaxTime;}                              // 基本块执行时间的最大值（时间从0开始。若总周期数为1，则时间最大值为0；若基本块内无计算单元，则时间最大值为-1）
private:
    bool regBind(const vector<int>& blockMaxTime, const vector<vector<Vertex*>>& vertexLists);      // 寄存器绑定函数
    bool opUnitBind(const vector<int>& blockMaxTime, const vector<vector<Vertex*>>& vertexLists);   // 运算单元绑定函数
private:
    int _blocknum = 0;               // 图内分块的数量
    vector<Vertex*> _vertexList;     // 图中所有点
    vector<Edge*> _edgeList;         // 图中所有边
    vector<vector<vector<int>>> _registers;          // 共享寄存器的连接信息,registers[块编号][寄存器编号][周期数]=输入算子编号
    vector<vector<vector<int>>> _units[NUM_OP_TYPE]; // 共享计算单元的连接信息，units[计算类型][块编号][计算单元编号][周期数]=算子编号
    vector<int> _blockMaxTime;       // 基本块执行时间的最大值（时间从0开始。若总周期数为1，则时间最大值为0；若基本块内无计算单元，则时间最大值为-1）
};

// 操作数
struct Operand {
    Operand(const string& name = "", bool isConst = false) : _name(name), _isConst(isConst) {}

    string _name;   // 操作数的名称（可以是常数、block的ID、Statement的ID，以字符串形式存储）
    bool _isConst;  // 是否是常数
};
// 语句
struct Statement {
    Statement(const string& name = "", int blockID = -1, bool isArray = false, int type = -1, int reg = -1, int opUnit = -1) : _name(name), _blockID(blockID), _isArray(isArray), _type(type), _reg(reg), _opUnit(opUnit) {}

    string _name;       // 变量名
    int _blockID;       // 基本块的序号（-1表示输入变量）
    int _type;          // 计算类型
    vector<Operand> _operands; // 操作数
    bool _isArray;      // 是否为数组
    int _reg;           // 绑定的寄存器
    int _opUnit;        // 绑定的计算单元
};
// 基本块结束的信息（返回或分支）
struct BlockEnd{
    bool _isReturn;                  // 是否返回（若为false，不返回，则该基本块的结束为分支）
    Operand _returnValue;            // 返回值
    int _branchCond;                 // 分支条件（若小于0，则为无条件）
    int _branchTrue, _branchFalse;    // 条件为真、条件为假的分支（无条件：跳转到条件为真的块）
};
// 函数
class Function {
public:
    Function(const string& fileName);
    ~Function() {}
    void schedule();    // 调度
    void bind();        // 绑定
    const string& getName() const {return _name;}                           // 函数名
    int getRetType() const {return _retType;}                               // 返回类型
    const vector<BlockEnd>& getBlockEnd() const {return _blockEnd;}         // 基本块结束的信息（返回或分支）
    const vector<Statement>& getStatements() const {return _statements;}    // 函数的所有语句（包括入口参数）
    const Graph& getGraph() const {return _graph;}                          // 数据流图
private:
    string _name;       // 函数名
    int _retType;       // 返回类型
    vector<BlockEnd> _blockEnd;    // 基本块结束的信息（返回或分支）
    vector<Statement> _statements; // 函数的所有语句（包括入口参数）
    Graph _graph;       // 数据流图
};

// RTL生成
namespace RTL {
    // 端口类型
    enum PORT_TYPE {
        PORT_INPUT,
        PORT_OUTPUT
    };
    // 变量类型
    enum VAR_TYPE {
        VAR_WIRE,
        VAR_REG
    };
    // 模块
    class Module {
        public:
            Module(const Function& function);
            void generateRTL();         // 生成RTL代码并写入文件
        private:
            void moduleDeclaration();   // 模块声明
            void portsDeclaration();    // 端口声明
            void varsDeclararion();     // 变量声明
            void stateMachine();        // 状态机生成
            void moduleIO();            // 根据函数入口参数，处理模块的输入输出（数组：输出地址等信号，非数组：将输入信号寄存）
            void moduleReturn();        // 模块返回值
            void blockCount(int blockID);                   // 基本块计时器
            void blockReadWrite(int blockID);               // 基本块的IO读写
            void opUnit(int blockID, int type, int unitID); // 运算单元
            void blockReg(int blockID, int regID);          // 寄存器
            void moduleEnd();           // 模块结束
        private:
            const Function& _function;  // 模块对应的函数
            string _moduleRTL;          // 模块的RTL代码
            vector<int> _input;         // 函数的输入
    };
    // 块内计时器位宽
    int blockCountWidth(int cycle);
    // 端口（例："input [31:0] a"）
    string port(int portType, int width, const string& name);
    // 变量（例："wire [31:0] a"）
    string var(int varType, int width, const string& name);
    // 语句对应寄存器的RTL变量名（例：基本块0 寄存器1 对应变量名"bb0_reg1"）
    string reg(const Function& f, int statementID);
    // 操作数对应寄存器的RTL变量名（例：操作数为基本块1的寄存器2，对应变量名"bb1_reg2"；操作数为常数10，对应"10"）
    string operandReg(const Function &f, const Operand& a);
    // 运算单元名（例：基本块0 加法器1 对应运算单元名"bb0_add1"）
    string opName(const Function& f, int statementID);
    // 运算单元名（例：基本块0 加法器1 对应运算单元名"bb0_add1"）
    string opName(int blockID, int type, int unitID);
    // 运算符号（例：乘法为"*"，加法为"+"）
    string operatorStr(int type);
    // 状态（例："5'b00001"）
    string state(int blockNum, int blockEnable, bool idle = false, bool complete = false);
}

#endif
