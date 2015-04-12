#ifndef IMAGEPROCESSING_GRAPH_VOLUME_H__
#define IMAGEPROCESSING_GRAPH_VOLUME_H__

#include "ExplicitVolume.h"
#include <lemon/list_graph.h>
#define WITH_LEMON
#include <vigra/tinyvector.hxx>
#include <vigra/multi_gridgraph.hxx>

/**
 * A volume represent by nodes and edges in 3D. Provides a node map to store the 
 * 3D positions of the nodes, and a list of boundary nodes.
 */
class GraphVolume : public DiscreteVolume {

public:

	typedef lemon::ListGraph Graph;
	typedef Graph::Node      Node;
	typedef Graph::Edge      Edge;
	typedef Graph::NodeIt    NodeIt;
	typedef Graph::EdgeIt    EdgeIt;
	typedef Graph::IncEdgeIt IncEdgeIt;

	typedef vigra::TinyVector<float, 3> Position;
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
	GraphVolume(GraphVolume&& other);

	/**
	 * Copy constructor.
	 */
	GraphVolume(const GraphVolume& other);

	/**
	 * Assignment operator.
	 */
	GraphVolume& operator=(const GraphVolume& other);

	~GraphVolume();

	Graph& graph() { return *_graph; }
	const Graph& graph() const { return *_graph; }

	Positions& positions() { return *_positions; }
	const Positions& positions() const { return *_positions; }

	unsigned int width()  const { return _size.x(); }
	unsigned int height() const { return _size.y(); }
	unsigned int depth()  const { return _size.z(); }

protected:

	util::box<float,3> computeBoundingBox() const override;

	void create();
	void copy(const GraphVolume& other);
	void del();

private:

	Graph*     _graph;
	Positions* _positions;

	util::point<unsigned int,3> _size;
};

template <typename T>
GraphVolume::GraphVolume(const ExplicitVolume<T>& volume) {

	create();

	setResolution(
			volume.getResolutionX(),
			volume.getResolutionY(),
			volume.getResolutionZ());
	setBoundingBox(volume.getBoundingBox());

	_size = util::point<unsigned int,3>(
			volume.width(),
			volume.height(),
			volume.depth());

	vigra::MultiArray<3, Graph::Node> nodeIds(volume.data().shape());
	vigra::GridGraph<3> grid(volume.data().shape(), vigra::IndirectNeighborhood);

	// add all non-background nodes
	for (vigra::GridGraph<3>::NodeIt node(grid); node != lemon::INVALID; ++node) {

		if (volume[node] == 0)
			continue;

		Graph::Node n = _graph->addNode();
		nodeIds[node] = n;
		(*_positions)[n] = *node;
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
}

#endif // IMAGEPROCESSING_GRAPH_VOLUME_H__

