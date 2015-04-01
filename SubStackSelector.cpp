#include "SubStackSelector.h"

logger::LogChannel substackselectorlog("substackselectorlog", "[SubStackSelector] ");

SubStackSelector::SubStackSelector(int firstImage, int lastImage) :
	_firstImage(firstImage),
	_lastImage(lastImage) {

	registerInput(_stack, "stack");
	registerOutput(_subStack, "stack");
}

void
SubStackSelector::updateOutputs() {

	if (!_subStack)
		_subStack = new ImageStack();

	_subStack->clear();
	_subStack->setResolution(
			_stack->getResolutionX(),
			_stack->getResolutionY(),
			_stack->getResolutionZ());

	LOG_ALL(substackselectorlog)
			<< "selecting substack from stack of size "
			<< _stack->size() << std::endl;

	LOG_ALL(substackselectorlog)
			<< "first section is " << _firstImage
			<< ", last section is " << _lastImage
			<< std::endl;

	if (_firstImage < 0) {

		LOG_ALL(substackselectorlog)
				<< "first section is negative, will set it to 0"
				<< std::endl;

		_firstImage = 0;
	}

	unsigned int lastImage = (_lastImage <= 0 ? _stack->size() - 1 + _lastImage : _lastImage);

	LOG_ALL(substackselectorlog)
			<< "set last section to " << lastImage
			<< std::endl;

	if (lastImage > _stack->size() - 1) {

		LOG_ERROR(substackselectorlog)
				<< "parameter last section (" << lastImage << ") "
				<< "is bigger than number of images in given stack -- "
				<< "will use " << (_stack->size() - 1) << " instead"
				<< std::endl;

		lastImage = _stack->size() - 1;
	}

	for (unsigned int i = _firstImage; i <= lastImage; i++)
		_subStack->add((*_stack)[i]);

	// set the resolution of the substack
	float resX = _stack->getResolutionX();
	float resY = _stack->getResolutionY();
	float resZ = _stack->getResolutionZ();

	_subStack->setResolution(resX, resY, resZ);

	// set the bounds of the new stack
	_subStack->setBoundingBox(_stack->getBoundingBox());

	float minZ    = _stack->getBoundingBox().minZ;
	float subMinZ = minZ + _firstImage*resZ;
	float subMaxZ = minZ + lastImage*resZ;

	_subStack->getBoundingBox().minZ = subMinZ;
	_subStack->getBoundingBox().maxZ = subMaxZ;
}
