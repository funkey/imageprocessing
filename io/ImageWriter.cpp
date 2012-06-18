#include <vigra/impex.hxx>

#include <util/Logger.h>
#include "ImageWriter.h"

logger::LogChannel imagewriterlog("imagewriterlog", "[ImageWriter] ");

ImageWriter::ImageWriter(std::string filename) :
	_filename(filename) {

	// let others know about our inputs
	registerInput(_image, "image");

	// send signals backwards from this input
	_image.registerBackwardSlot(_update);
}

void
ImageWriter::write() {

	LOG_DEBUG(imagewriterlog) << "attempting to write image" << std::endl;

	if (!_image) {

		LOG_ERROR(imagewriterlog) << "no input image set" << std::endl;
		return;
	}

	LOG_DEBUG(imagewriterlog) << "requesting image update" << std::endl;

	// request an update of the image
	_update(pipeline::Update());

	// save to file
	vigra::exportImage(vigra::srcImageRange(*_image), vigra::ImageExportInfo(_filename.c_str()));

	LOG_DEBUG(imagewriterlog) << "image written" << std::endl;
}
