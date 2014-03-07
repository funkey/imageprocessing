#include "ImageView.h"

namespace gui {

ImageView::ImageView() {

	registerInput(_image, "image");
	registerOutput(_painter, "painter");

	_image.registerCallback(&ImageView::onInputImageSet, this);
	_painter.registerSlot(_contentChanged);
	_painter.registerSlot(_sizeChanged);
}

void
ImageView::onInputImageSet(const pipeline::InputSet<Image>& /*signal*/) {

	if (!_painter)
		_painter = new ImagePainter<Image>();

	LOG_ALL(imageviewlog) << "got a new input image -- sending SizeChanged" << std::endl;

	_sizeChanged();
}

void
ImageView::updateOutputs() {

	LOG_ALL(imageviewlog) << "updating my painter" << std::endl;

	util::rect<double> oldSize = _painter->getSize();

	LOG_ALL(imageviewlog) << "old size is " << oldSize << std::endl;

	// update the painter
	_painter->setImage(_image);

	util::rect<double> newSize = _painter->getSize();

	LOG_ALL(imageviewlog) << "new size is " << newSize << std::endl;

	if (oldSize == newSize) {

		LOG_ALL(imageviewlog) << "image size did not change -- sending ContentChanged" << std::endl;

		_contentChanged();

	} else {

		LOG_ALL(imageviewlog) << "image size did change -- sending SizeChanged" << std::endl;

		_sizeChanged();
	}
}

} // namespace gui
