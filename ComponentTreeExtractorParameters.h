#ifndef IMAGEPROCESSING_COMPONENT_TREE_EXTRACTOR_PARAMETERS_H__
#define IMAGEPROCESSING_COMPONENT_TREE_EXTRACTOR_PARAMETERS_H__

#include <limits>
#include <pipeline/Data.h>

struct ComponentTreeExtractorParameters : public pipeline::Data {

	ComponentTreeExtractorParameters() :
		darkToBright(true),
		minSize(0),
		maxSize(std::numeric_limits<std::size_t>::max()),
		minIntensity(0),
		maxIntensity(0),
		sameIntensityComponents(false),
		spacedEdgeImage(false) {}

	// extract components, start with the darkest
	bool         darkToBright;

	// only consider components of at least this size
	unsigned int minSize;

	// only consider components of at most this size
	unsigned int maxSize;

	/**
	 * The min and max intensity of the image, used for discretization into 
	 * the Precision type. The default is 0 for both, in which case the 
	 * image is inspected to find them. You can set them to avoid this 
	 * inspection or to ensure that the values of the connected components 
	 * math across different images that might have different intensity 
	 * extrema.
	 */
	float minIntensity;
	float maxIntensity;

	// extract a flat tree that has only same-intensity regions
	bool sameIntensityComponents;

	// indicate that the image to parse is a scaled edge image (see 
	// ImageLevelParser::Parameters for details)
	bool spacedEdgeImage;
};

#endif // IMAGEPROCESSING_COMPONENT_TREE_EXTRACTOR_PARAMETERS_H__

