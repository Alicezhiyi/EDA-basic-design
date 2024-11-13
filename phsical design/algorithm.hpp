#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP
#include<iostream>
#include"graph.hpp"
#include<string.h>

namespace division{
    int graph_initialize(Graph* const&, std::string const& filename);
    void random_graph_zone(Graph*);
    void temp_store(Graph*);
    void result_store(Graph*);
    void print_vertex(Graph*, std::string const);
    size_t count_cut_edge(Graph* const&);
    void division_process(Graph* const&, size_t const);
};
#endif
