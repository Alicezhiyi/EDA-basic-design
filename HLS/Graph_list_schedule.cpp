#include "Head.h"

void Graph::list_schedule() {
	cal_indegree();								        // calculate the indegree within basic blocks for each vertext
	queue<Vertex*> q0;									// helper queue for ASAP
	for (int i = 0; i < _blocknum; i++) {
		for (Vertex* v : _vertexList) {
			if (v->_blockID == i) {
				if (v->_indegree == 0) {
					v->_time = 0;
					q0.push(v);
					
				}
			}
		}
		while (!q0.empty()) {
			Vertex* u = q0.front();
			q0.pop();
			u->_visited = 1;
			for (Edge* e : u->_outEdge) {
				Vertex* w = e->_toVertex;
				if (w->_blockID == i) {                                 //if w is in the same block, inblock_indegree minus 1
					w->_indegree--;
					if (w->_indegree == 0 && w->_visited == 0) {
						q0.push(w);
						w->_time = u->_time + 1;
					}
				}
			}
		}
	}
	//test
	for (int i = 0; i < _blocknum; i++) {
		cout << "this is block " << i << endl;
		for (Vertex* v : _vertexList) {
			if (v->_blockID == i) {
				cout << v->_identifier << " is scheduled in " << v->_time << " cycle within blocks" << endl;
			}
		}
	}
	}
