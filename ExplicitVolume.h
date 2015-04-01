#ifndef IMAGEPROCESSING_EXPLICIT_VOLUME_H__
#define IMAGEPROCESSING_EXPLICIT_VOLUME_H__

#include <vigra/multi_array.hxx>
#include "DiscreteVolume.h"
#include "ImageStack.h"

/**
 * Explicit representation of a discrete volume as a vigra multi-array.
 */
template <typename ValueType>
class ExplicitVolume : public DiscreteVolume {

public:

	typedef ValueType                       value_type;
	typedef vigra::MultiArray<3, ValueType> data_type;

	/**
	 * Create an empty volume.
	 */
	ExplicitVolume() {}

	/**
	 * Create a volume from another (type compatible) volume.
	 */
	template <typename T>
	ExplicitVolume(const ExplicitVolume<T>& other) :
		DiscreteVolume(other),
		_data(other.data()) {}

	/**
	 * Create a volume from an image stack.
	 */
	ExplicitVolume(const ImageStack& stack) :
		DiscreteVolume(stack),
		_data(vigra::Shape3(stack.width(), stack.height(), stack.size())) {

		// fill volume image by image
		for (unsigned int i = 0; i < stack.size(); i++)
			vigra::copyMultiArray(
					*stack[i],
					_data.template bind<2>(i));
	}

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
	 * Pixel access.
	 */
	ValueType&       operator[](vigra::Shape3 pos)       { return _data[pos]; }
	const ValueType& operator[](vigra::Shape3 pos) const { return _data[pos]; }
	ValueType&       operator()(unsigned int x, unsigned int y, unsigned int z)       { return _data(x, y, z); }
	const ValueType& operator()(unsigned int x, unsigned int y, unsigned int z) const { return _data(x, y, z); }

	/**
	 * 2D z-slice access.
	 */
	Image slice(int z) {

		vigra::MultiArrayView<2, ValueType> slice = _data.template bind<2>(z);

		Image image;
		image = slice;
		image.setResolution(
				getResolutionX(),
				getResolutionY(),
				getResolutionZ());
		image.setBoundingBox(
				util::box<float>(
						getBoundingBox().minX,
						getBoundingBox().minY,
						getBoundingBox().minZ + z*getResolutionZ(),
						getBoundingBox().maxX,
						getBoundingBox().maxY,
						getBoundingBox().minZ + (z+1)*getResolutionZ()));

		return image;
	}

	/**
	 * Get access to the vigra multi-array that contains the data of this 
	 * volume.
	 */
	data_type&       data()       { return _data; }
	const data_type& data() const { return _data; }

	unsigned int width()  const { return _data.shape()[0]; }
	unsigned int height() const { return _data.shape()[1]; }
	unsigned int depth()  const { return _data.shape()[2]; }

private:

	data_type _data;
};

#endif // IMAGEPROCESSING_EXPLICIT_VOLUME_H__

