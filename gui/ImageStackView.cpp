#include "ImageStackView.h"
#include <util/Logger.h>

static logger::LogChannel imagestackviewlog("imagestackviewlog", "[ImageStackView] ");

ImageStackView::ImageStackView(unsigned int numImages, bool showColored) :
	_painter(boost::make_shared<ImageStackPainter>(numImages, showColored)),
	_section(0) {

	registerInput(_stack, "imagestack");
	registerOutput(_painter, "painter");
	registerOutput(_currentImage, "current image");
	registerOutput(_clickX, "click x");
	registerOutput(_clickY, "click y");

	_painter.registerForwardSlot(_sizeChanged);
	_painter.registerForwardSlot(_contentChanged);
	_painter.registerForwardCallback(&ImageStackView::onKeyDown, this);
	_painter.registerForwardCallback(&ImageStackView::onButtonDown, this);
}

void
ImageStackView::setColors(std::vector<float> reds, std::vector<float> greens, std::vector<float> blues) {

	_painter->setColors(reds, greens, blues);
}

void
ImageStackView::updateOutputs() {

	util::rect<double> oldSize = _painter->getSize();

	_painter->setImageStack(_stack);
	_painter->setCurrentSection(_section);

	util::rect<double> newSize = _painter->getSize();

	if (oldSize == newSize) {

		LOG_ALL(imagestackviewlog) << "image size did not change -- sending ContentChanged" << std::endl;

		_contentChanged();

	} else {

		LOG_ALL(imagestackviewlog) << "image size did change -- sending SizeChanged" << std::endl;

		_sizeChanged();
	}

	if (_stack->size() == 0)
		return;

	// prepare current image data
	_currentImageData.reshape(vigra::MultiArray<2, float>::size_type(_stack->width(), _stack->height()));

	// copy current image data
	_currentImageData = *(*_stack)[_section];

	// set content of output
	*_currentImage = _currentImageData;

	// set last known mouse down position
	*_clickX = _mouseDownX;
	*_clickY = _mouseDownY;
}

void
ImageStackView::onKeyDown(gui::KeyDown& signal) {

	LOG_ALL(imagestackviewlog) << "got a key down event" << std::endl;

	if (signal.key == gui::keys::A) {

		_section = std::max(0, _section - 1);

		LOG_ALL(imagestackviewlog) << "setting current section to " << _section << std::endl;

		setDirty(_painter);
		setDirty(_currentImage);
	}

	if (signal.key == gui::keys::D) {

		_section = std::min((int)_stack->size() - 1, _section + 1);

		LOG_ALL(imagestackviewlog) << "setting current section to " << _section << std::endl;

		setDirty(_painter);
		setDirty(_currentImage);
	}
}

void
ImageStackView::onButtonDown(gui::MouseDown& signal) {

	LOG_ALL(imagestackviewlog) << "got a mouse down event" << std::endl;

	if (signal.button == gui::buttons::Left && signal.modifiers == 0) {

		_mouseDownX = signal.position.x;
		_mouseDownY = signal.position.y;

		LOG_ALL(imagestackviewlog) << "setting click position to (" << _mouseDownX << ", " << _mouseDownY << ")" << std::endl;

		setDirty(_clickX);
		setDirty(_clickY);
	}
}
