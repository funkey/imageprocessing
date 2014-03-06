#ifndef IMAGE_VIEW_H__
#define IMAGE_VIEW_H__

#include <pipeline/all.h>
#include <gui/MouseSignals.h>
#include <util/Logger.h>
#include <imageprocessing/Image.h>
#include "ImagePainter.h"

namespace gui {

static logger::LogChannel imageviewlog("imageviewlog", "[ImageView] ");

class ImageView : public pipeline::SimpleProcessNode<> {

public:

	ImageView();

private:

	void updateOutputs();

	void onInputImageSet(const pipeline::InputSet<Image>& signal);

	// input/output
	pipeline::Input<Image>                 _image;
	pipeline::Output<ImagePainter<Image> > _painter;

	// forward signals
	signals::Slot<const SizeChanged>    _sizeChanged;
	signals::Slot<const ContentChanged> _contentChanged;
};

} // namespace gui

#endif // IMAGE_VIEW_H__

