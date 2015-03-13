#include <util/assert.h>
#include "Skeleton.h"

Skeleton::Skeleton() {

	createGraph();
}

Skeleton::Skeleton(Skeleton&& other) :
	_graph(other._graph),
	_positions(other._positions),
	_segments(other._segments) {

	other._graph     = 0;
	other._positions = 0;
	other._segments  = 0;
}

Skeleton::Skeleton(const Skeleton& other) {

	createGraph();
	copyGraph(other);
}

Skeleton&
Skeleton::operator=(const Skeleton& other) {

	deleteGraph();
	createGraph();
	copyGraph(other);

	return *this;
}

Skeleton::~Skeleton() {

	deleteGraph();
}

void
Skeleton::createGraph() {

	_graph     = new Graph();
	_positions = new Positions(*_graph);
	_segments  = new Segments(*_graph);
}

void
Skeleton::copyGraph(const Skeleton& other) {

	lemon::GraphCopy<Graph, Graph> copy(*other._graph, *_graph);

	copy.nodeMap(*_positions, *other._positions);
	copy.edgeMap(*_segments, *other._segments);
	copy.run();
}

void
Skeleton::deleteGraph() {

	if (_segments)
		delete _segments;
	if (_positions)
		delete _positions;
	if (_graph)
		delete _graph;
}

void
Skeleton::openNode(Position pos) {

	Node node = _graph->addNode();

	if (_currentSegment.size() > 0) {

		Node prev = _currentPath.top();
		Edge edge = _graph->addEdge(prev, node);

		(*_segments)[edge] = _currentSegment;
		_currentSegment.clear();
	}

	(*_positions)[node] = pos;
	_currentPath.push(node);
}

void
Skeleton::extendEdge(Position pos) {

	_currentSegment.push_back(pos);
}

void
Skeleton::closeNode() {

	_currentPath.pop();
}

BoundingBox
Skeleton::computeBoundingBox() const {

	BoundingBox bb;

	for (Graph::EdgeIt edge(graph()); edge!= lemon::INVALID; ++edge) {

		const Position& u = positions()[graph().u(edge)];
		const Position& v = positions()[graph().v(edge)];

		bb += BoundingBox(
				std::min(u[0], v[0]), std::min(u[1], v[1]), std::min(u[2], v[2]),
				std::max(u[0], v[0]), std::max(u[1], v[1]), std::max(u[2], v[2]));
	}

	return bb;
}
