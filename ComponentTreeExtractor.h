#ifndef IMAGEPROCESSING_COMPNENT_TREE_EXTRACTOR_H__
#define IMAGEPROCESSING_COMPNENT_TREE_EXTRACTOR_H__

#include <pipeline/SimpleProcessNode.h>
#include <imageprocessing/ImageLevelParser.h>
#include "ComponentTree.h"
#include "ComponentTreeExtractorParameters.h"
#include "Image.h"
#include <util/Logger.h>

extern logger::LogChannel componenttreeextractorlog;

template <typename Precision = unsigned char>
class ComponentTreeExtractor : public pipeline::SimpleProcessNode<> {

public:

	ComponentTreeExtractor();

private:

	class ComponentVisitor {

	public:

		ComponentVisitor(
				boost::shared_ptr<Image>         image,
				unsigned int                     minSize,
				unsigned int                     maxSize) :
			_image(image),
			_minSize(minSize),
			_maxSize(maxSize),
			_prevBegin(0),
			_prevEnd(0) {}

		void newChildComponent(float value);

		void finalizeComponent(
				float                        value,
				PixelList::const_iterator    begin,
				PixelList::const_iterator    end,
				boost::shared_ptr<PixelList> pixelList);

		boost::shared_ptr<ComponentTree::Node> getRoot() { return _rootNode; }

	private:

		boost::shared_ptr<Image>         _image;

		boost::shared_ptr<ComponentTree::Node> _rootNode;
		boost::shared_ptr<ComponentTree::Node> _currentNode;

		unsigned int _minSize;
		unsigned int _maxSize;

		// extents of the previous component
		PixelList::const_iterator _prevBegin;
		PixelList::const_iterator _prevEnd;
	};

	void updateOutputs();

	pipeline::Input<Image>                            _image;
	pipeline::Input<ComponentTreeExtractorParameters> _parameters;
	pipeline::Output<ComponentTree>                   _componentTree;
};

////////////////////
// IMPLEMENTATION //
////////////////////

template <typename Precision>
void
ComponentTreeExtractor<Precision>::ComponentVisitor::newChildComponent(float value) {

	LOG_ALL(componenttreeextractorlog)
			<< "create new component for value " << value << std::endl;

	LOG_ALL(componenttreeextractorlog)
			<< "current node is " << _currentNode << std::endl;

	if (!_currentNode) {

		LOG_ALL(componenttreeextractorlog)
				<< "this is the first component -- I make it the root" << std::endl;

		// create the root node
		_currentNode = boost::make_shared<ComponentTree::Node>();
		_rootNode    = _currentNode;

	} else {

		// create a child of the current node and go down
		boost::shared_ptr<ComponentTree::Node> newNode = boost::make_shared<ComponentTree::Node>();

		_currentNode->addChild(newNode);
		newNode->setParent(_currentNode);

		_currentNode = newNode;

		LOG_ALL(componenttreeextractorlog)
				<< "setting the new current node to " << _currentNode << std::endl;
		LOG_ALL(componenttreeextractorlog)
				<< "parent of new current node is " << _currentNode->getParent() << std::endl;
	}
}

template <typename Precision>
void
ComponentTreeExtractor<Precision>::ComponentVisitor::finalizeComponent(
		float                        value,
		PixelList::const_iterator    begin,
		PixelList::const_iterator    end,
		boost::shared_ptr<PixelList> pixelList) {

	LOG_ALL(componenttreeextractorlog)
			<< "finalize component with value " << value << std::endl;

	assert(_currentNode);

	boost::shared_ptr<ComponentTree::Node> parent = _currentNode->getParent();
	size_t size = end - begin;

	// is this an invalid component that is not the root node?
	if (parent && (
			size < _minSize ||
			(_maxSize > 0 && size >= _maxSize) ||
			(begin == _prevBegin && end == _prevEnd))
		) {

		LOG_ALL(componenttreeextractorlog)
				<< "I don't want this component (" << size
				<< " not in [" << _minSize << ", " << _maxSize
				<< ") and not the root node) -- skip it" << std::endl;

		parent->removeChild(_currentNode);
		foreach (boost::shared_ptr<ComponentTree::Node> child, _currentNode->getChildren()) {

			child->setParent(parent);
			parent->addChild(child);
		}

	// valid component or root node (that we want to keep anyway)
	} else {

		// create a connected component
		boost::shared_ptr<ConnectedComponent> component =
				boost::make_shared<ConnectedComponent>(
						_image,
						value,
						pixelList,
						begin,
						end);

		// set it to the current node
		_currentNode->setComponent(component);

		LOG_ALL(componenttreeextractorlog)
				<< "setting the new current node to " << _currentNode << std::endl;
	}

	// make the parent of the current component the new current component
	// (if this was the root node, the new current node should point to 0)
	_currentNode = parent;

	_prevBegin = begin;
	_prevEnd   = end;
}

template <typename Precision>
ComponentTreeExtractor<Precision>::ComponentTreeExtractor() {

	registerInput(_image, "image");
	registerInput(_parameters, "parameters", pipeline::Optional);

	registerOutput(_componentTree, "component tree");
}

template <typename Precision>
void
ComponentTreeExtractor<Precision>::updateOutputs() {

	if (!_componentTree)
		_componentTree = new ComponentTree();
	else
		_componentTree->clear();

	unsigned int minSize = 0;
	unsigned int maxSize = 0;

	if (_parameters.isSet()) {

		minSize = _parameters->minSize;
		maxSize = _parameters->maxSize;
	}

	// create a new visitor
	ComponentVisitor visitor(_image.getSharedPointer(), minSize, maxSize);

	// create an image level parser
	typename ImageLevelParser<Precision>::Parameters parameters;
	parameters.darkToBright = (_parameters.isSet() ? _parameters->darkToBright : true);
	ImageLevelParser<Precision> parser(*_image, parameters);

	// let the visitor run over the components
	parser.parse(visitor);

	// set the root node in the component tree
	_componentTree->setRoot(visitor.getRoot());
}

#endif // IMAGEPROCESSING_COMPNENT_TREE_EXTRACTOR_H__

