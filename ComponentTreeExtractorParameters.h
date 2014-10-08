#ifndef IMAGEPROCESSING_COMPONENT_TREE_EXTRACTOR_PARAMETERS_H__
#define IMAGEPROCESSING_COMPONENT_TREE_EXTRACTOR_PARAMETERS_H__

#include <pipeline/Data.h>

struct ComponentTreeExtractorParameters : public pipeline::Data {

	// extract components, start with the darkest
	bool         darkToBright;

	// only consider components of at least this size
	unsigned int minSize;

	// only consider components of at most this size
	unsigned int maxSize;
};

#endif // IMAGEPROCESSING_COMPONENT_TREE_EXTRACTOR_PARAMETERS_H__

