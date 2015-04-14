#include "GraphVolume.h"

GraphVolume::GraphVolume() {

	create();
}

GraphVolume::GraphVolume(GraphVolume&& other) :
	DiscreteVolume(other),
	_graph(other._graph),
	_positions(other._positions) {

	other._graph     = 0;
	other._positions = 0;
}

GraphVolume::GraphVolume(const GraphVolume& other) :
	DiscreteVolume(other) {

	create();
	copy(other);
}

GraphVolume&
GraphVolume::operator=(const GraphVolume& other) {

	del();
	create();
	copy(other);

	return *this;
}

GraphVolume::~GraphVolume() {

	del();
}

void
GraphVolume::create() {

	_graph     = new Graph();
	_positions = new Positions(*_graph);
}

void
GraphVolume::copy(const GraphVolume& other) {

	lemon::GraphCopy<Graph, Graph> copy(*other._graph, *_graph);

	copy.nodeMap(*_positions, *other._positions);
	copy.run();
}

void
GraphVolume::del() {

	if (_positions)
		delete _positions;
	if (_graph)
		delete _graph;

	_positions = 0;
	_graph     = 0;
}

util::box<unsigned int,3>
GraphVolume::computeDiscreteBoundingBox() const {

	util::box<unsigned int,3> bb;

	// bounding box of discrete points
	for (Graph::NodeIt node(graph()); node != lemon::INVALID; ++node)
		bb.fit(util::box<unsigned int,3>(positions()[node], positions()[node] + util::point<unsigned int,3>(1, 1, 1)));

	return bb;
}
