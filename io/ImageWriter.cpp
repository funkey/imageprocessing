#include <vigra/impex.hxx>

#include <util/Logger.h>
#include "ImageWriter.h"

logger::LogChannel imagewriterlog("imagewriterlog", "[ImageWriter] ");

ImageWriter::ImageWriter(std::string filename) :
	_filename(filename) {

	// let others know about our inputs
	registerInput(_image, "image");
}

void
ImageWriter::write(std::string filename) {

	LOG_DEBUG(imagewriterlog) << "requesting image update" << std::endl;

	updateInputs();

	if (!_image.isSet()) {

		LOG_ERROR(imagewriterlog) << "no input image set" << std::endl;
		return;
	}

	if (filename == "")
		filename = _filename;

	LOG_DEBUG(imagewriterlog) << "attempting to write image" << std::endl;

	// save to file
	vigra::exportImage(vigra::srcImageRange(*_image), vigra::ImageExportInfo(filename.c_str()));

	LOG_DEBUG(imagewriterlog) << "image written" << std::endl;
}
