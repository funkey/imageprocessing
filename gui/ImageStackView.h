#ifndef IMAGEPROCESSING_GUI_IMAGE_STACK_VIEW_H__
#define IMAGEPROCESSING_GUI_IMAGE_STACK_VIEW_H__

#include <pipeline/all.h>
#include <imageprocessing/ImageStack.h>
#include <gui/Keys.h>
#include <gui/GuiSignals.h>
#include <gui/KeySignals.h>
#include <gui/MouseSignals.h>
#include "ImageStackPainter.h"

class ImageStackView : public pipeline::SimpleProcessNode<> {

public:

	/**
	 * Create a new image stack view.
	 *
	 * @param numImages
	 *             The number of images to show at the same time vertically 
	 *             aligned.
	 * @param gap
	 *             The gap between the images shown.
	 * @param showColored
	 *             Show all images superimposed, each with a different color.
	 */
	ImageStackView(unsigned int numImages = 1, double gap = 0.0, bool showColored = false);

	/**
	 * Set the first k numbers to color images in the stack. If there are
	 * more images in the stack, they will be colored randomly.
	 */
	void setColors(std::vector<float> reds, std::vector<float> greens, std::vector<float> blues);
private:

	void updateOutputs();

	void onKeyDown(gui::KeyDown& signal);
	void onButtonDown(gui::MouseDown& signal);

	pipeline::Input<ImageStack>         _stack;
	pipeline::Output<ImageStackPainter> _painter;
	pipeline::Output<Image>             _currentImage;
	pipeline::Output<int>               _section;
	pipeline::Output<float>             _clickX;
	pipeline::Output<float>             _clickY;

	signals::Slot<gui::SizeChanged>    _sizeChanged;
	signals::Slot<gui::ContentChanged> _contentChanged;

	// copy of the currently visible image
	vigra::MultiArray<2, float> _currentImageData;

	// the last mouse down position
	float _mouseDownX;
	float _mouseDownY;
};

#endif // IMAGEPROCESSING_GUI_IMAGE_STACK_VIEW_H__

