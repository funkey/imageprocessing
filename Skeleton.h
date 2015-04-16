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

	typedef Graph::NodeMap<float> Diameters;

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
	 * Start a new segment (a chain of nodes) in the skeleton at the given 
	 * position.
	 */
	Node openSegment(Position pos, float diameter);

	/**
	 * Extend the currently open segment by one position.
	 */
	Node extendSegment(Position pos, float diameter);

	/**
	 * Close the currently open segment, backtrack to the end of the previous 
	 * segment in the tree.
	 */
	void closeSegment();

	/**
	 * Get a node property map with the diameters of each skeleton node.
	 */
	Diameters& diameters() { return *_diameters; }
	const Diameters& diameters() const { return *_diameters; }

private:

	void create();
	void copy(const Skeleton& other);
	void del();

	// list of previous segment end nodes
	std::stack<Node> _currentSegmentPath;
	// previously added node
	Node             _prevNode;

	Diameters* _diameters;
};

#endif // IMAGEPROCESSING_TUBES_SKELETON_H__

