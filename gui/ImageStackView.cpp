#include "ImageStackView.h"
#include <util/Logger.h>

static logger::LogChannel imagestackviewlog("imagestackviewlog", "[ImageStackView] ");

ImageStackView::ImageStackView(unsigned int numImages, double gap, bool showColored) :
	_painter(boost::make_shared<ImageStackPainter>(numImages, gap, showColored)),
	_section(boost::make_shared<int>(0)) {

	registerInput(_stack, "imagestack");
	registerOutput(_painter, "painter");
	registerOutput(_currentImage, "current image");
	registerOutput(_section, "section");
	registerOutput(_clickX, "click x");
	registerOutput(_clickY, "click y");

	_painter.registerForwardSlot(_sizeChanged);
	_painter.registerForwardSlot(_contentChanged);
	_painter.registerForwardCallback(&ImageStackView::onKeyDown, this);
	_painter.registerForwardCallback(&ImageStackView::onButtonDown, this);
	_painter.registerForwardCallback(&ImageStackView::onMouseMove, this);
}

void
ImageStackView::setColors(std::vector<float> reds, std::vector<float> greens, std::vector<float> blues) {

	_painter->setColors(reds, greens, blues);
}

void
ImageStackView::updateOutputs() {

	util::rect<double> oldSize = _painter->getSize();

	_painter->setImageStack(_stack);
	_painter->setCurrentSection(*_section);

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
	_currentImage->reshape(_stack->width(), _stack->height());

	// copy current image data
	*_currentImage = *(*_stack)[*_section];

	// set last known mouse down position
	*_clickX = _mouseDownX;
	*_clickY = _mouseDownY;
}

void
ImageStackView::onKeyDown(gui::KeyDown& signal) {

	LOG_ALL(imagestackviewlog) << "got a key down event" << std::endl;

	if (signal.key == gui::keys::A) {

		*_section = std::max(0, *_section - 1);

		LOG_ALL(imagestackviewlog) << "setting current section to " << *_section << std::endl;

		setDirty(_painter);
		setDirty(_currentImage);
		setDirty(_section);
	}

	if (signal.key == gui::keys::D) {

		*_section = std::min((int)_stack->size() - 1, *_section + 1);

		LOG_ALL(imagestackviewlog) << "setting current section to " << *_section << std::endl;

		setDirty(_painter);
		setDirty(_currentImage);
		setDirty(_section);
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

void
ImageStackView::onMouseMove(gui::MouseMove& signal) {

	LOG_ALL(imagestackviewlog) << "got a mouse move event" << std::endl;

	int x = signal.position.x;
	int y = signal.position.y;

	if (x >= 0 && x < (int)_stack->width() && y >= 0 && y < (int)_stack->height())
		_painter->setAnnotation(
				x, y,
				boost::lexical_cast<std::string>(x) + ", " +
				boost::lexical_cast<std::string>(y) + ", " +
				boost::lexical_cast<std::string>(*_section));
	else
		_painter->unsetAnnotation();

	_contentChanged();
}
