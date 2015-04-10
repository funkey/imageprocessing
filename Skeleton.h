#ifndef IMAGEPROCESSING_TUBES_SKELETON_H__
#define IMAGEPROCESSING_TUBES_SKELETON_H__

#include <stack>
#include <vector>
#include <lemon/list_graph.h>
#include "GraphVolume.h"

/**
 * Represents a skeleton as a graph of terminal nodes and branch points.
 */
class Skeleton : public GraphVolume {

public:

	/**
	 * Skeleton graph datastructures: Nodes are terminal and branch points, 
	 * edges indicate skeleton segments between nodes.
	 */
	typedef lemon::ListGraph Graph;
	typedef Graph::Node      Node;
	typedef Graph::Edge      Edge;

	/**
	 * Pixel locations belonging to one edge in the skeleton graph.
	 */
	typedef vigra::TinyVector<float, 3> Position;
	typedef std::vector<Position>       Segment;

	/**
	 * Node and edge property maps.
	 */
	typedef Graph::NodeMap<Position> Positions;
	typedef Graph::EdgeMap<Segment>  Segments;

	/**
	 * Create an empty skeleton.
	 */
	Skeleton();

	/**
	 * Move constructor.
	 */
	Skeleton(Skeleton&& other);

	/**
	 * Copy constructor.
	 */
	Skeleton(const Skeleton& other);

	/**
	 * Assignment operator.
	 */
	Skeleton& operator=(const Skeleton& other);

	~Skeleton();

	/**
	 * Get the edge property map that stores a list of positions for each 
	 * skeleton edge.
	 */
	Segments& segments() { return *_segments; }
	const Segments& segments() const { return *_segments; }

	/**
	 * Start a new node in the skeleton at the given position.
	 */
	void openNode(Position pos);

	/**
	 * Extend the edge of the currently open node by one position.
	 */
	void extendEdge(Position pos);

	/**
	 * Close the currently open node, backtrack to the previous node in the 
	 * tree.
	 */
	void closeNode();

protected:

	util::box<float,3> computeBoundingBox() const override;

private:

	void create();
	void copy(const Skeleton& other);
	void del();

	Segments* _segments;

	std::stack<Node>      _currentPath;
	std::vector<Position> _currentSegment;
};

#endif // IMAGEPROCESSING_TUBES_SKELETON_H__

