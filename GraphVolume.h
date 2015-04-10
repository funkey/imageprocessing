#ifndef IMAGEPROCESSING_GRAPH_VOLUME_H__
#define IMAGEPROCESSING_GRAPH_VOLUME_H__

#include "Volume.h"
#include <lemon/list_graph.h>
#include <vigra/tinyvector.hxx>

/**
 * A volume represent by nodes and edges in 3D.
 */
class GraphVolume : public Volume {

public:

	typedef lemon::ListGraph Graph;
	typedef Graph::Node      Node;
	typedef Graph::Edge      Edge;
	typedef Graph::NodeIt    NodeIt;
	typedef Graph::EdgeIt    EdgeIt;

	typedef vigra::TinyVector<float, 3> Position;
	typedef Graph::NodeMap<Position>    Positions;

	/**
	 * Create an empty graph volume.
	 */
	GraphVolume();

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

protected:

	util::box<float,3> computeBoundingBox() const override;

	void create();
	void copy(const GraphVolume& other);
	void del();

private:

	Graph*     _graph;
	Positions* _positions;
};

#endif // IMAGEPROCESSING_GRAPH_VOLUME_H__

