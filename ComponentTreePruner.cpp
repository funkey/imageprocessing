#include <util/foreach.h>
#include <util/Logger.h>
#include "ComponentTreePruner.h"

static logger::LogChannel componenttreeprunerlog("componenttreeprunerlog", "[ComponentTreePruner] ");

ComponentTreePruner::ComponentTreePruner() {

	registerInput(_componentTree, "component tree");
	registerInput(_maxHeight, "max height");
	registerOutput(_pruned,  "component tree");
}

void
ComponentTreePruner::updateOutputs() {

	prune();
}

void
ComponentTreePruner::prune() {

	// the new root will be a clone of the old root
	_root = boost::make_shared<ComponentTree::Node>(_componentTree->getRoot()->getComponent());

	// copy and prune on-the-fly, starting with the root node
	int rootLevel;
	boost::shared_ptr<ComponentTree::Node> pruned = prune(_componentTree->getRoot(), rootLevel);

	// the whole tree did not exceed the threshold
	if (pruned)
		_root = pruned;

	_pruned->setRoot(_root);
}

boost::shared_ptr<ComponentTree::Node>
ComponentTreePruner::prune(
		boost::shared_ptr<ComponentTree::Node> node,
		int& level) {

	// copies of all children of node that do not exceed the threshold
	std::vector<boost::shared_ptr<ComponentTree::Node> > validChildren;

	// prune the trees under every child and add them to the clone
	int maxChildLevel = -1;
	foreach (boost::shared_ptr<ComponentTree::Node> child, node->getChildren()) {

		// copy the child nodes and get their level
		int childLevel;
		boost::shared_ptr<ComponentTree::Node> childClone = prune(child, childLevel);

		if (childLevel > maxChildLevel)
			maxChildLevel = childLevel;

		// collect the child clones temporarily (if there was no child clone 
		// returned, child was already exceeding the threshold)
		if (childClone)
			validChildren.push_back(childClone);
	}

	// update our level (zero if we have no children)
	level = maxChildLevel + 1;

	// we are exceeding the threshold
	if (level > *_maxHeight) {

		// connect our children (that are not exceeding the threshold) directly 
		// to the root node
		foreach (boost::shared_ptr<ComponentTree::Node> child, validChildren)
			_root->addChild(child);

		// return nothing to indicate we are done in this branch
		return boost::shared_ptr<ComponentTree::Node>();
	}

	// we are good, create a copy...
	boost::shared_ptr<ComponentTree::Node> nodeClone = boost::make_shared<ComponentTree::Node>(node->getComponent());

	// ...connect our children to it...
	foreach (boost::shared_ptr<ComponentTree::Node> child, validChildren)
		nodeClone->addChild(child);

	// ...and return it
	return nodeClone;
}
