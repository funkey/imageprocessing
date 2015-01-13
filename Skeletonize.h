#ifndef IMAGEPROCESSING_SKELETONIZE_H__
#define IMAGEPROCESSING_SKELETONIZE_H__

#include <vigra/multi_array.hxx>
#include <vigra/distancetransform.hxx>
#include <vigra/multi_pointoperators.hxx>
#include <vigra/functorexpression.hxx>

template <typename ValueType>
class Skeletonize {

public:

	typedef vigra::MultiArrayView<3, ValueType> view_t;

	Skeletonize();

	/**
	 * Iteratively erode the skeleton image.
	 */
	void skeletonize(view_t& image);

private:

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

	// 3D offsets
	vigra::Shape3 N; // north
	vigra::Shape3 S; // south
	vigra::Shape3 E; // east
	vigra::Shape3 W; // west
	vigra::Shape3 U; // up
	vigra::Shape3 B; // bottom

	// Euler look-up-table
	int _lut[256];

	// copy patch for labeling
	int _cube[26];

	// is the image volumetric?
	bool _isVolume;

	int _currentBorder;

	std::vector<vigra::Shape3> _simpleBorderPoints;
};

////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////

template <typename ValueType>
Skeletonize<ValueType>::Skeletonize() :
		// offsets in patch centered at (1, 1, 1)
		N(1, 0, 1),
		S(1, 2, 1),
		E(2, 1, 1),
		W(0, 1, 1),
		U(1, 1, 2),
		B(1, 1, 0) {

	createEulerLut();
}

template <typename ValueType>
void
Skeletonize<ValueType>::skeletonize(view_t& image){

	// the size of the image
	vigra::Shape3 size = image.shape();

	// index to run over all pixels
	vigra::Shape3 i;

	// do we have a volumetric image?
	_isVolume = (size[2] > 1);

	// 3x3x3 neighborhood patch
	vigra::MultiArray<3, ValueType> patch(vigra::Shape3(3));

	// Loop through the image several times until there is no change.
	int unchangedBorders = 0;
	while (unchangedBorders < 6) { // loop until no change for all the six border types

		unchangedBorders = 0;

		for (_currentBorder = 1; _currentBorder <= 6; _currentBorder++) {

			// iterate over all pixels
			for (i[2] = 0; i[2] < size[2]; i[2]++)
			for (i[1] = 0; i[1] < size[1]; i[1]++)
			for (i[0] = 0; i[0] < size[0]; i[0]++) {

				// unbelievable -- on a  500x500 test image, this test is ~10% 
				// faster than:
				//   if (image[i] == 0)
				if (!(image[i] > 0 || image[i] < 0))
					continue; // current point is already background 

				// get the 3x3x3 patch around i
				getPatch(image, i, patch);

				if (canBeDeleted(patch)) {

					// add all simple border points to a list for sequential 
					// re-checking
					markAsSimpleBorderPoint(i);
				}
			}

			if (deleteSimpleBorderPoints(image) == 0)
				unchangedBorders++;

		} // end currentBorder for loop

	} // end unchangedBorders while loop
}

template <typename ValueType>
bool
Skeletonize<ValueType>::canBeDeleted(const view_t& patch) {

	if (!isBorder(patch))
		return false;

	if (isArchEnd(patch))
		return false;

	if (!isEulerInvariant(patch))
		return false;

	if (!isSimplePoint(patch))
		return false;

	return true;
}

template <typename ValueType>
bool
Skeletonize<ValueType>::isBorder(const view_t& patch) {

	// check 6-neighbors if point is a border point of type 
	// currentBorder
	bool isBorderPoint = false;
	if      (_currentBorder == 1 && patch[N] == 0)
		isBorderPoint = true;
	else if (_currentBorder == 2 && patch[S] == 0)
		isBorderPoint = true;
	else if (_currentBorder == 3 && patch[E] == 0)
		isBorderPoint = true;
	else if (_currentBorder == 4 && patch[W] == 0)
		isBorderPoint = true;
	// don't consider pixels of a 2D image as border pixels in the z 
	// directions
	else if (_isVolume) {

		if      (_currentBorder == 5 && patch[U] == 0)
			isBorderPoint = true;
		else if (_currentBorder == 6 && patch[B] == 0)
			isBorderPoint = true;
	}

	return isBorderPoint;
}

template <typename ValueType>
bool
Skeletonize<ValueType>::isArchEnd(const view_t& patch) {

	// check if point is the end of an arc
	int numberOfNeighbors = -1; // -1 and not 0 because the center pixel will be counted as well	
	for (typename view_t::iterator j = patch.begin(); j != patch.end(); j++)
		numberOfNeighbors += (*j == 0 ? 0 : 1);

	if (numberOfNeighbors == 1)
		return true;

	return false;
}

template <typename ValueType>
void
Skeletonize<ValueType>::markAsSimpleBorderPoint(const vigra::Shape3& i) {

	_simpleBorderPoints.push_back(i);
}

template <typename ValueType>
unsigned int
Skeletonize<ValueType>::deleteSimpleBorderPoints(view_t& image) {

	vigra::MultiArray<3, ValueType> patch(vigra::Shape3(3));
	unsigned int deleted = 0;

	std::vector<vigra::Shape3>::iterator j;
	for (j = _simpleBorderPoints.begin(); j != _simpleBorderPoints.end(); j++) {

		// Check if neighborhood would still be connected
		getPatch(image, *j, patch);
		if (isSimplePoint(patch)) {

			// we can delete current point
			image[*j] = 0;
			deleted++;

		}
	}

	_simpleBorderPoints.clear();

	return deleted;
}

template <typename ValueType>
void
Skeletonize<ValueType>::createEulerLut(){

	_lut[1]  =  1;
	_lut[3]  = -1;
	_lut[5]  = -1;
	_lut[7]  =  1;
	_lut[9]  = -3;
	_lut[11] = -1;
	_lut[13] = -1;
	_lut[15] =  1;
	_lut[17] = -1;
	_lut[19] =  1;
	_lut[21] =  1;
	_lut[23] = -1;
	_lut[25] =  3;
	_lut[27] =  1;
	_lut[29] =  1;
	_lut[31] = -1;
	_lut[33] = -3;
	_lut[35] = -1;
	_lut[37] =  3;
	_lut[39] =  1;
	_lut[41] =  1;
	_lut[43] = -1;
	_lut[45] =  3;
	_lut[47] =  1;
	_lut[49] = -1;
	_lut[51] =  1;
	_lut[53] =  1;
	_lut[55] = -1;
	_lut[57] =  3;
	_lut[59] =  1;
	_lut[61] =  1;
	_lut[63] = -1;
	_lut[65] = -3;
	_lut[67] =  3;
	_lut[69] = -1;
	_lut[71] =  1;
	_lut[73] =  1;
	_lut[75] =  3;
	_lut[77] = -1;
	_lut[79] =  1;
	_lut[81] = -1;
	_lut[83] =  1;
	_lut[85] =  1;
	_lut[87] = -1;
	_lut[89] =  3;
	_lut[91] =  1;
	_lut[93] =  1;
	_lut[95] = -1;
	_lut[97] =  1;
	_lut[99] =  3;
	_lut[101] =  3;
	_lut[103] =  1;
	_lut[105] =  5;
	_lut[107] =  3;
	_lut[109] =  3;
	_lut[111] =  1;
	_lut[113] = -1;
	_lut[115] =  1;
	_lut[117] =  1;
	_lut[119] = -1;
	_lut[121] =  3;
	_lut[123] =  1;
	_lut[125] =  1;
	_lut[127] = -1;
	_lut[129] = -7;
	_lut[131] = -1;
	_lut[133] = -1;
	_lut[135] =  1;
	_lut[137] = -3;
	_lut[139] = -1;
	_lut[141] = -1;
	_lut[143] =  1;
	_lut[145] = -1;
	_lut[147] =  1;
	_lut[149] =  1;
	_lut[151] = -1;
	_lut[153] =  3;
	_lut[155] =  1;
	_lut[157] =  1;
	_lut[159] = -1;
	_lut[161] = -3;
	_lut[163] = -1;
	_lut[165] =  3;
	_lut[167] =  1;
	_lut[169] =  1;
	_lut[171] = -1;
	_lut[173] =  3;
	_lut[175] =  1;
	_lut[177] = -1;
	_lut[179] =  1;
	_lut[181] =  1;
	_lut[183] = -1;
	_lut[185] =  3;
	_lut[187] =  1;
	_lut[189] =  1;
	_lut[191] = -1;
	_lut[193] = -3;
	_lut[195] =  3;
	_lut[197] = -1;
	_lut[199] =  1;
	_lut[201] =  1;
	_lut[203] =  3;
	_lut[205] = -1;
	_lut[207] =  1;
	_lut[209] = -1;
	_lut[211] =  1;
	_lut[213] =  1;
	_lut[215] = -1;
	_lut[217] =  3;
	_lut[219] =  1;
	_lut[221] =  1;
	_lut[223] = -1;
	_lut[225] =  1;
	_lut[227] =  3;
	_lut[229] =  3;
	_lut[231] =  1;
	_lut[233] =  5;
	_lut[235] =  3;
	_lut[237] =  3;
	_lut[239] =  1;
	_lut[241] = -1;
	_lut[243] =  1;
	_lut[245] =  1;
	_lut[247] = -1;
	_lut[249] =  3;
	_lut[251] =  1;
	_lut[253] =  1;
	_lut[255] = -1;
}

template <typename ValueType>
bool
Skeletonize<ValueType>::isEulerInvariant(const view_t& patch){

	// calculate Euler characteristic for each octant and sum up
	int eulerCharacteristic = 0;
	unsigned char n;

	// Octant SWU
	n = 1;
	if (patch(0, 2, 2) != 0)
		n |= 128;
	if (patch(1, 2, 2) != 0)
		n |=  64;
	if (patch(0, 2, 1) != 0)
		n |=  32;
	if (patch(1, 2, 1) != 0)
		n |=  16;
	if (patch(0, 1, 2) != 0)
		n |=   8;
	if (patch(1, 1, 2) != 0)
		n |=   4;
	if (patch(0, 1, 1) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	// Octant SEU
	n = 1;
	if (patch(2, 2, 2) != 0)
		n |= 128;
	if (patch(2, 1, 2) != 0)
		n |=  64;
	if (patch(2, 2, 1) != 0)
		n |=  32;
	if (patch(2, 1, 1) != 0)
		n |=  16;
	if (patch(1, 2, 2) != 0)
		n |=   8;
	if (patch(1, 1, 2) != 0)
		n |=   4;
	if (patch(1, 2, 1) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	// Octant NWU
	n = 1;
	if (patch(0, 0, 2) != 0)
		n |= 128;
	if (patch(0, 1, 2) != 0)
		n |=  64;
	if (patch(0, 0, 1) != 0)
		n |=  32;
	if (patch(0, 1, 1) != 0)
		n |=  16;
	if (patch(1, 0, 2) != 0)
		n |=   8;
	if (patch(1, 1, 2) != 0)
		n |=   4;
	if (patch(1, 0, 1) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	// Octant NEU
	n = 1;
	if (patch(2, 0, 2) != 0)
		n |= 128;
	if (patch(2, 1, 2) != 0)
		n |=  64;
	if (patch(1, 0, 2) != 0)
		n |=  32;
	if (patch(1, 1, 2) != 0)
		n |=  16;
	if (patch(2, 0, 1) != 0)
		n |=   8;
	if (patch(2, 1, 1) != 0)
		n |=   4;
	if (patch(1, 0, 1) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	// Octant SWB
	n = 1;
	if (patch(0, 2, 0) != 0)
		n |= 128;
	if (patch(0, 2, 1) != 0)
		n |=  64;
	if (patch(1, 2, 0) != 0)
		n |=  32;
	if (patch(1, 2, 1) != 0)
		n |=  16;
	if (patch(0, 1, 0) != 0)
		n |=   8;
	if (patch(0, 1, 1) != 0)
		n |=   4;
	if (patch(1, 1, 0) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	// Octant SEB
	n = 1;
	if (patch(2, 2, 0) != 0)
		n |= 128;
	if (patch(1, 2, 0) != 0)
		n |=  64;
	if (patch(2, 2, 1) != 0)
		n |=  32;
	if (patch(1, 2, 1) != 0)
		n |=  16;
	if (patch(2, 1, 0) != 0)
		n |=   8;
	if (patch(1, 1, 0) != 0)
		n |=   4;
	if (patch(2, 1, 1) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	// Octant NWB
	n = 1;
	if (patch(0, 0, 0) != 0)
		n |= 128;
	if (patch(0, 0, 1) != 0)
		n |=  64;
	if (patch(0, 1, 0) != 0)
		n |=  32;
	if (patch(0, 1, 1) != 0)
		n |=  16;
	if (patch(1, 0, 0) != 0)
		n |=   8;
	if (patch(1, 0, 1) != 0)
		n |=   4;
	if (patch(1, 1, 0) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	// Octant NEB
	n = 1;
	if (patch(2, 0, 0) != 0)
		n |= 128;
	if (patch(1, 0, 0) != 0)
		n |=  64;
	if (patch(2, 0, 1) != 0)
		n |=  32;
	if (patch(1, 0, 1) != 0)
		n |=  16;
	if (patch(2, 1, 0) != 0)
		n |=   8;
	if (patch(1, 1, 0) != 0)
		n |=   4;
	if (patch(2, 1, 1) != 0)
		n |=   2;

	eulerCharacteristic += _lut[n];

	return (eulerCharacteristic == 0);
}

template <typename ValueType>
bool
Skeletonize<ValueType>::isSimplePoint(const view_t& patch){

	for (int i = 0; i < 13; i++) // i =  0..12 -> _cube[0..12]
		_cube[i] = (patch[i] == 0 ? 0 : 1);
	// i != 13 : ignore center pixel when counting (see [Lee94])
	for(int i = 14; i < 27; i++) // i = 14..26 -> _cube[13..25]
		_cube[i-1] = (patch[i] == 0 ? 0 : 1);

	// set initial label
	int label = 2;

	// for all points in the _cube
	for (int i = 0; i < 26; i++) {

		if (_cube[i] == 1) { // voxel has not been labelled yet

			// start recursion with any octant that contains the point i
			switch (i) {

				case 0:
				case 1:
				case 3:
				case 4:
				case 9:
				case 10:
				case 12:
					labelComponentsAfterRemoval(1, label, _cube);
					break;
				case 2:
				case 5:
				case 11:
				case 13:
					labelComponentsAfterRemoval(2, label, _cube);
					break;
				case 6:
				case 7:
				case 14:
				case 15:
					labelComponentsAfterRemoval(3, label, _cube);
					break;
				case 8:
				case 16:
					labelComponentsAfterRemoval(4, label, _cube);
					break;
				case 17:
				case 18:
				case 20:
				case 21:
					labelComponentsAfterRemoval(5, label, _cube);
					break;
				case 19:
				case 22:
					labelComponentsAfterRemoval(6, label, _cube);
					break;
				case 23:
				case 24:
					labelComponentsAfterRemoval(7, label, _cube);
					break;
				case 25:
					labelComponentsAfterRemoval(8, label, _cube);
					break;
			}

			label++;

			if (label - 2 >= 2)
				// not a simple point
				return false;
		}
	}

	//return label-2; in [Lee94] if the number of connected compontents would be needed
	return true;
}

template <typename ValueType>
void
Skeletonize<ValueType>::labelComponentsAfterRemoval(int octant, int label, int* _cube) {

	// check if there are points in the octant with value 1
	if (octant == 1) {

		// set points in this octant to current label
		// and recurseive labeling of adjacent octants
		if (_cube[0] == 1)
			_cube[0] = label;
		if (_cube[1] == 1) {
		
			_cube[1] = label;
			labelComponentsAfterRemoval(2, label, _cube);
		}
		if (_cube[3] == 1) {
		
			_cube[3] = label;
			labelComponentsAfterRemoval(3, label, _cube);
		}
		if (_cube[4] == 1) {
		
			_cube[4] = label;
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(4, label, _cube);
		}
		if (_cube[9] == 1) {
		
			_cube[9] = label;
			labelComponentsAfterRemoval(5, label, _cube);
		}
		if (_cube[10] == 1) {
		
			_cube[10] = label;
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(5, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
		}
		if (_cube[12] == 1) {
		
			_cube[12] = label;
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(5, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
		}
	}

	if (octant==2) {
	
		if (_cube[1] == 1) {
		
			_cube[1] = label;
			labelComponentsAfterRemoval(1, label, _cube);
		}
		if (_cube[4] == 1) {
		
			_cube[4] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(4, label, _cube);
		}
		if (_cube[10] == 1) {
		
			_cube[10] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(5, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
		}
		if (_cube[2] == 1)
			_cube[2] = label;
		if (_cube[5] == 1) {
		
			_cube[5] = label;
			labelComponentsAfterRemoval(4, label, _cube);
		}
		if (_cube[11] == 1) {
		
			_cube[11] = label;
			labelComponentsAfterRemoval(6, label, _cube);
		}
		if (_cube[13] == 1) {
		
			_cube[13] = label;
			labelComponentsAfterRemoval(4, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
	}

	if (octant==3) {
	
		if (_cube[3] == 1) {
		
			_cube[3] = label;
			labelComponentsAfterRemoval(1, label, _cube);
		}
		if (_cube[4] == 1) {
		
			_cube[4] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(4, label, _cube);
		}
		if (_cube[12] == 1) {
		
			_cube[12] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(5, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
		}
		if (_cube[6] == 1)
			_cube[6] = label;
		if (_cube[7] == 1) {
		
			_cube[7] = label;
			labelComponentsAfterRemoval(4, label, _cube);
		}
		if (_cube[14] == 1) {
		
			_cube[14] = label;
			labelComponentsAfterRemoval(7, label, _cube);
		}
		if (_cube[15] == 1) {
		
			_cube[15] = label;
			labelComponentsAfterRemoval(4, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
	}

	if (octant==4) {
	
		if (_cube[4] == 1) {
		
			_cube[4] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(3, label, _cube);
		}
		if (_cube[5] == 1) {
		
			_cube[5] = label;
			labelComponentsAfterRemoval(2, label, _cube);
		}
		if (_cube[13] == 1) {
		
			_cube[13] = label;
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
		if (_cube[7] == 1) {
		
			_cube[7] = label;
			labelComponentsAfterRemoval(3, label, _cube);
		}
		if (_cube[15] == 1) {
		
			_cube[15] = label;
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
		if (_cube[8] == 1)
			_cube[8] = label;
		if (_cube[16] == 1) {
		
			_cube[16] = label;
			labelComponentsAfterRemoval(8, label, _cube);
		}
	}

	if (octant==5) {
	
		if (_cube[9] == 1) {
		
			_cube[9] = label;
			labelComponentsAfterRemoval(1, label, _cube);
		}
		if (_cube[10] == 1) {
		
			_cube[10] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
		}
		if (_cube[12] == 1) {
		
			_cube[12] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
		}
		if (_cube[17] == 1)
			_cube[17] = label;
		if (_cube[18] == 1) {
		
			_cube[18] = label;
			labelComponentsAfterRemoval(6, label, _cube);
		}
		if (_cube[20] == 1) {
		
			_cube[20] = label;
			labelComponentsAfterRemoval(7, label, _cube);
		}
		if (_cube[21] == 1) {
		
			_cube[21] = label;
			labelComponentsAfterRemoval(6, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
	}

	if (octant==6) {
	
		if (_cube[10] == 1) {
		
			_cube[10] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(5, label, _cube);
		}
		if (_cube[11] == 1) {
		
			_cube[11] = label;
			labelComponentsAfterRemoval(2, label, _cube);
		}
		if (_cube[13] == 1) {
		
			_cube[13] = label;
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(4, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
		if (_cube[18] == 1) {
		
			_cube[18] = label;
			labelComponentsAfterRemoval(5, label, _cube);
		}
		if (_cube[21] == 1) {
		
			_cube[21] = label;
			labelComponentsAfterRemoval(5, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
		if (_cube[19] == 1)
			_cube[19] = label;
		if (_cube[22] == 1) {
		
			_cube[22] = label;
			labelComponentsAfterRemoval(8, label, _cube);
		}
	}

	if (octant==7) {
	
		if (_cube[12] == 1) {
		
			_cube[12] = label;
			labelComponentsAfterRemoval(1, label, _cube);
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(5, label, _cube);
		}
		if (_cube[14] == 1) {
		
			_cube[14] = label;
			labelComponentsAfterRemoval(3, label, _cube);
		}
		if (_cube[15] == 1) {
		
			_cube[15] = label;
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(4, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
		if (_cube[20] == 1) {
		
			_cube[20] = label;
			labelComponentsAfterRemoval(5, label, _cube);
		}
		if (_cube[21] == 1) {
		
			_cube[21] = label;
			labelComponentsAfterRemoval(5, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
			labelComponentsAfterRemoval(8, label, _cube);
		}
		if (_cube[23] == 1)
			_cube[23] = label;
		if (_cube[24] == 1) {
		
			_cube[24] = label;
			labelComponentsAfterRemoval(8, label, _cube);
		}
	}

	if (octant==8) {
	
		if (_cube[13] == 1) {
		
			_cube[13] = label;
			labelComponentsAfterRemoval(2, label, _cube);
			labelComponentsAfterRemoval(4, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
		}
		if (_cube[15] == 1) {
		
			_cube[15] = label;
			labelComponentsAfterRemoval(3, label, _cube);
			labelComponentsAfterRemoval(4, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
		}
		if (_cube[16] == 1) {
		
			_cube[16] = label;
			labelComponentsAfterRemoval(4, label, _cube);
		}
		if (_cube[21] == 1) {
		
			_cube[21] = label;
			labelComponentsAfterRemoval(5, label, _cube);
			labelComponentsAfterRemoval(6, label, _cube);
			labelComponentsAfterRemoval(7, label, _cube);
		}
		if (_cube[22] == 1) {
		
			_cube[22] = label;
			labelComponentsAfterRemoval(6, label, _cube);
		}
		if (_cube[24] == 1) {
		
			_cube[24] = label;
			labelComponentsAfterRemoval(7, label, _cube);
		}
		if (_cube[25] == 1)
			_cube[25] = label;
	}
}

#endif // IMAGEPROCESSING_SKELETONIZE_H__

