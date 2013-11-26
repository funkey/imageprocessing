#include "ImageStackPainter.h"

static logger::LogChannel imagestackpainterlog("imagestackpainterlog", "[ImageStackPainter] ");

ImageStackPainter::ImageStackPainter(unsigned int numImages, double gap, bool showColored) :
	_numImages(numImages),
	_section(0),
	_gap(gap),
	_showColored(showColored) {

	if (!_showColored)
		for (unsigned int i = 0; i < _numImages; i++)
			_imagePainters.push_back(boost::make_shared<gui::ImagePainter<Image> >());
}

void
ImageStackPainter::setImageStack(boost::shared_ptr<ImageStack> stack) {

	LOG_DEBUG(imagestackpainterlog) << "got a new stack" << std::endl;

	_stack = stack;

	if (_stack && _section >= _stack->size())
		setCurrentSection(0);

	if (_showColored) {

		_imagePainters.clear();

		for (unsigned int i = 0; i < _stack->size(); i++) {

			boost::shared_ptr<gui::ImagePainter<Image> > painter = boost::make_shared<gui::ImagePainter<Image> >();
			painter->setImage((*_stack)[i]);
			painter->setColor(
				(i < _reds.size()   ? _reds[i]   : static_cast<float>(rand())/RAND_MAX),
				(i < _greens.size() ? _greens[i] : static_cast<float>(rand())/RAND_MAX),
				(i < _blues.size()  ? _blues[i]  : static_cast<float>(rand())/RAND_MAX));
			painter->setTransparent(true);
			painter->update();

			_imagePainters.push_back(painter);

			setSize(painter->getSize());
		}
	}
}

void
ImageStackPainter::setCurrentSection(unsigned int section) {

	if (_showColored)
		return;

	if (!_stack || _stack->size() == 0 || _imagePainters.size() == 0)
		return;

	_section = std::min(section, _stack->size() - 1);

	for (unsigned int i = 0; i < _numImages; i++) {

		int imageIndex = std::max(std::min(static_cast<int>(_section) + static_cast<int>(i - _numImages/2), static_cast<int>(_stack->size()) - 1), 0);

		LOG_ALL(imagestackpainterlog) << "index for image " << i << " is " << imageIndex << std::endl;

		_imagePainters[i]->setImage((*_stack)[imageIndex]);
		_imagePainters[i]->update();
	}

	util::rect<double> size = _imagePainters[0]->getSize();

	_imageHeight = size.height();

	size.minY -= _numImages/2*_imageHeight + _numImages*_gap/2;
	size.maxY += (_numImages/2 - (_numImages + 1)%2)*_imageHeight + _numImages*_gap/2;

	setSize(size);

	LOG_DEBUG(imagestackpainterlog) << "current section set to " << _section << std::endl;
}

bool
ImageStackPainter::draw(
		const util::rect<double>&  roi,
		const util::point<double>& resolution) {

	LOG_ALL(imagestackpainterlog) << "redrawing section " << _section << std::endl;

	if (_showColored) {

		for (unsigned int i = 0; i < _stack->size(); i++) {

			_imagePainters[i]->draw(roi, resolution);
		}

	} else {

		for (unsigned int i = 0; i < _numImages; i++) {

			double d = static_cast<int>(i - _numImages/2)*(_imageHeight + _gap);

			glTranslated(0, -d, 0);
			_imagePainters[i]->draw(roi - util::point<double>(static_cast<double>(0), -d), resolution);
			glTranslated(0,  d, 0);
		}
	}

	return false;
}

void
ImageStackPainter::showColored(bool showColored) {

	_showColored = showColored;
}

void
ImageStackPainter::setColors(std::vector<float> reds, std::vector<float> greens, std::vector<float> blues) {

	_reds   = reds;
	_greens = greens;
	_blues  = blues;
}
