#include <util/foreach.h>
#include <util/Logger.h>
#include "ComponentTreeDownSampler.h"

static logger::LogChannel componenttreedownsamplerlog("componenttreedownsamplerlog", "[ComponentTreeDownSampler] ");

ComponentTreeDownSampler::ComponentTreeDownSampler() {

	registerInput(_componentTree, "component tree");
	registerOutput(_downsampled,  "component tree");
}

void
ComponentTreeDownSampler::updateOutputs() {

	if (!_downsampled)
		_downsampled = new ComponentTree();

	downsample();
}

void
ComponentTreeDownSampler::downsample() {

	boost::shared_ptr<ComponentTree::Node> rootNode = _componentTree->getRoot();

	// create a clone of the root node
	boost::shared_ptr<ComponentTree::Node> rootNodeClone = boost::make_shared<ComponentTree::Node>(rootNode->getComponent());

	// downsample the trees under every child of the root node and add them to 
	// the cloned root node
	foreach (boost::shared_ptr<ComponentTree::Node> child, rootNode->getChildren()) {

		boost::shared_ptr<ComponentTree::Node> childClone = downsample(child);

		rootNodeClone->addChild(childClone);
	}

	// set the downsampled component tree
	_downsampled->setRoot(rootNodeClone);
}

boost::shared_ptr<ComponentTree::Node>
ComponentTreeDownSampler::downsample(boost::shared_ptr<ComponentTree::Node> node) {

	// create a clone of the node
	boost::shared_ptr<ComponentTree::Node> nodeClone = boost::make_shared<ComponentTree::Node>(node->getComponent());

	// skip over all single children
	while (node->getChildren().size() == 1)
		node = node->getChildren().front();

	// downsample the trees under every child and add them to the clone
	foreach (boost::shared_ptr<ComponentTree::Node> child, node->getChildren()) {

		boost::shared_ptr<ComponentTree::Node> childClone = downsample(child);

		nodeClone->addChild(childClone);
	}

	// return the clone
	return nodeClone;
}
