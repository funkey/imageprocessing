#ifndef IMAGEPROCESSING_GRAPH_VOLUME_H__
#define IMAGEPROCESSING_GRAPH_VOLUME_H__

#include <memory>
#include "ExplicitVolume.h"
#include <lemon/list_graph.h>
#define WITH_LEMON
#include <vigra/tinyvector.hxx>
#include <vigra/multi_gridgraph.hxx>

/**
 * A volume represented by nodes and edges on a 3D grid. Provides a node map to 
 * store the 3D grid positions of the nodes.
 */
class GraphVolume : public DiscreteVolume {

public:

	typedef lemon::ListGraph Graph;
	typedef Graph::Node      Node;
	typedef Graph::Edge      Edge;
	typedef Graph::NodeIt    NodeIt;
	typedef Graph::EdgeIt    EdgeIt;
	typedef Graph::IncEdgeIt IncEdgeIt;

	typedef util::point<unsigned int,3> Position;
	typedef Graph::NodeMap<Position>    Positions;

	/**
	 * Size of neighborhood of a node.
	 */
	static const int NumNeighbors = 26;

	/**
	 * Create an empty graph volume.
	 */
	GraphVolume();

	/**
	 * Create a graph volume from an explicit volume.
	 */
	template <typename T>
	explicit GraphVolume(const ExplicitVolume<T>& volume);

	/**
	 * Move constructor.
	 */
	GraphVolume(GraphVolume&& other) = default;

	/**
	 * Copy constructor.
	 */
	GraphVolume(const GraphVolume& other);

	/**
	 * Assignment operator.
	 */
	GraphVolume& operator=(const GraphVolume& other);

	Graph& graph() { return *_graph; }
	const Graph& graph() const { return *_graph; }

	Positions& positions() { return *_positions; }
	const Positions& positions() const { return *_positions; }

protected:

	util::box<unsigned int,3> computeDiscreteBoundingBox() const override;

	void copy(const GraphVolume& other);

private:
	std::unique_ptr<Graph>     _graph{new Graph};
	std::unique_ptr<Positions> _positions{new Positions(*_graph)};
};

template <typename T>
GraphVolume::GraphVolume(const ExplicitVolume<T>& volume) {
	vigra::MultiArray<3, Graph::Node> nodeIds(volume.data().shape());
	vigra::GridGraph<3> grid(volume.data().shape(), vigra::IndirectNeighborhood);

	// add all non-background nodes
	for (vigra::GridGraph<3>::NodeIt node(grid); node != lemon::INVALID; ++node) {

		if (volume[node] == 0)
			continue;

		Graph::Node n = _graph->addNode();
		nodeIds[node] = n;
		(*_positions)[n] = Position((*node)[0], (*node)[1], (*node)[2]);
	}

	// add all edges between non-background nodes
	for (vigra::GridGraph<3>::EdgeIt edge(grid); edge != lemon::INVALID; ++edge) {

		int insideVoxels = (volume[grid.u(edge)] != 0) + (volume[grid.v(edge)] != 0);

		if (insideVoxels == 2) {

			Graph::Node u = nodeIds[grid.u(edge)];
			Graph::Node v = nodeIds[grid.v(edge)];

			_graph->addEdge(u, v);
		}
	}

	// our (0,0,0) would be at the same location as volume's (0,0,0)
	setOffset(volume.getOffset());
	setResolution(volume.getResolution());
}

#endif // IMAGEPROCESSING_GRAPH_VOLUME_H__

