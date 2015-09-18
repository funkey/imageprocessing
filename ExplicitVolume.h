#ifndef IMAGEPROCESSING_EXPLICIT_VOLUME_H__
#define IMAGEPROCESSING_EXPLICIT_VOLUME_H__

#include <vigra/multi_array.hxx>
#include <vigra/functorexpression.hxx>
#include <imageprocessing/Image.h>
#include "DiscreteVolume.h"

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
	ValueType&       operator[](util::point<unsigned int, 3> pos)       { return _data(pos.x(), pos.y(), pos.z()); }
	const ValueType& operator[](util::point<unsigned int, 3> pos) const { return _data(pos.x(), pos.y(), pos.z()); }
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
		image.setOffset(
						getBoundingBox().min().x(),
						getBoundingBox().min().y(),
						getBoundingBox().min().z() + z*getResolutionZ());

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

	/**
	 * Resize this volume and initialize with zeros.
	 */
	void resize(
			unsigned int width,
			unsigned int height,
			unsigned int depth) {

		_data.reshape(vigra::Shape3(width, height, depth));
		setDiscreteBoundingBoxDirty();
	}

	/**
	 * Ensure that the values of this ExplicitVolume are in the range [0,1], if 
	 * they aren't already. If the max exceeds 1 but not 255, values will be 
	 * scaled with 1/255. Otherwise, values will be scaled with 1/max.
	 *
	 * Should the min be negative, a shift of -min will be applied before 
	 * determining the max.
	 */
	void normalize() {

		ValueType min, max;
		ValueType shift = 0;
		data().minmax(&min, &max);
		if (min < 0) {
			shift = -min;
			max  += shift;
		}
		if (min >= 0 && max > 1.0 && max <= 255.0)
			max = 255;
		using namespace vigra::functor;
		if (shift != 0 || max != 1.0)

			vigra::transformMultiArray(
					data(),
					data(),
					(Arg1() + Param(shift))/Param(max));
	}

protected:

	util::box<unsigned int,3> computeDiscreteBoundingBox() const override {

		return util::box<unsigned int,3>(0, 0, 0, _data.shape(0), _data.shape(1), _data.shape(2));
	}

private:

	data_type _data;
};

#endif // IMAGEPROCESSING_EXPLICIT_VOLUME_H__

