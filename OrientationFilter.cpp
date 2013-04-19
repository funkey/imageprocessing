#include <vigra/convolution.hxx>
#include <vigra/functorexpression.hxx>
#include <vigra/multi_pointoperators.hxx>

#include "OrientationFilter.h"

OrientationFilter::OrientationFilter(unsigned int numOrientations) :
		_numOrientations(numOrientations) {

	registerInput(_scale, "scale");
	registerInput(_image, "image");
	registerOutput(_orientations, "orientations");
}


void
OrientationFilter::updateOutputs() {

	int width  = _image->width();
	int height = _image->height();

	_orientationsData.reshape(vigra::MultiArray<2, float>::size_type(width, height));

	_gradX.reshape(vigra::MultiArray<2, float>::size_type(width, height));
	_gradY.reshape(vigra::MultiArray<2, float>::size_type(width, height));

	vigra::gaussianGradient(
			srcImageRange(*_image),
			destImage(_gradX),
			destImage(_gradY),
			*_scale);

	DiscretizeOrientation discretizeOrientations(_numOrientations);

	vigra::combineTwoMultiArrays(
			srcMultiArrayRange(_gradX),
			srcMultiArray(_gradY),
			destMultiArray(_orientationsData),
			discretizeOrientations);

	*_orientations = _orientationsData;
}
