#ifndef PIPELINE_IMAGE_READER_H__
#define PIPELINE_IMAGE_READER_H__

#include <pipeline/all.h>
#include <imageprocessing/Image.h>

class ImageReader : public pipeline::SimpleProcessNode<> {

public:

	ImageReader(std::string filename);

private:

	void updateOutputs();

	/**
	 * Reads the image.
	 */
	void readImage();

	// the output image
	pipeline::Output<Image> _image;

	// the name of the file to read
	std::string _filename;
};

#endif // PIPELINE_IMAGE_READER_H__

