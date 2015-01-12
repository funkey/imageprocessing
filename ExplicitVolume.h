#ifndef IMAGEPROCESSING_EXPLICIT_VOLUME_H__
#define IMAGEPROCESSING_EXPLICIT_VOLUME_H__

#include <vigra/multi_array.hxx>
#include "DiscreteVolume.h"

/**
 * Explicit representation of a discrete volume as a vigra multi-array.
 */
template <typename ValueType>
class ExplicitVolume : public DiscreteVolume {

public:

	typedef vigra::MultiArray<3, ValueType> data_type;

	/**
	 * Create an empty volume.
	 */
	ExplicitVolume() {}

	/**
	 * Create a new explicit volume of the given size.
	 */
	ExplicitVolume(
			unsigned int width,
			unsigned int height,
			unsigned int depth) :
		_data(vigra::Shape3(width, height, depth)) {}

	/**
	 * Create a new explicit volume of the given size. Initialize all voxels 
	 * with the given value.
	 */
	ExplicitVolume(
			unsigned int width,
			unsigned int height,
			unsigned int depth,
			const ValueType& value) :
		_data(vigra::Shape3(width, height, depth), value) {}

	/**
	 * Get access to the vigra multi-array that contains the data of this 
	 * volume.
	 */
	data_type&       data()       { return _data; }
	const data_type& data() const { return _data; }

private:

	data_type _data;
};

#endif // IMAGEPROCESSING_EXPLICIT_VOLUME_H__

