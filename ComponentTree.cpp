#include "ComponentTree.h"

ComponentTree::Node::Node() {}

ComponentTree::Node::Node(boost::shared_ptr<ConnectedComponent> component) :
	_component(component) {}

void
ComponentTree::Node::setParent(boost::shared_ptr<ComponentTree::Node> parent) {

	_parent = parent;
}

boost::shared_ptr<ComponentTree::Node>
ComponentTree::Node::getParent() {

	boost::shared_ptr<ComponentTree::Node> parent = _parent.lock();

	return parent;
}

void
ComponentTree::Node::addChild(boost::shared_ptr<ComponentTree::Node> componentNode) {

	_children.push_back(componentNode);
}

bool
ComponentTree::Node::removeChild(boost::shared_ptr<Node> child) {

	std::vector<boost::shared_ptr<ComponentTree::Node> >::iterator i =
			std::find(_children.begin(), _children.end(), child);

	if (i == _children.end())
		return false;

	_children.erase(i);

	return true;
}

const std::vector<boost::shared_ptr<ComponentTree::Node> >&
ComponentTree::Node::getChildren() const {

	return _children;
}

void
ComponentTree::Node::setComponent(boost::shared_ptr<ConnectedComponent> component) {

	_component = component;
}

boost::shared_ptr<ConnectedComponent>
ComponentTree::Node::getComponent() {

	return _component;
}

ComponentTree::ComponentTree() :
	_boundingBox(0, 0, 0, 0) {}

void
ComponentTree::clear() {

	_root.reset();
	_boundingBox = util::box<double,2>(0, 0, 0, 0);
}

void
ComponentTree::setRoot(boost::shared_ptr<ComponentTree::Node> root) {

	_root = root;
	updateBoundingBox();
}

boost::shared_ptr<ComponentTree::Node>
ComponentTree::getRoot() {

	return _root;
}

unsigned int
ComponentTree::size() const {

	return count(_root);
}

unsigned int
ComponentTree::count(boost::shared_ptr<ComponentTree::Node> node) const {

	int numNodes = 0;

	foreach (boost::shared_ptr<ComponentTree::Node> child, node->getChildren())
		numNodes += count(child);

	numNodes++;

	return numNodes;
}

const util::box<double,2>&
ComponentTree::getBoundingBox() const {

	return _boundingBox;
}

/**
 * Creates a copy of the component tree, but not a copy of the involved
 * connected components.
 *
 * @return A copy of the component tree.
 */
ComponentTree
ComponentTree::clone() {

	boost::shared_ptr<ComponentTree::Node> root = clone(_root);

	ComponentTree tree;

	tree.setRoot(_root);

	return tree;
}

boost::shared_ptr<ComponentTree::Node>
ComponentTree::clone(boost::shared_ptr<ComponentTree::Node> node) {

	boost::shared_ptr<ComponentTree::Node> nodeClone = boost::make_shared<ComponentTree::Node>(node->getComponent());

	foreach (boost::shared_ptr<ComponentTree::Node> child, node->getChildren()) {

		boost::shared_ptr<ComponentTree::Node> childClone = clone(child);

		nodeClone->addChild(childClone);
	}

	return nodeClone;
}

void
ComponentTree::updateBoundingBox() {

	_boundingBox = updateBoundingBox(_root);
}

util::box<double,2>
ComponentTree::updateBoundingBox(boost::shared_ptr<ComponentTree::Node> node) {

	util::box<double,2> boundingBox = node->getComponent()->getBoundingBox();

	foreach (boost::shared_ptr<ComponentTree::Node> child, node->getChildren()) {

		util::box<double,2> childBoundingBox = updateBoundingBox(child);

		boundingBox.min().x() = std::min(boundingBox.min().x(), childBoundingBox.min().x());
		boundingBox.max().x() = std::max(boundingBox.max().x(), childBoundingBox.max().x());
		boundingBox.min().y() = std::min(boundingBox.min().y(), childBoundingBox.min().y());
		boundingBox.max().y() = std::max(boundingBox.max().y(), childBoundingBox.max().y());
	}

	return boundingBox;
}

