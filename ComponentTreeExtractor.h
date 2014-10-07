#ifndef IMAGEPROCESSING_COMPNENT_TREE_EXTRACTOR_H__
#define IMAGEPROCESSING_COMPNENT_TREE_EXTRACTOR_H__

#include <pipeline/SimpleProcessNode.h>
#include "ComponentTree.h"
#include "Image.h"

template <typename Precision = unsigned char>
class ComponentTreeExtractor : public pipeline::SimpleProcessNode<> {

public:

private:

	void updateOutputs();

	pipeline::Input<Image>          _image;
	pipelien::Output<ComponentTree> _componentTree;
};

#endif // IMAGEPROCESSING_COMPNENT_TREE_EXTRACTOR_H__

