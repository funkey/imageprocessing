#ifndef IMAGE_H__
#define IMAGE_H__

#include <vigra/multi_array.hxx>

#include <pipeline/Data.h>
#include "DiscreteVolume.h"

typedef vigra::MultiArray<2, float> array_type;

/**
 * A vigra-compatible image class.
 */
class Image : public pipeline::Data, public DiscreteVolume, public array_type {

public:

	Image(std::string identifier = std::string()) : _identifier(identifier) {};

	Image(size_t width, size_t height, float initialValue = 0.0f, std::string identifier = std::string()) :
		array_type(array_type::difference_type(width, height)),
		_identifier(identifier) {

		init(initialValue);
	}

	using array_type::operator=;

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

	const std::string& getIdentifier() {

		return _identifier;
	}

	void setIdentifiyer(std::string identifier) {

		_identifier = identifier;
	}

private:

	std::string _identifier;
};

#endif // IMAGE_H__
