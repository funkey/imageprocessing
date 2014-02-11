#ifndef IMAGEPROCESSING_IMAGE_CROP_H__
#define IMAGEPROCESSING_IMAGE_CROP_H__

#include <vigra/multi_pointoperators.hxx>

#include <pipeline/all.h>
#include "Image.h"

class ImageCrop : public pipeline::SimpleProcessNode<> {

public:

	ImageCrop() {

		registerInput(_image, "image");
		registerInput(_x, "x");
		registerInput(_y, "y");
		registerInput(_width, "width");
		registerInput(_height, "height");

		registerOutput(_cropped, "cropped image");
	}

private:

	void updateOutputs() {

		*_cropped = Image::Create(*_width, *_height);

		Image::difference_type upperLeft(*_x, *_y);
		Image::difference_type lowerRight(*_x + *_width, *_y + *_height);

		vigra::copyMultiArray(
				srcMultiArrayRange(_image->subarray(upperLeft, lowerRight)),
				destMultiArrayRange(*_cropped));
	}

	pipeline::Input<Image> _image;
	pipeline::Input<int>   _x;
	pipeline::Input<int>   _y;
	pipeline::Input<int>   _width;
	pipeline::Input<int>   _height;

	pipeline::Output<Image> _cropped;
};

#endif // IMAGEPROCESSING_IMAGE_CROP_H__

