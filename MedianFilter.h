#ifndef IMAGEPROCESSING_MEDIAN_FILTER_H__
#define IMAGEPROCESSING_MEDIAN_FILTER_H__

#include <vigra/functorexpression.hxx>
#include <vigra/flatmorphology.hxx>

#include <pipeline/all.h>
#include "Image.h"

class MedianFilter : public pipeline::SimpleProcessNode<> {

public:

	MedianFilter();

private:

	void updateOutputs();

	pipeline::Input<int>    _radius;
	pipeline::Input<Image>  _image;
	pipeline::Output<Image> _filtered;
};

#endif // IMAGEPROCESSING_MEDIAN_FILTER_H__

