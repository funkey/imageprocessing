#ifndef IMAGEPROCESSING_ORIENTATION_FILTER_H__
#define IMAGEPROCESSING_ORIENTATION_FILTER_H__

#include <pipeline/all.h>
#include "Image.h"

/**
 * Performs a Gaussian smoothing to find the principal orientation for every 
 * pixel. The orientations are discretized (numOrientations in constructor) and 
 * given back in an orientation map, where the value of orientation i is the 
 * highest value of the i-th equally sized interval in the range [0, 1].  
 * Orientation 0 is vertical, subsequent orientations follow clock-wise.
 */
class OrientationFilter : public pipeline::SimpleProcessNode<> {

public:

	OrientationFilter(unsigned int numOrientations);

private:

	struct DiscretizeOrientation {

		DiscretizeOrientation(int numOrientations) :
			_numOrientations(numOrientations) {}

		float operator()(float gradX, float gradY) const;

		int _numOrientations;
	};

	void updateOutputs();

	pipeline::Input<double> _scale;
	pipeline::Input<Image>  _image;
	pipeline::Output<Image> _orientations;

	int _numOrientations;

	// the data for the orientation image
	vigra::MultiArray<2, float> _gradX;
	vigra::MultiArray<2, float> _gradY;
	vigra::MultiArray<2, float> _orientationsData;
};


#endif // IMAGEPROCESSING_ORIENTATION_FILTER_H__

