#include <util/assert.h>
#include "Skeleton.h"

Skeleton::Skeleton() {}

Skeleton::Skeleton(const Skeleton& other) :
	GraphVolume(other) {
	copy(other);
}

Skeleton&
Skeleton::operator=(const Skeleton& other) {
	GraphVolume::operator=(other);
	copy(other);
	return *this;
}

void
Skeleton::copy(const Skeleton& other) {
	_currentSegmentPath = other._currentSegmentPath;
	_prevNode = other._prevNode;
	for (NodeIt n(graph()); n != lemon::INVALID; ++n)
		(*_diameters)[n] = (*other._diameters)[n];
}

Skeleton::Node
Skeleton::openSegment(Position pos, float diameter) {

	Node node = extendSegment(pos, diameter);
	_currentSegmentPath.push(node);

	return node;
}

Skeleton::Node
Skeleton::extendSegment(Position pos, float diameter) {

	Node node = graph().addNode();
	positions()[node] = pos;
	diameters()[node] = diameter;

	if (_prevNode != lemon::INVALID)
		graph().addEdge(_prevNode, node);
	_prevNode = node;

	return node;
}

void
Skeleton::closeSegment() {

	if (_currentSegmentPath.size() == 0)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"closeSegment() called without prior call to openSegment()");

	_currentSegmentPath.pop();

	if (_currentSegmentPath.size() > 0)
		_prevNode = _currentSegmentPath.top();
	else
		_prevNode = Node(lemon::INVALID);
}

