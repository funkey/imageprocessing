#ifndef IMAGEPROCESSING_MASK_IMAGE_H__
#define IMAGEPROCESSING_MASK_IMAGE_H__

#include <vigra/multi_pointoperators.hxx>

#include <pipeline/all.h>
#include "Image.h"

class MaskImage : public pipeline::SimpleProcessNode<> {

public:

	MaskImage(float maskValue = 0.0);

private:

	void updateOutputs();

	pipeline::Input<Image> _image;
	pipeline::Input<Image>  _mask;
	pipeline::Output<Image> _masked;

	float _maskValue;
};

#endif // IMAGEPROCESSING_MASK_IMAGE_H__

