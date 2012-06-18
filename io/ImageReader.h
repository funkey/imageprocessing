#ifndef PIPELINE_IMAGE_READER_H__
#define PIPELINE_IMAGE_READER_H__

#include <pipeline/signals/Update.h>
#include <pipeline/signals/Updated.h>
#include <pipeline/ProcessNode.h>
#include <imageprocessing/Image.h>

class ImageReader : public pipeline::ProcessNode {

public:

	ImageReader(std::string filename);

private:

	/**
	 * Update signal callback.
	 */
	void onUpdate(const pipeline::Update& update);

	/**
	 * Reads the image.
	 */
	void readImage();

	// forward signals

	signals::Slot<const pipeline::Updated> _updated;

	// the output image
	pipeline::Output<Image> _image;

	// the name of the file to read
	std::string _filename;

	// the image data
	vigra::MultiArray<2, float> _imageData;

	// indicate that the image was not read or changed
	bool _dirty;
};

#endif // PIPELINE_IMAGE_READER_H__

