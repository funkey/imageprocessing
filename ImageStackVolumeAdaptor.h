#ifndef IMAGEPROCESSING_IMAGE_STACK_VOLUME_ADAPTOR_H__
#define IMAGEPROCESSING_IMAGE_STACK_VOLUME_ADAPTOR_H__

#include "ImageStack.h"

class ImageStackVolumeAdaptor {

public:

	typedef Image::value_type value_type;

	ImageStackVolumeAdaptor(const ImageStack& stack) :
		_stack(stack) {}

	float width()  const { return _stack.width() *_stack.getResolutionX(); }
	float height() const { return _stack.height()*_stack.getResolutionY(); }
	float depth()  const { return _stack.size()  *_stack.getResolutionZ(); }

	float operator()(float x, float y, float z) const {

		unsigned int section = z/_stack.getResolutionZ();
		unsigned int dx = x/_stack.getResolutionX();
		unsigned int dy = y/_stack.getResolutionY();

		return (*_stack[section])(dx, dy);
	}

private:

	const ImageStack& _stack;

};

#endif // IMAGEPROCESSING_IMAGE_STACK_VOLUME_ADAPTOR_H__

