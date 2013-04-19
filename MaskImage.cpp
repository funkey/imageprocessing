#include <vigra/functorexpression.hxx>

#include "MaskImage.h"

MaskImage::MaskImage(float maskValue) :
		_maskValue(maskValue) {

	registerInput(_image, "image");
	registerInput(_mask, "mask");
	registerOutput(_masked, "masked");
}

void
MaskImage::updateOutputs() {

	int width  = _image->width();
	int height = _image->height();

	_data.reshape(vigra::MultiArray<2, float>::size_type(width, height));

	vigra::combineTwoMultiArrays(
			srcMultiArrayRange(*_image),
			srcMultiArray(*_mask),
			destMultiArray(_data),
			ifThenElse(vigra::functor::Arg2() == vigra::functor::Param(1), vigra::functor::Arg1(), vigra::functor::Param(_maskValue)));

	*_masked = _data;
}
