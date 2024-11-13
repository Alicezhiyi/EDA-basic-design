#include<iostream>
#include"algorithm.hpp"
#include"graph.hpp"
#include"bucket.hpp"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <queue>
#include <cstdlib>
#include <ctime>
using namespace std;

int division::graph_initialize(Graph* const& g, std::string const& filename){
    int vernum;
    ifstream fin;
    fin.open(filename, ios::in);
    if (!fin.is_open())
    {
        cout << "Failed when opening the files!" << endl;
        return 1;
    }

    char buffer[256];

    int count_tmp = 0;
    int nowv=0;
    while (!fin.eof()) {
        fin.getline(buffer, 100);
        int countinV = 0;
        std::stringstream ss;
        ss << buffer;
        int p;
        while (ss >> p) {
            if (count_tmp == 0) {
                for (int i = 0; i < p; i++) {
                    Vertex* v = g->getV(i);
                }
            }
            else {
                if (countinV == 0) {
                    nowv = p;
                }
                else {
                    Edge* e= g->getE(g->getV(p), g->getV(nowv));
                }
            }
            countinV++;
        }
        count_tmp++;
    }
    fin.close();
    return 0;
};

void division::random_graph_zone(Graph *G){
    int i = 0;
    for (Graph::VertexIterator iter = G->begin(G); iter != G->end(G); iter++, i++) {
        if (i < G->getVnum()/2) {
            iter->setZoneTmpStore(0);
            iter->setZone(0);
        }
        else {
            iter->setZoneTmpStore(1);
            iter->setZone(1);
        }
    }
    //给每个点设置初始增益
    for (Graph::VertexIterator iter = G->begin(G); iter != G->end(G); iter++, i++)
        G->setGain(&*iter);
}

void division::temp_store(Graph *G){
    for(Graph::VertexIterator iter=G->begin(G);iter!=G->end(G);iter++)
    {
        iter->setGainTmpStore(iter->getGain());
        iter->setZoneTmpStore(iter->getZone());
    }
}


void division::result_store(Graph* G) {
    for (Graph::VertexIterator iter = G->begin(G); iter != G->end(G); iter++)
    {
        iter->setGain(iter->getGainTmpStore());
        iter->setZone(iter->getZoneTmpStore());
    }
}

void division::print_vertex(Graph* G, std::string const filename)
{
    ofstream fout;
    fout.open(filename);
    fout << "A ZONE :" << endl;
    for (Graph::VertexIterator iter = G->begin(G); iter != G->end(G); iter++)
    {
        if (iter->getZone() == 0)
        {
            fout << iter->getSeq() << "   ";

        }
    }
    fout << endl;
    fout << "B ZONE :" << endl;
    for (Graph::VertexIterator iter = G->begin(G); iter != G->end(G); iter++)
    {
        if (iter->getZone() == 1)
        {
            fout << iter->getSeq() << "   ";

        }
    }
    fout.close();
}

void division::division_process(Graph* const& g, size_t const iter_time){
    cout<<"Before iteration"<<": "<<division::count_cut_edge(g)<<" edges are cut."<<g->getVnum()/2<<" vertexes in each zone."<<endl;
    for (int i = 0; i < iter_time; i++) {
        Bucket A, B;

        A.initial_bucketA(g);
        B.initial_bucketB(g);
        //假设构造了两个桶，分别为A,B     这里的变量放在迭代循环内，交换循环外
        int sum = 0; //记录的增益的和
        int sumMax = 0; //历史记录中最大的增益

        //一对交换
        for (int j = 0; j < g->getVnum() / 2; j++) {
            //////////从A到B移动////////////
            //从A中选择增益最大的点移出
            int max_A = A.getMaxGain(*g); //这里的传参感觉有问题？
            sum += max_A; //更新增益
            int seq_A = 0;
            bool a=A.remove(max_A, seq_A); //从桶中删除并获得节点序号

            //改变该移动节点节点的特性
            g->getV(seq_A)->setZone(1); //这里传常数不知道有没有问题QAQ
            g->getV(seq_A)->setGain((-1) * g->getV(seq_A)->getGain()); //节点移动到另一划分后增益取反

            //遍历图中该节点连接的点，改变增益并同时对桶进行重排序
            for (Graph::EdgeOutIterator it_e = g->begin(g->getV(seq_A)); it_e != g->end(g->getV(seq_A)); ++it_e) {
                if (it_e->getVin()->getZone() == 0) { //与节点移动前同域,移动增益加2
                    A.changeGain(it_e->getVin()->getSeq(), it_e->getVin()->getGain() + 2, *g); //修改节点在桶中的排序，并不修改图中节点的增益
                    it_e->getVin()->setGain(it_e->getVin()->getGain() + 2);
                }
                else if (it_e->getVin()->getZone() == 1) { //与节点移动后同域,移动增益减2
                    B.changeGain(it_e->getVin()->getSeq(), it_e->getVin()->getGain() - 2, *g); //修改节点在桶中的排序，并不修改图中节点的增益
                    it_e->getVin()->setGain(it_e->getVin()->getGain() - 2);
                }
            }

            //////////从B到A移动////////////
            //从B中选择增益最大的点移出
            int max_B = B.getMaxGain(*g); //这里的传参感觉有问题？
            sum += max_B; //更新增益
            int seq_B = 0;
            B.remove(max_B, seq_B); //从桶中删除并获得节点序号

            //改变该移动节点节点的特性
            g->getV(seq_B)->setZone(0);
            g->getV(seq_B)->setGain((-1) * g->getV(seq_B)->getGain()); //节点移动到另一划分后增益取反

            //遍历图中该节点连接的点，改变增益并同时对桶进行重排序

            for (Graph::EdgeOutIterator it_e = g->begin(g->getV(seq_B)); it_e != g->end(g->getV(seq_B)); ++it_e) {
                if (it_e->getVin()->getZone() == 1) { //与节点移动前同域,移动增益加2
                    B.changeGain(it_e->getVin()->getSeq(), it_e->getVin()->getGain() + 2, *g); //修改节点在桶中的排序，并不修改图中节点的增益
                    it_e->getVin()->setGain(it_e->getVin()->getGain() + 2);
                }
                else if (it_e->getVin()->getZone() == 0) { //与节点移动后同域,移动增益减2
                    A.changeGain(it_e->getVin()->getSeq(), it_e->getVin()->getGain() - 2, *g); //修改节点在桶中的排序，并不修改图中节点的增益
                    it_e->getVin()->setGain(it_e->getVin()->getGain() - 2);
                }
            }
            //////////比较sum与sumMax，如果性能提升则暂存结果////////////
            if (sum > sumMax) {
                sumMax = sum; //更新sumMax
                division::temp_store(g); //暂存结果
            }

        }

        division::result_store(g); //每次迭代后存放最优结果，作为下一次迭代的初始
        // cout << "SUMMAX" << sumMax<<endl;
        cout<<"Iteration "<<(i+1)<<": "<<division::count_cut_edge(g)<<" edges are cut. "<<g->getVnum()/2<<" vertexes in each zone."<<endl;

        // print_vertex(g, "output.txt"); //打印结果
    }
};

size_t division::count_cut_edge(Graph* const& g){
    size_t a=0;
    for(Graph::EdgeIterator iter=g->begin(*g); iter!=g->end(*g); ++iter){
        if(iter->getVin()->getZone()!=iter->getVout()->getZone())
            ++a;
    }
    return a;
}
