#ifndef PIPELINE_IMAGE_WRITER_H__
#define PIPELINE_IMAGE_WRITER_H__

#include <pipeline/signals/Update.h>
#include <pipeline/ProcessNode.h>
#include <signals/Slot.h>
#include <imageprocessing/Image.h>

class ImageWriter : public pipeline::ProcessNode {

public:

	ImageWriter(std::string filename);

	/**
	 * Initiate writing of the image that is connected to this writer.
	 */
	void write();

private:

	// the input image
	pipeline::Input<Image> _image;

	// update signal slot
	signals::Slot<const pipeline::Update> _update;

	// the name of the file to write to
	std::string _filename;
};

#endif // PIPELINE_IMAGE_WRITER_H__

