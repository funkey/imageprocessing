#include <util/assert.h>
#include "Skeleton.h"

Skeleton::Skeleton() {

	create();
}

Skeleton::Skeleton(Skeleton&& other) :
	GraphVolume(std::forward<GraphVolume>(other)),
	_segments(other._segments) {

	other._segments = 0;
}

Skeleton::Skeleton(const Skeleton& other) :
	GraphVolume(other) {

	create();
	copy(other);
}

Skeleton&
Skeleton::operator=(const Skeleton& other) {

	del();
	create();
	copy(other);

	return *this;
}

Skeleton::~Skeleton() {

	del();
}

void
Skeleton::create() {

	GraphVolume::create();
	_segments = new Segments(graph());
}

void
Skeleton::copy(const Skeleton& other) {

	GraphVolume::copy(other);

	for (EdgeIt e(graph()); e != lemon::INVALID; ++e)
		(*_segments)[e] = (*other._segments)[e];
}

void
Skeleton::del() {

	GraphVolume::del();

	if (_segments)
		delete _segments;

	_segments = 0;
}

void
Skeleton::openNode(Position pos) {

	Node node = graph().addNode();

	if (_currentSegment.size() > 0) {

		Node prev = _currentPath.top();
		Edge edge = graph().addEdge(prev, node);

		(*_segments)[edge] = _currentSegment;
		_currentSegment.clear();
	}

	positions()[node] = pos;
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

