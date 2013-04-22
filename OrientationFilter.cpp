#include <cmath>

#include <vigra/convolution.hxx>
#include <vigra/functorexpression.hxx>
#include <vigra/multi_pointoperators.hxx>

#include "OrientationFilter.h"

logger::LogChannel orientationfilterlog("orientationfilterlog", "[OrientationFilter] ");

OrientationFilter::OrientationFilter(unsigned int numOrientations) :
		_numOrientations(numOrientations) {

	registerInput(_scale, "scale");
	registerInput(_image, "image");
	registerOutput(_orientations, "orientations");
}


void
OrientationFilter::updateOutputs() {

	LOG_DEBUG(orientationfilterlog)
			<< "updating orientations with scale " << (*_scale)
			<< " and " << _numOrientations << " orientations"
			<< std::endl;

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

float
OrientationFilter::DiscretizeOrientation::operator()(float gradX, float gradY) const {

	float mag = sqrt(gradX*gradX + gradY*gradY);

	float alpha = std::asin(std::abs(gradX)/mag);

	// pointing upwards
	if (gradY < 0) {

		// pointing left
		if (gradX < 0)
			alpha = 2*M_PI - alpha;

	// pointing downwards
	} else {

		// pointing right
		if (gradX >= 0) {

			alpha = M_PI - alpha;

		// pointing left
		} else {

			alpha = M_PI + alpha;
		}
	}

	float segmentAngle = M_PI/_numOrientations;

	// relevant half-circle for orientation starts at -segmentAngle/2
	alpha = (alpha + segmentAngle/2);

	// modulo M_PI
	while (alpha > M_PI)
		alpha -= M_PI;

	int orientation = static_cast<int>(alpha/segmentAngle) % _numOrientations;

	return 1.0/_numOrientations*orientation;
}
