#ifndef IMAGE_H__
#define IMAGE_H__

#include <vector>
#include <boost/shared_ptr.hpp>

#include <vigra/multi_array.hxx>

#include <pipeline/Data.h>

typedef vigra::MultiArrayView<2, float> array_type;

/**
 * A vigra-compatible image class.
 */
class Image : public pipeline::Data, public array_type {

public:

	Image() :
		array_type() {};

	Image(size_t width, size_t height, boost::shared_ptr<std::vector<float> > data) :
		array_type(array_type::difference_type(width, height), &(*data)[0]),
		_data(data) {};

	using array_type::operator=;

	array_type::difference_type_1 width() const { return size(0); }
	array_type::difference_type_1 height() const { return size(1); }

private:

	boost::shared_ptr<std::vector<float> > _data;
};

#endif // IMAGE_H__
