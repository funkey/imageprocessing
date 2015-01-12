#ifndef IMAGEPROCESSING_EXTRACT_SKELETON_H__
#define IMAGEPROCESSING_EXTRACT_SKELETON_H__

#include <pipeline/SimpleProcessNode.h>
#include "ImageStack.h"
#include "Skeletonize.h"

class ExtractSkeleton : public pipeline::SimpleProcessNode<> {

public:

	ExtractSkeleton();

private:

	void updateOutputs();

	/**
	 * Copy the input image in an intermediate data structure to find the 
	 * skeleton.
	 */
	void prepareSkeletonImage();

	pipeline::Input<ImageStack>  _stack;
	pipeline::Output<ImageStack> _skeleton;

	Skeletonize _skeletonize;
};


#endif // IMAGEPROCESSING_EXTRACT_SKELETON_H__

