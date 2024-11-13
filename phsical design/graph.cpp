#include<iostream>
#include"graph.hpp"
using namespace std;

Graph::Graph():
    _vNumPerList(128),
    _eNumPerList(128),
    _vListNum(-1),
    _eListNum(-1),
    _vLastListCur(_vNumPerList),
    _eLastListCur(_eNumPerList),
    _vList(std::vector<Vertex*>()),
    _eList(std::vector<Edge*>())
{};
Graph::~Graph(){
    for(std::vector<Vertex*>::iterator iter=_vList.begin(); iter!=_vList.end(); ++iter){
        delete [] *iter;
        *iter=NULL;
    }
    for(std::vector<Edge*>::iterator iter=_eList.begin(); iter!=_eList.end(); ++iter){
        delete [] *iter;
        *iter=NULL;
    }
};
Vertex* Graph::newV(){
    if(_vLastListCur==_vNumPerList){
        Vertex* v=new Vertex[_vNumPerList];
        _vList.push_back(v);
        _vLastListCur=0;
        ++_vListNum;
    }
    Vertex* vp=_vList[_vListNum]+_vLastListCur;
    vp->setSeq(_vListNum*_vNumPerList+_vLastListCur);
    vp->setEinNum(0);
    vp->setEoutNum(0);
    ++_vLastListCur;
    return vp;
};
Edge* Graph::newE(Vertex* const& vIn, Vertex* const& vOut){
    if(_eLastListCur==_eNumPerList){
        Edge* e=new Edge[_eNumPerList];
        _eList.push_back(e);
        _eLastListCur=0;
        ++_eListNum;
    }
    Edge* ep=_eList[_eListNum]+_eLastListCur;
    ep->setSeq(_eListNum*_eNumPerList+_eLastListCur);
    ++_eLastListCur;
    ep->setVin(vIn);
    ep->setVout(vOut);
    return ep;
};
Vertex* Graph::getV(int const i){
    size_t vListNum=i/_vNumPerList;
    size_t vLastListCur=i%_vNumPerList;
    if((int)vListNum<(int)_vListNum || vListNum==_vListNum && vLastListCur<_vLastListCur)
        return (_vList[vListNum]+vLastListCur);
    else
        return newV();
};
Edge* Graph::getE(Vertex* const& vIn, Vertex* const& vOut){
    Edge* ep;
    Graph::EdgeInIterator iterEin=begin(*vIn);
    Graph::EdgeInIterator iterEinEnd=end(*vIn);
    for(; iterEin!=iterEinEnd && iterEin->getEinNxt() && iterEin->getEinNxt()->getVout()->getSeq()<vOut->getSeq(); ++iterEin);
    if(&*iterEin==NULL){
        ep=newE(vIn, vOut);
        vIn->setEin(ep);
        vIn->setEinNum(1);
    }
    else if(iterEin->getEinNxt() && iterEin->getEinNxt()->getVout()->getSeq()==vOut->getSeq())
        return iterEin->getEinNxt();
    else{
        ep=newE(vIn, vOut);
        ep->setEinNxt(iterEin->getEinNxt());
        iterEin->setEinNxt(ep);
        vIn->setEinNum(vIn->getEinNum()+1);
    }
    Graph::EdgeOutIterator iterEout=begin(vOut);
    Graph::EdgeOutIterator iterEoutEnd=end(vOut);
    for(; iterEout!=iterEoutEnd && iterEout->getEoutNxt() && iterEout->getEoutNxt()->getVin()->getSeq()<vIn->getSeq(); ++iterEout);
    if(&*iterEout==NULL){
        vOut->setEout(ep);
        vOut->setEoutNum(1);
        return ep;
    }
    else{
        ep->setEoutNxt(iterEout->getEoutNxt());
        iterEout->setEoutNxt(ep);
        vOut->setEoutNum(vOut->getEoutNum()+1);
        return ep;
    }
};
void Graph::rmV(Vertex* const& v) const{
    while(v->getEin()!=NULL)
        rmE(v->getEin());
    while(v->getEout()!=NULL)
        rmE(v->getEout());
    v->setEinNum(0);
    v->setEoutNum(0);
};
void Graph::rmE(Edge* const& e) const{
    Graph::EdgeInIterator iterEin=begin(*(e->getVin()));
    Graph::EdgeOutIterator iterEout=begin(e->getVout());
    Graph::EdgeInIterator iterEinEnd=end(*(e->getVin()));
    Graph::EdgeOutIterator iterEoutEnd=end(e->getVout());
    for(; iterEin!=iterEinEnd && iterEin->getEinNxt() && iterEin->getEinNxt()==e; ++iterEin);
    if(iterEin==iterEinEnd || iterEin->getEinNxt()==NULL)
        return;
    else
        iterEin->setEinNxt(e->getEinNxt());
    for(; iterEout!=iterEoutEnd && iterEout->getEoutNxt() && iterEout->getEoutNxt()==e; ++iterEout);
    if(iterEout==iterEoutEnd || iterEout->getEoutNxt()==NULL)
        return;
    else
        iterEout->setEoutNxt(e->getEoutNxt());
    e->getVin()->setEinNum(e->getVin()->getEinNum()-1);
    e->getVout()->setEoutNum(e->getVout()->getEoutNum()-1);
    e->setVin(NULL);
    e->setVout(NULL);
    e->setEinNxt(NULL);
    e->setEoutNxt(NULL);
};
int Graph::setGain(Vertex* const& v) const{
    int zone=v->getZone();
    int gain=0;
    for(Graph::EdgeOutIterator iter=begin(v); iter!=end(v); ++iter){
        if(iter->getVin()!=v && iter->getVin()!=NULL){
            if(iter->getVin()->getZone()==zone)
                --gain;
            else
                ++gain;
        }
    }
    v->setGain(gain);
    return gain;
};
bool Graph::checkGain(Vertex* const& v) const{
    int zone=v->getZone();
    int gain=0;
    for(Graph::EdgeOutIterator iter=begin(v); iter!=end(v); ++iter){
        if(iter->getVin()!=v && iter->getVin()!=NULL){
            if(iter->getVin()->getZone()==zone)
                --gain;
            else
                ++gain;
        }
    }
    return (gain==v->getGain()) ? true : false;
};
int Graph::getVnum() const{
    return _vNumPerList*_vListNum+_vLastListCur;
};
int Graph::getEnum() const{
    return _eNumPerList*_eListNum+_eLastListCur;
}

Graph::VertexIterator Graph::begin(Graph* const& g) const{return Graph::VertexIterator(g, 0, 0);};
Graph::VertexIterator Graph::end(Graph* const& g) const{
    return Graph::VertexIterator(g,
        (_vLastListCur==_vNumPerList) ? (_vListNum+1) : _vListNum,
        (_vLastListCur==_vNumPerList) ? 0 : (_vLastListCur));
};
Graph::EdgeIterator Graph::begin(Graph& g) const{return Graph::EdgeIterator(&g, 0, 0);};
Graph::EdgeIterator Graph::end(Graph& g) const{
    return Graph::EdgeIterator(&g,
        (_eLastListCur==_eNumPerList) ? (_eListNum+1) : _eListNum,
        (_eLastListCur==_eNumPerList) ? 0 : (_eLastListCur));
};
Graph::EdgeInIterator Graph::begin(Vertex const& v) const{return Graph::EdgeInIterator(v.getEin());};
Graph::EdgeInIterator Graph::end(Vertex const& v) const{return Graph::EdgeInIterator(NULL);};
Graph::EdgeOutIterator Graph::begin(Vertex* const& v) const{return Graph::EdgeOutIterator(v->getEout());};
Graph::EdgeOutIterator Graph::end(Vertex* const& v) const{return Graph::EdgeOutIterator(NULL);};

Edge::Edge(Vertex* const& vIn, Vertex* const& vOut):
    _seq(0), _vIn(vIn), _vOut(vOut), _eInNxt(NULL), _eOutNxt(NULL){};
Edge::Edge():
    _seq(0), _vIn(NULL), _vOut(NULL), _eInNxt(NULL), _eOutNxt(NULL){};
Edge::~Edge(){};
size_t Edge::getSeq() const{return _seq;};
Vertex* const Edge::getVin() const{return _vIn;};
Vertex* const Edge::getVout() const{return _vOut;};
Edge* const Edge::getEinNxt() const{return _eInNxt;};
Edge* const Edge::getEoutNxt() const{return _eOutNxt;};
void Edge::setSeq(size_t const& seq){_seq=seq;};
void Edge::setVin(Vertex* const& vIn){_vIn=vIn;};
void Edge::setVout(Vertex* const& vOut){_vOut=vOut;};
void Edge::setEinNxt(Edge* const& eInNxt){_eInNxt=eInNxt;};
void Edge::setEoutNxt(Edge* const& eOutNxt){_eOutNxt=eOutNxt;};

Vertex::Vertex():
    _gain(0), _zone(0), _eInNum(0), _eOutNum(0), _eIn(NULL), _eOut(NULL){};
Vertex::~Vertex(){};
int Vertex::getGain() const{return _gain;};
int Vertex::getZone() const{return _zone;};
int Vertex::getGainTmpStore() const{return _gainTmpStore;};
int Vertex::getZoneTmpStore() const{return _zoneTmpStore;};
size_t Vertex::getSeq() const{return _seq;};
size_t Vertex::getEinNum() const{return _eInNum;};
size_t Vertex::getEoutNum() const{return _eOutNum;};
Edge* const Vertex::getEin() const{return _eIn;};
Edge* const Vertex::getEout() const{return _eOut;};
void Vertex::setGain(int const gain){_gain=gain;};
void Vertex::setZone(int const zone){_zone=zone;};
void Vertex::setGainTmpStore(int const gain){_gainTmpStore=gain;};
void Vertex::setZoneTmpStore(int const zone){_zoneTmpStore=zone;};
void Vertex::setSeq(int const seq){_seq=seq;};
void Vertex::setEinNum(size_t const& eInNum){_eInNum=eInNum;};
void Vertex::setEoutNum(size_t const& eOutNum){_eOutNum=eOutNum;};
void Vertex::setEin(Edge* const& eIn){_eIn=eIn;};
void Vertex::setEout(Edge* const& eOut){_eOut=eOut;};

Graph::VertexIterator::VertexIterator(Graph* const& g, size_t const& vListCur, size_t const& vLastListCur):
    _g(g), _vListCur(vListCur), _vLastListCur(vLastListCur){};
Graph::VertexIterator::~VertexIterator(){};
Vertex* const Graph::VertexIterator::operator->() const{return (_g->_vList)[_vListCur]+_vLastListCur;};
Vertex& Graph::VertexIterator::operator*() const{return (_g->_vList)[_vListCur][_vLastListCur];};
Graph::VertexIterator& Graph::VertexIterator::operator++(){
    _vListCur=(_vLastListCur==(_g->_vNumPerList-1)) ? (_vListCur+1) : _vListCur;
    _vLastListCur=(_vLastListCur==(_g->_vNumPerList-1)) ? 0 : (_vLastListCur+1);
    return *this;
};
Graph::VertexIterator Graph::VertexIterator::operator++(int const){
    _vListCur=(_vLastListCur==(_g->_vNumPerList-1)) ? (_vListCur+1) : _vListCur;
    _vLastListCur=(_vLastListCur==(_g->_vNumPerList-1)) ? 0 : (_vLastListCur+1);
    return *this;
}
Graph::VertexIterator Graph::VertexIterator::operator=(VertexIterator const& v){
    return Graph::VertexIterator(v._g, v._vListCur, v._vLastListCur);
};
bool Graph::VertexIterator::operator==(VertexIterator const& v){
    return (_vListCur==v._vListCur)&&(_vLastListCur==v._vLastListCur)&&(_g==v._g);
};
bool Graph::VertexIterator::operator!=(VertexIterator const& v){
    return (_vListCur!=v._vListCur)||(_vLastListCur!=v._vLastListCur)||(_g!=v._g);
};

Graph::EdgeIterator::EdgeIterator(Graph* const& g, size_t const& eListCur, size_t const& eLastListCur):
    _g(g), _eListCur(eListCur), _eLastListCur(eLastListCur){};
Graph::EdgeIterator::~EdgeIterator(){};
Edge* const Graph::EdgeIterator::operator->() const{return (_g->_eList)[_eListCur]+_eLastListCur;};
Edge& Graph::EdgeIterator::operator*() const{return (_g->_eList)[_eListCur][_eLastListCur];};
Graph::EdgeIterator& Graph::EdgeIterator::operator++(){
    _eListCur=(_eLastListCur==(_g->_eNumPerList-1)) ? (_eListCur+1) : _eListCur;
    _eLastListCur=(_eLastListCur==(_g->_eNumPerList-1)) ? 0 : (_eLastListCur+1);
    return *this;
};
Graph::EdgeIterator Graph::EdgeIterator::operator++(int const){
    _eListCur=(_eLastListCur==(_g->_eNumPerList-1)) ? (_eListCur+1) : _eListCur;
    _eLastListCur=(_eLastListCur==(_g->_eNumPerList-1)) ? 0 : (_eLastListCur+1);
    return *this;
};
Graph::EdgeIterator Graph::EdgeIterator::operator=(EdgeIterator const& e){
    return Graph::EdgeIterator(e._g, e._eListCur, e._eLastListCur);
};
bool Graph::EdgeIterator::operator==(EdgeIterator const& e){
    return (_eListCur==e._eListCur)&&(_eLastListCur==e._eLastListCur)&&(_g==e._g);
};
bool Graph::EdgeIterator::operator!=(EdgeIterator const& e){
    return (_eListCur!=e._eListCur)||(_eLastListCur!=e._eLastListCur)||(_g!=e._g);
};

Graph::EdgeInIterator::EdgeInIterator(Edge* const& e): _cur(e){};
Graph::EdgeInIterator::~EdgeInIterator(){};
Edge* const Graph::EdgeInIterator::operator->() const{return _cur;};
Edge& Graph::EdgeInIterator::operator*() const{return *_cur;};
Graph::EdgeInIterator& Graph::EdgeInIterator::operator++(){_cur=_cur->getEinNxt(); return *this;};
Graph::EdgeInIterator Graph::EdgeInIterator::operator++(int const){_cur=_cur->getEinNxt(); return *this;};
bool Graph::EdgeInIterator::operator==(EdgeInIterator const& e){return _cur==e._cur;};
bool Graph::EdgeInIterator::operator!=(EdgeInIterator const& e){return _cur!=e._cur;};

Graph::EdgeOutIterator::EdgeOutIterator(Edge* const& e): _cur(e){};
Graph::EdgeOutIterator::~EdgeOutIterator(){};
Edge* const Graph::EdgeOutIterator::operator->() const{return _cur;};
Edge& Graph::EdgeOutIterator::operator*() const{return *_cur;};
Graph::EdgeOutIterator& Graph::EdgeOutIterator::operator++(){_cur=_cur->getEoutNxt(); return *this;};
Graph::EdgeOutIterator Graph::EdgeOutIterator::operator++(int const){_cur=_cur->getEoutNxt(); return *this;};
bool Graph::EdgeOutIterator::operator==(EdgeOutIterator const& e){return _cur==e._cur;};
bool Graph::EdgeOutIterator::operator!=(EdgeOutIterator const& e){return _cur!=e._cur;};

// int main(){
//     Graph* g=new Graph;
//     int i=0;
//     for(int i=0; i<400; ++i){
//         Vertex* v=g->getV(i);
//     }
//     for(Graph::VertexIterator iter=g->begin(g); iter!=g->end(g); ++iter){
//         for(Graph::VertexIterator iter2=g->begin(g); iter2!=g->end(g); ++iter2){
//             Edge* e=g->getE(&*iter, &*iter2);
//         }
//     }
//     // for(Graph::VertexIterator iter=g->begin(g); iter!=g->end(g); ++iter){
//     //     for(Graph::EdgeInIterator iter2=g->begin(*iter); iter2!=g->end(*iter); ++iter2){
//     //         cout<<iter2->getVout()->getSeq()<<" ";
//     //     }
//     //     cout<<endl;
//     // }
//     for(Graph::EdgeIterator iter=g->begin(*g); iter!=g->end(*g); ++iter){
//         cout<<iter->getVout()->getSeq()<<"->"<<iter->getVin()->getSeq()<<endl;
//     }
//     delete g;
//     return 0;
// }
