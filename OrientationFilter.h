#ifndef IMAGEPROCESSING_ORIENTATION_FILTER_H__
#define IMAGEPROCESSING_ORIENTATION_FILTER_H__

#include <pipeline/all.h>
#include "Image.h"

class OrientationFilter : public pipeline::SimpleProcessNode<> {

public:

	OrientationFilter(unsigned int numOrientations);

private:

	struct DiscretizeOrientation {

		DiscretizeOrientation(int numOrientations) :
			_numOrientations(numOrientations) {}

		float operator()(float gradX, float gradY) const {

			float mag = sqrt(gradX*gradX + gradY*gradY);

			float alpha = asin(abs(gradX)/mag);

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

