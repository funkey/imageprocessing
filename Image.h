#ifndef IMAGE_H__
#define IMAGE_H__

#include <vigra/multi_array.hxx>

#include <pipeline/Data.h>

typedef vigra::MultiArray<2, float> array_type;

/**
 * A vigra-compatible image class.
 */
class Image : public pipeline::Data, public array_type {

public:

	Image() {};

	Image(size_t width, size_t height, float initialValue = 0.0f) :
		array_type(array_type::difference_type(width, height)) {

		init(initialValue);
	}

	/**
	 * The width of the image.
	 */
	array_type::difference_type_1 width() const { return size(0); }

	/**
	 * The height of the image.
	 */
	array_type::difference_type_1 height() const { return size(1); }

	/**
	 * Reshape the image.
	 */
	using array_type::reshape;

	/**
	 * Reshape the image.
	 */
	void reshape(size_t width, size_t height) {

		reshape(array_type::difference_type(width, height));
	}
};

#endif // IMAGE_H__
