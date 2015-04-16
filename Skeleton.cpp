#include <util/assert.h>
#include "Skeleton.h"

Skeleton::Skeleton() :
	_prevNode(lemon::INVALID) {

	create();
}

Skeleton::Skeleton(Skeleton&& other) :
	GraphVolume(std::forward<GraphVolume>(other)),
	_prevNode(lemon::INVALID),
	_diameters(other._diameters) {

	other._diameters = 0;
}

Skeleton::Skeleton(const Skeleton& other) :
	GraphVolume(other),
	_prevNode(lemon::INVALID) {

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
	_diameters = new Diameters(graph());
}

void
Skeleton::copy(const Skeleton& other) {

	GraphVolume::copy(other);

	for (NodeIt n(graph()); n != lemon::INVALID; ++n)
		(*_diameters)[n] = (*other._diameters)[n];
}

void
Skeleton::del() {

	GraphVolume::del();

	if (_diameters)
		delete _diameters;

	_diameters= 0;
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

