#ifndef IMAGEPROCESSING_IMAGE_STACK_VOLUME_ADAPTOR_H__
#define IMAGEPROCESSING_IMAGE_STACK_VOLUME_ADAPTOR_H__

#include "ImageStack.h"

class ImageStackVolumeAdaptor {

public:

	typedef Image::value_type value_type;

	ImageStackVolumeAdaptor(const ImageStack& stack) :
		_stack(stack) {}

	const BoundingBox& getBoundingBox() const { return _stack.getBoundingBox(); }

	float operator()(float x, float y, float z) const {

		unsigned int dx, dy, dz;

		_stack.getDiscreteCoordinates(x, y, z, dx, dy, dz);

		return (*_stack[dz])(dx, dy);
	}

private:

	const ImageStack& _stack;

};

#endif // IMAGEPROCESSING_IMAGE_STACK_VOLUME_ADAPTOR_H__

