#ifndef IMAGEPROCESSING_EXPLICIT_VOLUME_H__
#define IMAGEPROCESSING_EXPLICIT_VOLUME_H__

#include <cmath>
#include <vigra/multi_array.hxx>
#include <vigra/functorexpression.hxx>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <imageprocessing/Image.h>
#include <util/exceptions.h>
#include <util/typename.h>
#include "DiscreteVolume.h"

template <typename T>
struct numpy_type_traits {};

template <>
struct numpy_type_traits<int> {

	static char getNumpyType() { return NPY_INT32; }
};

template <>
struct numpy_type_traits<float> {

	static char getNumpyType() { return NPY_FLOAT32; }
};

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

	/**
	 * Reverse the order of the axises.
	 */
	void transpose() {

		_data = _data.transpose();

		auto res = getResolution();
		auto off = getOffset();

		setResolution(res.z(), res.y(), res.x());
		setOffset(off.z(), off.y(), off.x());
		setBoundingBoxDirty();
	}

	/**
	 * Cut a subvolume of this ExplicitVolume<ValueType>.
	 *
	 * @param boundingBox
	 *              The bounding box of the requested subvolume. The target gets 
	 *              resized to be at least that large, but might be larger to 
	 *              fit all the voxels that are intersecting the requested 
	 *              subvolume.
	 * @param target
	 *              An explicit volume to fill.
	 */
	void cut(const util::box<float, 3>& boundingBox, ExplicitVolume<ValueType>& target) {

		util::box<float, 3> intersection = boundingBox.intersection(getBoundingBox());

		if (intersection.isZero()) {

			target = ExplicitVolume<ValueType>();
			return;
		}

		// the discrete offset of the requested region in this volume
		util::point<unsigned int, 3> offset =
				(intersection.min() - getBoundingBox().min())/
				getResolution();

		// the discrete size of the requested region
		util::point<unsigned int, 3> size(
				std::ceil(intersection.width() /getResolution().x()),
				std::ceil(intersection.height()/getResolution().y()),
				std::ceil(intersection.depth() /getResolution().z()));

		target = ExplicitVolume<ValueType>(size.x(), size.y(), size.z());
		target.setResolution(getResolution());
		target.setOffset(getOffset() + offset*getResolution());

		typedef typename data_type::difference_type Shape;
		target.data() = data().subarray(
				Shape(
						offset.x(),
						offset.y(),
						offset.z()),
				Shape(
						offset.x() + size.x(),
						offset.y() + size.y(),
						offset.z() + size.z()));
	}

protected:

	util::box<unsigned int,3> computeDiscreteBoundingBox() const override {

		return util::box<unsigned int,3>(0, 0, 0, _data.shape(0), _data.shape(1), _data.shape(2));
	}

private:

	data_type _data;
};

template <typename ValueType>
ExplicitVolume<ValueType>
volumeFromNumpyArray(PyObject* a) {

	PyArrayObject* array = (PyArrayObject*)PyArray_FromAny(
			a,
			PyArray_DescrFromType(numpy_type_traits<ValueType>::getNumpyType()),
			0, 0, // min and max dimension, we check that later
			0,    // requirements
			0);

	if (array == NULL)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given numpy array is not of type " << typeName(ValueType()));

	int dims = PyArray_NDIM(array);
	if (dims != 3)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"only arrays of dimensions 3 are supported.");

	size_t d = PyArray_DIM(array, 0);
	size_t h = PyArray_DIM(array, 1);
	size_t w = PyArray_DIM(array, 2);

	auto volume = ExplicitVolume<ValueType>(w, h, d);

	for (size_t z = 0; z < d; z++)
	for (size_t y = 0; y < h; y++)
	for (size_t x = 0; x < w; x++)
		volume(x,y,z) = *static_cast<ValueType*>(PyArray_GETPTR3(array, z, y, x));

	return volume;
}

#endif // IMAGEPROCESSING_EXPLICIT_VOLUME_H__

