#include "GraphVolume.h"

GraphVolume::GraphVolume() {}

GraphVolume::GraphVolume(const GraphVolume& other) :
	DiscreteVolume(other) {
	copy(other);
}

GraphVolume&
GraphVolume::operator=(const GraphVolume& other) {
	DiscreteVolume::operator=(other);
	copy(other);
	return *this;
}

void
GraphVolume::copy(const GraphVolume& other) {
	lemon::GraphCopy<Graph, Graph> copy(other.graph(), *_graph);
	copy.nodeMap(other.positions(), *_positions);
	copy.run();
}

util::box<unsigned int,3>
GraphVolume::computeDiscreteBoundingBox() const {

	util::box<unsigned int,3> bb;

	// bounding box of discrete points
	for (Graph::NodeIt node(graph()); node != lemon::INVALID; ++node)
		bb.fit(util::box<unsigned int,3>(positions()[node], positions()[node] + util::point<unsigned int,3>(1, 1, 1)));

	return bb;
}
