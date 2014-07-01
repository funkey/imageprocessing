#include <fstream>

#include <vigra/impex.hxx>

#include <util/Logger.h>
#include <util/exceptions.h>
#include "ImageReader.h"

logger::LogChannel imagereaderlog("imagereaderlog", "[ImageReader] ");

ImageReader::ImageReader(std::string filename) :
	_filename(filename) {

	// let others know about our output
	registerOutput(_image, "image");
}

void
ImageReader::updateOutputs() {

	readImage();
}

void
ImageReader::readImage() {

	// get information about the image to read
	vigra::ImageImportInfo info(_filename.c_str());

	// abort if image is not grayscale
	if (!info.isGrayscale()) {

		UTIL_THROW_EXCEPTION(
				IOError,
				_filename << " is not a gray-scale image!");
	}

	// allocate image
	_image = new Image(info.width(), info.height());

	try {

		// read image
		importImage(info, vigra::destImage(*_image));

	} catch (vigra::PostconditionViolation& e) {

		UTIL_THROW_EXCEPTION(
				IOError,
				"error reading " << _filename << ": " << e.what());
	}

	if (strcmp(info.getPixelType(), "FLOAT") == 0)
		return;

	// scale image to [0..1]

	float factor;
	if (strcmp(info.getPixelType(), "UINT8") == 0)
		factor = 255.0;
	else if (strcmp(info.getPixelType(), "INT16") == 0)
		factor = 511.0;
	else {

		factor = 1.0;
		LOG_ERROR(imagereaderlog) << _filename << " has a unsupported pixel format: " << info.getPixelType() << std::endl;
	}

	vigra::transformImage(
			vigra::srcImageRange(*_image),
			vigra::destImage(*_image),
			vigra::linearIntensityTransform<float>(1.0/factor));
}
