#ifndef IMAGEPROCESSING_SKELETONIZE_H__
#define IMAGEPROCESSING_SKELETONIZE_H__

#include <pipeline/SimpleProcessNode.h>
#include "Image.h"

class Skeletonize : public pipeline::SimpleProcessNode<> {

public:

	Skeletonize();

private:

	typedef vigra::MultiArrayView<3, int> view_t;

	void updateOutputs();

	void prepareSkeletonImage();

	void skeletonize(view_t& image);

	void createEulerLut();

	bool isEulerInvariant(const view_t& patch);

	bool isSimplePoint(const view_t& patch);

	template <typename PatchType>
	void getPatch(view_t& image, vigra::Shape3 location, PatchType& patch) {

		// create 3x3x3 patch around image at location
		vigra::Shape3 patchBegin = max(vigra::Shape3(0), location - vigra::Shape3(1));
		vigra::Shape3 patchEnd   = min(image.shape(),    location + vigra::Shape3(2));

		view_t patchView = image.subarray(patchBegin, patchEnd);

		if (patchView.shape() == vigra::Shape3(3)) {

			patch = patchView;

		} else {

			patch = 0;
			patch.subarray(
					patchBegin - location + vigra::Shape3(1),
					patchEnd   - location + vigra::Shape3(1)) = patchView;
		}
	}

	/**
	 * Label the connected components after the center pixel is removed in the 
	 * cube.
	 */
	void labelComponentsAfterRemoval(int octant, int label, int* cube);

	pipeline::Input<Image>  _image;
	pipeline::Output<Image> _skeleton;

	// 3D offsets
	vigra::Shape3 N; // north
	vigra::Shape3 S; // south
	vigra::Shape3 E; // east
	vigra::Shape3 W; // west
	vigra::Shape3 U; // up
	vigra::Shape3 B; // bottom

	// Euler look-up-table
	int _lut[256];
};

#endif // IMAGEPROCESSING_SKELETONIZE_H__

