#include <vigra/transformimage.hxx>

#include "MedianFilter.h"

MedianFilter::MedianFilter() {

	registerInput(_radius, "radius");
	registerInput(_image, "image");
	registerOutput(_filtered, "filtered");
}


void
MedianFilter::updateOutputs() {

	int width  = _image->width();
	int height = _image->height();

	vigra::MultiArray<2, int> image;
	vigra::MultiArray<2, int> filtered;

	image.reshape(vigra::MultiArray<2, int>::size_type(width, height));
	filtered.reshape(vigra::MultiArray<2, int>::size_type(width, height));
	_filtered->reshape(width, height);

	vigra::transformImage(
			srcImageRange(*_image),
			destImage(image),
			vigra::functor::Arg1()*vigra::functor::Param(255.0));

	vigra::discMedian(
			srcImageRange(image),
			destImage(filtered),
			*_radius);

	vigra::transformImage(
			srcImageRange(filtered),
			destImage(*_filtered),
			vigra::functor::Arg1()/vigra::functor::Param(255.0));
}
