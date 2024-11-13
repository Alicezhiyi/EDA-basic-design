#include "Head.h"
// 绑定（寄存器和运算单元）
bool Graph::bind()
{
    _blockMaxTime = vector<int>(_blocknum, -1);
    vector<vector<Vertex*>> vertexLists(_blocknum);
    // 统计每个块的节点，计算每个块的最大周期
    for(Vertex* v:_vertexList)
    {
        if(_blockMaxTime[v->_blockID] < v->_time)
            _blockMaxTime[v->_blockID] = v->_time;
        vertexLists[v->_blockID].push_back(v);
    }
    for(auto i=0; i<_blockMaxTime.size(); i++)
        cout<<i<<" "<<_blockMaxTime[i]<<endl;
    cout<<"block size:"<<_blockMaxTime.size()<<endl;
    // 绑定
    return (regBind(_blockMaxTime, vertexLists) && opUnitBind(_blockMaxTime, vertexLists));
}

//寄存器绑定

//需要点和边都包含指向的对象
//需要点包含周期数

//对于每一个点(算子)输出到的寄存器的信息，即数据生命周期信息
//由于每个点(算子)只有一个输出，故点和regInfo一一对应
struct regInfo
{
    Vertex* fromVertex;
    int startTime;
    int endTime;

    int regNum;

    regInfo(Vertex* f,int s,int e):fromVertex(f),startTime(s),endTime(e),regNum(-1){}
};
//寄存器绑定函数
bool Graph::regBind(const vector<int>& blockMaxTime, const vector<vector<Vertex*>>& vertexLists)
{
    //使用左边算法的数据结构
    vector<regInfo> regTable[blockMaxTime.size()];

    for(int b=0;b<blockMaxTime.size();b++)
    {
        //整个块的最大周期
        int maxTime=blockMaxTime[b];

        //最大寄存器编号
        int maxRegNum=-1;

        //初始化每个RegInfo并推入regTable
        for(int i=0;i<vertexLists[b].size();i++)
        {
            Vertex* fromVertex=vertexLists[b][i];
            if(fromVertex->_type != OP_STORE && fromVertex->_type != OP_BR && fromVertex->_type != OP_RET)
            {
                int startTime=fromVertex->_time;
                int endTime=startTime;
                for(auto e:fromVertex->_outEdge)
                {
                    if(e->_toVertex->_blockID!=b)
                    {
                        endTime=maxTime;
                        break;
                    }
                    int time=e->_toVertex->_time-1;
                    if(time>endTime)
                        endTime=time;
                }
                regTable[b].push_back(regInfo(fromVertex,startTime,endTime));
            }
        }
        
        //可命名编号数组，数组下标为寄存器编号，数组元素为占用情况
        //当数组元素为0时，代表该下标对应的编号未被占用
        vector<int> regNum(regTable[b].size(),0);

        //遍历每一个周期，按时序分配寄存器
        for(int t=0;t<=maxTime;t++)
        {
            //更新可命名编号数组
            for(int j=0;j<regTable[b].size();j++)
                if(regNum[j]>0)
                    regNum[j]-=1;

            //遍历每一个算子对应的数据生存周期
            for(int i=0;i<regTable[b].size();i++)
            {
                //获取未被占用的寄存器编号
                int num=0;
                for(;num<regTable[b].size();num++)
                    if(regNum[num]==0)
                        break;
                //如果该数据生存周期在从前周期开始，则分配一个寄存器编号
                if(regTable[b][i].startTime==t)
                {
                    if(num>maxRegNum) maxRegNum=num;
                    regTable[b][i].regNum=num;
                    regNum[num]=regTable[b][i].endTime-regTable[b][i].startTime+1;   
                }
            }
        }
        
        //初始化registers
        _registers.push_back(vector<vector<int>>());
        for(int i=0;i<=maxRegNum;i++)
        {
            _registers[b].push_back(vector<int>(maxTime+1,-1));
        }
        //将所有寄存器信息汇总
        for(int i=0;i<regTable[b].size();i++)
        {
            if(regTable[b][i].regNum==-1)  break;
            _registers[b][regTable[b][i].regNum][regTable[b][i].fromVertex->_time]=regTable[b][i].fromVertex->_identifier;
        }
    }

    int in=0;
    for(auto i:_registers)
    {
        int jn=0;
        cout<<"block "<<in++<<":"<<endl;
        for(auto j:i)
        {
            cout<<"reg"<<jn++<<":";
            for(auto k:j)
            {
                cout<<"\t"<<k;
            }
            cout<<endl;
        }
        cout<<endl;
    }
    return true;
}

// 运算单元绑定（和寄存器绑定类似）

struct opUnitInfo
{
    Vertex* fromVertex;
    int time;

    int opUnitNum;

    opUnitInfo(Vertex* f,int t):fromVertex(f),time(t),opUnitNum(-1){}
};
// 运算单元绑定函数
bool Graph::opUnitBind(const vector<int>& blockMaxTime, const vector<vector<Vertex*>>& vertexLists)
{
    for(int type = 0; type < NUM_OP_TYPE; type++) {
        //使用左边算法的数据结构
        vector<opUnitInfo> opUnitTable[blockMaxTime.size()];

        for(int b=0;b<blockMaxTime.size();b++)
        {
            //整个块的最大周期
            int maxTime=blockMaxTime[b];

            //最大寄存器编号
            int maxOpUnitNum=-1;

            //初始化每个opUnitInfo并推入opUnitTable
            for(int i=0;i<vertexLists[b].size();i++)
            {
                Vertex* fromVertex=vertexLists[b][i];
                if(type != OP_BR && type != OP_RET && fromVertex->_type == type) {
                    opUnitTable[b].push_back(opUnitInfo(fromVertex,fromVertex->_time));
                }
            }

            //可命名编号数组，数组下标为运算单元编号，数组元素为占用情况
            //当数组元素为0时，代表该下标对应的编号未被占用
            vector<int> opUnitNum(opUnitTable[b].size(),0);

            //遍历每一个周期，按时序分配运算单元
            for(int t=0;t<=maxTime;t++)
            {
                //更新可命名编号数组
                for(int j=0;j<opUnitTable[b].size();j++)
                    if(opUnitNum[j]>0)
                        opUnitNum[j]-=1;
    
                //遍历每一个算子对应的数据生存周期
                for(int i=0;i<opUnitTable[b].size();i++)
                {
                    //获取未被占用的运算单元编号
                    int num=0;
                    for(;num<opUnitTable[b].size();num++)
                        if(opUnitNum[num]==0)
                            break;
                    //如果该数据生存周期在从前周期开始，则分配一个运算单元编号
                    if(opUnitTable[b][i].time==t)
                    {
                        if(num>maxOpUnitNum) maxOpUnitNum=num;
                        opUnitTable[b][i].opUnitNum=num;
                        opUnitNum[num]=1;   
                    }
                }
            }

            //初始化units
            _units[type].push_back(vector<vector<int>>());
            for(int i=0;i<=maxOpUnitNum;i++)
            {
                _units[type][b].push_back(vector<int>(maxTime+1,-1));
            }
            //将所有运算单元信息汇总
            for(int i=0;i<opUnitTable[b].size();i++)
            {
                if(opUnitTable[b][i].opUnitNum==-1)  break;
                _units[type][b][opUnitTable[b][i].opUnitNum][opUnitTable[b][i].fromVertex->_time]=opUnitTable[b][i].fromVertex->_identifier;
            }
        }
    }
    return true;
}