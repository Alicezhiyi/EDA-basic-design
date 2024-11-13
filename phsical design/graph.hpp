#ifndef GRAPH_HPP
#define GRAPH_HPP
#include<iostream>
#include<vector>

class Graph;
class Edge;
class Vertex;

// This is top-graph
// using crossing-link-table to store edges
// Vertexes and edges stored in multi-array
class Graph{
    public:
        Graph();
        ~Graph();
        // Access vertex by index
        // If vertexes with index exist, return the pointer of vertex directly
        // If vertexes with index don't exist, create new vertex, but its index is <cur_max_index+1>
        Vertex* getV(int const);
        // Access edge by vertex-in and vertex-out
        // If edges with the vertexes exist, return the pointer of edge directly
        // If edges with the vertexes don't exist, create new edge
        Edge* getE(Vertex* const&, Vertex* const&);
        // Remove this vertex and all edges linked
        // Don't free up space, so it would be accessed while traversing
        // getV() still return this vertex
        void rmV(Vertex* const&) const;
        // Remove this edge from crossing-link-table
        // Don't free up space, so it would be accessed while traversing
        void rmE(Edge* const&) const;
        // Modify the vertex-in and vertex-out
        void mvE(Edge* const&, Vertex* const&, Vertex* const&) const;
        // Calculate and set the gain of vertex
        int setGain(Vertex* const&) const;
        bool checkGain(Vertex* const&) const;
        // Get the total number of vertexes
        int getVnum() const;
        // Get the total number of edges
        int getEnum() const;
        // Iterator for all vertexes, their index is increasing
        class VertexIterator;
        // Iterator for all edges, but not BFS or DFS
        class EdgeIterator;
        // Iterator for all edges into certain vertex
        class EdgeInIterator;
        // Iterator for all edges out from certain vertex
        class EdgeOutIterator;
        // Get the begin position of iterator, the in-argument type decide iterator type
        VertexIterator begin(Graph* const&) const;
        EdgeIterator begin(Graph&) const;
        EdgeInIterator begin(Vertex const&) const;
        EdgeOutIterator begin(Vertex* const&) const;
        // Get the ending position of iterator, the out-argument type decide iterator type
        VertexIterator end(Graph* const&) const;
        EdgeIterator end(Graph&) const;
        EdgeInIterator end(Vertex const&) const;
        EdgeOutIterator end(Vertex* const&) const;
        // Usage of Iterator:
        // for-cycle:
        // for(Graph::xxxIterator iter=g->begin(...); iter!=g->end(...); ++iter)
        // while-cycle:
        // Graph::xxxIterator iter=g->begin(...);
        // while(iter!=g->end(...)){...; ...; ...; ++iter;}
        // Iterator is strictly equivalent to pointer of vertex or edge!
        // iter->... is strictly equivalent to vertex_ptr->..., meaning a member of vertex
        // (*iter) is strictly equivalent to (*vertex_ptr), meaning a vertex object
        // ++iter and iter++ means iterator moving to the next object
        // iter==... and iter!=... means the comparation of two iterators, but cannot use <(less than) and >(more than)
        // iterator can be assigned by each other by =, but when definiting you should "iter=g->begin()"
    private:
        int const _eNumPerList;
        int const _vNumPerList;
        std::vector<Vertex*> _vList;
        std::vector<Edge*> _eList;
        int _vListNum;
        int _vLastListCur;
        int _eListNum;
        int _eLastListCur;
        Vertex* newV();
        Edge* newE(Vertex* const&, Vertex* const&);
};

class Edge{
    public:
        // Initialize edge object, using in-vertex and out-vertex
        Edge(Vertex* const&, Vertex* const&);
        // Initialize edge object, in-vertex and out-vertex are NULL
        Edge();
        ~Edge();
        // Get the index of edge, I feel it is useless and plan to delete it
        size_t getSeq() const;
        // Get the in-vertex of edge
        Vertex* const getVin() const;
        // Get the out-vertex of edge
        Vertex* const getVout() const;
        // Get the next edge with the same in-vertex; the out-vertex index of the next edge must be greater
        Edge* const getEinNxt() const;
        // Get the next edge with the same out-vertex; the in-vertex index of the next edge must be greater
        Edge* const getEoutNxt() const;
        // The methods below aren't supposed to use independently
        // Suggest to use methods of Graph to modify the datas
        void setSeq(size_t const&);
        void setVin(Vertex* const&);
        void setVout(Vertex* const&);
        void setEinNxt(Edge* const&);
        void setEoutNxt(Edge* const&);
    private:
        size_t _seq;
        Vertex* _vIn;
        Vertex* _vOut;
        Edge* _eInNxt;
        Edge* _eOutNxt;
};

class Vertex{
    public:
        // Initialize the vertex object
        Vertex();
        ~Vertex();
        // Get the gain
        int getGain() const;
        // Get the gain buffer data
        int getGainTmpStore() const;
        // Get the zone number the vertex belonging to
        int getZone() const;
        // Get the zone buffer data
        int getZoneTmpStore() const;
        // Get the index
        size_t getSeq() const;
        // Get the indegree
        size_t getEinNum() const;
        // Get the outdegree
        size_t getEoutNum() const;
        // Get the first edge into it
        Edge* const getEin() const;
        // Get the first edge out from it
        Edge* const getEout() const;
        // Set the zone vertex belonging to
        void setZone(int const);
        // Set the zone buffer data
        void setZoneTmpStore(int const);
        // Set the gain buffer data
        void setGainTmpStore(int const);
        // The methods below aren't supposed to use independently
        // Suggest to use methods of Graph to modify the datas
        // Set the gain
        void setGain(int const);
        void setSeq(int const);
        void setEinNum(size_t const&);
        void setEoutNum(size_t const&);
        void setEin(Edge* const&);
        void setEout(Edge* const&);
    private:
        int _gain;
        int _zone;
        int _gainTmpStore;
        int _zoneTmpStore;
        size_t _seq;
        size_t _eInNum;
        size_t _eOutNum;
        Edge* _eIn;
        Edge* _eOut;
};

class Graph::VertexIterator{
    public:
        VertexIterator(Graph* const&, size_t const&, size_t const&);
        ~VertexIterator();
        Vertex* const operator->() const;
        Vertex& operator*() const;
        VertexIterator& operator++();
        VertexIterator operator++(int const);
        VertexIterator operator=(VertexIterator const&);
        bool operator==(VertexIterator const&);
        bool operator!=(VertexIterator const&);
    private:
        Graph* _g;
        size_t _vListCur;
        size_t _vLastListCur;
};

class Graph::EdgeInIterator{
    public:
        EdgeInIterator(Edge* const&);
        ~EdgeInIterator();
        Edge* const operator->() const;
        Edge& operator*() const;
        EdgeInIterator& operator++();
        EdgeInIterator operator++(int const);
        EdgeInIterator operator=(EdgeInIterator const&);
        bool operator==(EdgeInIterator const&);
        bool operator!=(EdgeInIterator const&);
    private:
        Edge* _cur;
};

class Graph::EdgeOutIterator{
    public:
        EdgeOutIterator(Edge* const&);
        ~EdgeOutIterator();
        Edge* const operator->() const;
        Edge& operator*() const;
        EdgeOutIterator& operator++();
        EdgeOutIterator operator++(int const);
        EdgeOutIterator operator=(EdgeOutIterator const&);
        bool operator==(EdgeOutIterator const&);
        bool operator!=(EdgeOutIterator const&);
    private:
        Edge* _cur;
};

class Graph::EdgeIterator{
    public:
        EdgeIterator(Graph* const&, size_t const&, size_t const&);
        ~EdgeIterator();
        Edge* const operator->() const;
        Edge& operator*() const;
        EdgeIterator& operator++();
        EdgeIterator operator++(int);
        EdgeIterator operator=(EdgeIterator const&);
        bool operator==(EdgeIterator const&);
        bool operator!=(EdgeIterator const&);
    private:
        Graph* _g;
        size_t _eListCur;
        size_t _eLastListCur;
};
#endif
