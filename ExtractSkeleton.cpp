#include <boost/timer/timer.hpp>
#include <util/Logger.h>
#include "ExtractSkeleton.h"

logger::LogChannel extractskeletonlog("extractskeletonlog", "[ExtractSkeleton] ");

ExtractSkeleton::ExtractSkeleton() {

	registerInput(_stack, "image stack");
	registerOutput(_skeleton, "skeleton");
}

void
ExtractSkeleton::updateOutputs() {

	unsigned int width  = _stack->width();
	unsigned int height = _stack->height();
	unsigned int depth  = _stack->size();

	vigra::MultiArray<3, int> volume(vigra::Shape3(width, height, depth));

	// fill volume image by image
	for (unsigned int i = 0; i < depth; i++)
		vigra::copyMultiArray(
				*(*_stack)[i],
				volume.bind<2>(i));

	{
		boost::timer::auto_cpu_timer t;

		_skeletonize.skeletonize(volume);
	}

	LOG_USER(extractskeletonlog) << "preparing output image stack" << std::endl;

	prepareSkeletonImage();

	LOG_USER(extractskeletonlog) << "copy skeletons" << std::endl;

	// read back output stack
	for (unsigned int i = 0; i < depth; i++)
		vigra::copyMultiArray(
				volume.bind<2>(i),
				*(*_skeleton)[i]);

	LOG_USER(extractskeletonlog) << "done" << std::endl;
}

void
ExtractSkeleton::prepareSkeletonImage() {

	if (!_skeleton                               ||
		 _skeleton->width()  != _stack->width()  ||
		 _skeleton->height() != _stack->height() ||
		 _skeleton->size()   != _stack->size()) {

		_skeleton = new ImageStack();

		for (unsigned int i = 0; i < _stack->size(); i++)
			_skeleton->add(boost::make_shared<Image>(_stack->width(), _stack->height()));
	}
}
