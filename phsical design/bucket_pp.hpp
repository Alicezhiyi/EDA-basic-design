#include <iostream>
#include <vector>
#include <list>
#include <map>
using namespace std;

class Vertex;
class Graph;
class Bucket{
    private:
        map<int,list<Vertex*>> gainMap;
        //int maxGain;
    public:
        // 构造函数
        Bucket(){};
        // 析构函数
        ~Bucket(){
            gainMap.clear();
        }
        // 用增益查找list
        bool getVlist(int gain, list<Vertex*> & vList){
            map<int, list<Vertex *>>::iterator it = gainMap.find(gain); // 在gainMap中查找int gain对应的list<Vertex*>
            if (it == gainMap.end()){
                return false; // 未找到，插入失败
            }
            vList = it->second; // it->second赋给vList
            return true;
        }
        // 插入结点V的指针，到对应的增益的list末尾
        bool insert(int seq, Graph& G){
            Vertex* V = G.getV(seq); // V[seq]的指针
            int gain = V->gain(); // 查找V[seq]的增益
            list<Vertex*> vList;
            if(!getVlist(gain, vList)){
                // 如果该增益值尚未在gainMap中创建
                list<Vertex *> vl;
                vl.push_back(V);
                gainMap[gain] = vl;
                return true;
            }
            vList.push_back(V); // 插入到链表末尾
            return true;
        }
        // 更改节点A的增益为newGain（注：此操作需在修改图之前进行）
        bool changeGain(int seq, int newGain, Graph& G){
            Vertex *V = G.getV(seq);
            int gain = V->gain();
            list<Vertex *> vList;
            if (!getVlist(gain, vList)){
                return false; // 如果V[seq]对应的增益在gainMap中不存在,return false
            }
            list<Vertex *>:: iterator itl;
            for (itl = vList.begin(); itl != vList.end(); itl++){
                // 在vList中查找V
                if (*itl == V){
                    vList.erase(itl); // 从list中删除
                    break;
                }
            }
            // 将V查到newGain对应的list中
            if (!getVlist(newGain, vList)){
                list<Vertex *> vl;
                vl.push_back(V);
                gainMap[newGain] = vl;
                return true;
            }
            vList.push_back(V);
            return true;
        }
        // 查找最大增益(待实现)
        /*
        int getMaxGain(){
            int maxGain;
            return maxGain;
        }
        */
        // 删除增益为p的链表的第1个结点
        bool remove(int gain){
            list<Vertex *> vList;
            if (!getVlist(gain, vList)){
                return false; // 未查到gain
            }
            if (vList.empty()){
                return false; // gain对应的list已空
            }
            vList.pop_front();
            return true;
        }

        void random_graph_zone(Graph &G){
            int i=0;
             for(Graph::VertexIterator iter=G->begin(G);iter!=G->end(G);iter++,i++)
                if(i<G->getVnum()/2)
                    iter->setZoneTmpStore(0);
                else 
                    iter->setZoneTmpStore(1);

        }

        void initial_bucketA(Graph &G)
        {

            for(Graph::VertexIterator iter=G->begin(G);iter!=G->end(G);iter++)
            {
                if(iter->getZone()==0)
                insert(iter->getSeq,G);
            }
        }
        void initial_bucketB(Graph &G)
        {
             for(Graph::VertexIterator iter=G->begin(G);iter!=G->end(G);iter++)
            {
                if(iter->getZone()==1)
                insert(iter->getSeq,G);
            }
        }
/*
获取节点暂存的一个增益值（可以暂存一个值 与当前情况无关）
int getGainTmpStore();
设定节点暂存的一个增益值（可以暂存一个值 与当前情况无关）
void setGainTmpStore();
获取节点暂存的一个分区编号值（可以暂存一个值 与当前情况无关）
int getZoneTmpStore();
设定节点暂存的一个分区编号值（可以暂存一个值 与当前情况无关）
void setZoneTmpStore(int);
*/
        void temp_store(){
            for(Graph::VertexIterator iter=G->begin(G);iter!=G->end(G);iter++)
            {
                iter->setGainTmpDStore(iter->getGain());
                iter->setZoneTmpStore(iter->getZone());
            }
            
        }

        void print_vertex()
        {
            std::cout<<"A ZONE :"<<endl;
            for(Graph::VertexIterator iter=G->begin(G);iter!=G->end(G);iter++)
            {
                if(iter->getZone()==0)
                {
                    std::cout<<iter->getSeq()<<"   ";

                }
            }
                std::cout<<endl;
                std::cout<<"B ZONE :"<<endl;
            for(Graph::VertexIterator iter=G->begin(G);iter!=G->end(G);iter++)
            {
                if(iter->getZone()==0)
                {
                    std::cout<<iter->getSeq()<<"   ";

                }
            }
        }
};


