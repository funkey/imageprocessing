#ifndef IMAGEPROCESSING_COMPONENT_TREE_PRUNER_H__
#define IMAGEPROCESSING_COMPONENT_TREE_PRUNER_H__

#include <pipeline/all.h>
#include <imageprocessing/ComponentTree.h>

/**
 * The component tree pruner removes nodes from a given component tree if they 
 * exceed a maximal height in the tree. The height is counted as the maximum 
 * number downwards of edges to a leaf node.
 */
class ComponentTreePruner : public pipeline::SimpleProcessNode<> {

public:

	ComponentTreePruner();

private:

	void updateOutputs();

	void prune();

	/**
	 * Prune the subtree rooted at node, return the result and the level of node 
	 * counted from the bottom.
	 */
	boost::shared_ptr<ComponentTree::Node> prune(
			boost::shared_ptr<ComponentTree::Node> node,
			int& level);

	pipeline::Input<ComponentTree>  _componentTree;
	pipeline::Input<int>            _maxHeight;
	pipeline::Output<ComponentTree> _pruned;

	boost::shared_ptr<ComponentTree::Node> _root;
};

#endif // IMAGEPROCESSING_COMPONENT_TREE_PRUNER_H__

