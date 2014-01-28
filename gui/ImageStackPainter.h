#ifndef IMAGEPROCESSING_GUI_IMAGE_STACK_PAINTER_H__
#define IMAGEPROCESSING_GUI_IMAGE_STACK_PAINTER_H__

#include <gui/Painter.h>
#include <imageprocessing/ImageStack.h>
#include <imageprocessing/gui/ImagePainter.h>

class ImageStackPainter : public gui::Painter {

public:

	/**
	 * Create a new image stack painter.
	 *
	 * @param numImages
	 *             The number of images to show at the same time vertically 
	 *             aligned.
	 * @param gap
	 *             The gap between the images shown.
	 * @param showColored
	 *             Show all images superimposed, each with a different color.
	 */
	ImageStackPainter(unsigned int numImages = 1, double gap = 0.0, bool showColored = true);

	void setImageStack(boost::shared_ptr<ImageStack> stack);

	void setCurrentSection(unsigned int section);

	/**
	 * Overwritten from painter.
	 */
	virtual bool draw(
		const util::rect<double>&  roi,
		const util::point<double>& resolution);

	/**
	 * Enable or disable color mode. In color mode, all images are shown
	 * transparently with a different color on top of each other.
	 */
	void showColored(bool showColored);

	/**
	 * Set the first k numbers to color images in the stack. If there are
	 * more images in the stack, they will be colored randomly.
	 */
	void setColors(std::vector<float> reds, std::vector<float> greens, std::vector<float> blues);

	/**
	 * Set an annotation to show for the image stack.
	 */
	void setAnnotation(double x, double y, std::string annotation) {

		_annotationX = x;
		_annotationY = y;
		_annotation  = annotation;
	}

	/**
	 * Remove an annotation.
	 */
	void unsetAnnotation() {

		_annotation = "";
	}

private:

	// the whole stack
	boost::shared_ptr<ImageStack> _stack;

	// the image painters for the currently visible sections
	std::vector<boost::shared_ptr<gui::ImagePainter<Image> > > _imagePainters;

	// the number of images to show at the same time
	unsigned int _numImages;

	// the section to draw
	unsigned int _section;

	// the height of the images to show
	double _imageHeight;

	// the gap between images to show
	double _gap;

	// show the images of the stack in a colored overlay
	bool _showColored;

	// the colors for the first k images
	std::vector<float> _reds;
	std::vector<float> _greens;
	std::vector<float> _blues;

	double _annotationX;
	double _annotationY;
	std::string _annotation;
};

#endif // IMAGEPROCESSING_GUI_IMAGE_STACK_PAINTER_H__

