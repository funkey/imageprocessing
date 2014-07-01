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

	/**
	 * Copy the input image in an intermediate data structure to find the 
	 * skeleton.
	 */
	void prepareSkeletonImage();

	/**
	 * Iteratively erode the skeleton image.
	 */
	void skeletonize(view_t& image);

	/**
	 * Check, whether a foreground pixel (described by the patch around it) can 
	 * be deleted.
	 */
	bool canBeDeleted(const view_t& patch);

	/**
	 * Is the pixel centered in patch on a boundary?
	 */
	bool isBorder(const view_t& patch);

	/**
	 * Is the pixel centered in patch the end of an arch?
	 */
	bool isArchEnd(const view_t& patch);

	/**
	 * Is the pixel centered in patch Euler invariant?
	 */
	bool isEulerInvariant(const view_t& patch);

	/**
	 * Is the pixel centered in patch simple, i.e., would deleting it create 
	 * disconnected components?
	 */
	bool isSimplePoint(const view_t& patch);

	/**
	 * Mark a location as a simple border point that can be deleted.
	 */
	void markAsSimpleBorderPoint(const vigra::Shape3& i);

	/**
	 * Delete simple border points that were marked as "can be deleted" earlier.
	 */
	unsigned int deleteSimpleBorderPoints(view_t& image);

	/**
	 * Fill the look-up-table for fast Euler invariance test.
	 */
	void createEulerLut();

	/**
	 * Fill a 3x3x3 patch with a subarray from image centered around location.
	 */
	template <typename PatchType>
	void getPatch(view_t& image, vigra::Shape3 location, PatchType& patch) {

		// create 3x3x3 patch around image at location
		vigra::Shape3 patchBegin = max(vigra::Shape3(0), location - vigra::Shape3(1));
		vigra::Shape3 patchEnd   = min(image.shape(),    location + vigra::Shape3(2));

		view_t patchView = image.subarray(patchBegin, patchEnd);

		if (patchView.shape() == vigra::Shape3(3)) {

			patch = patchView;

		// border cases: fill with zero
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

	// is the image volumetric?
	bool _isVolume;

	int _currentBorder;

	std::vector<vigra::Shape3> _simpleBorderPoints;
};

#endif // IMAGEPROCESSING_SKELETONIZE_H__

