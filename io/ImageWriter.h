#ifndef PIPELINE_IMAGE_WRITER_H__
#define PIPELINE_IMAGE_WRITER_H__

#include <pipeline/all.h>
#include <signals/Slot.h>
#include <imageprocessing/Image.h>

class ImageWriter : public pipeline::SimpleProcessNode<> {

public:

	ImageWriter(std::string filename);

	/**
	 * Initiate writing of the image that is connected to this writer.
	 */
	void write(std::string filename = "");

private:

	void updateOutputs() {};

	// the input image
	pipeline::Input<Image> _image;

	// the name of the file to write to
	std::string _filename;
};

#endif // PIPELINE_IMAGE_WRITER_H__

