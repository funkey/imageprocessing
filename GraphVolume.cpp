#include "GraphVolume.h"

GraphVolume::GraphVolume() {

	create();
}

GraphVolume::GraphVolume(GraphVolume&& other) :
	_graph(other._graph),
	_positions(other._positions) {

	other._graph     = 0;
	other._positions = 0;
}

GraphVolume::GraphVolume(const GraphVolume& other) {

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

util::box<float,3>
GraphVolume::computeBoundingBox() const {

	util::box<float,3> bb;

	for (Graph::EdgeIt edge(graph()); edge!= lemon::INVALID; ++edge) {

		const Position& u = positions()[graph().u(edge)];
		const Position& v = positions()[graph().v(edge)];

		bb += util::box<float,3>(
				std::min(u[0], v[0]), std::min(u[1], v[1]), std::min(u[2], v[2]),
				std::max(u[0], v[0]), std::max(u[1], v[1]), std::max(u[2], v[2]));
	}

	return bb;
}
