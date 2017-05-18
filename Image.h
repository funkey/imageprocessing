#ifndef IMAGE_H__
#define IMAGE_H__

#include <vigra/multi_array.hxx>

typedef vigra::MultiArray<2, size_t> array_type;

/**
 * A vigra-compatible image class.
 */
class Image : public array_type {

public:

	Image(std::string identifier = std::string()) : _identifier(identifier) {};

	Image(size_t width, size_t height, size_t initialValue = 0, std::string identifier = std::string()) :
		array_type(array_type::difference_type(width, height)),
		_identifier(identifier) {

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
