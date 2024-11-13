#include<iostream>
#include"graph.hpp"
#include"bucket.hpp"
#include"algorithm.hpp"
using namespace std;


int main(int argc,char *argv[]){
    string fileIn="prob1.txt";
    string fileOut="output.txt";
    size_t iterTime=10;
    switch(argc){
        case 1: break;
        case 2: iterTime=atoi(argv[1]); break;
        case 3: fileIn=string(argv[1]); fileOut=string(argv[2]); break;
        case 4: fileIn=string(argv[1]); fileOut=string(argv[2]); iterTime=atoi(argv[3]); break;
        default: break;
    }
    cout<<"================ Guidance ================"<<endl;
    cout<<"<path>/division.out"<<endl;
    cout<<"\tRunning division process with default input file \"prob1.txt\" and output file \"output.txt\", and iteration time is 10."<<endl;
    cout<<"<path>/division.out iteration_times"<<endl;
    cout<<"\tRunning division process with manually set and iteration times, and default input file \"prob1.txt\" and output file \"output.txt\""<<endl;
    cout<<"<path>/division.out input_file_path output_file_path"<<endl;
    cout<<"\tRunning division process with manually set input file and output file. The iteration time is 10."<<endl;
    cout<<"<path>/division.out input_file_path output_file_path iteration_times"<<endl;
    cout<<"\tRunning division process with manually set input file, output file and iteration time."<<endl;
    cout<<"================ Processing ================"<<endl;
    Graph* g = new Graph;
    division::graph_initialize(g, fileIn);
    division::random_graph_zone(g);
    division::division_process(g, iterTime);
    division::print_vertex(g, fileOut);
    return 0;
}
