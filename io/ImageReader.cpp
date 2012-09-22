#include <vigra/impex.hxx>

#include <util/Logger.h>
#include "ImageReader.h"

logger::LogChannel imagereaderlog("imagereaderlog", "[ImageReader] ");

ImageReader::ImageReader(std::string filename) :
	_filename(filename) {

	// let others know about our output
	registerOutput(_image, "image");
}

void
ImageReader::updateOutputs() {

	LOG_DEBUG(imagereaderlog) << "need to (re)load image..." << std::endl;
	readImage();
}

void
ImageReader::readImage() {

	// get information about the image to read
	vigra::ImageImportInfo info(_filename.c_str());

	// abort if image is not grayscale
	if (!info.isGrayscale()) {

		LOG_ERROR(imagereaderlog) << _filename << " is not a gray-scale image!" << std::endl;
		return;
	}
	_imageData.reshape(vigra::MultiArray<2, float>::size_type(info.width(), info.height()));

	// resize image
	*_image = _imageData;

	// read image
	importImage(info, vigra::destImage(*_image));

	// scale image to [0..1]
	vigra::transformImage(
			vigra::srcImageRange(*_image),
			vigra::destImage(*_image),
			vigra::linearIntensityTransform<float>(1.0/255.0));
}
