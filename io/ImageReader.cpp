#include <fstream>

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

	readImage();
}

void
ImageReader::readImage() {

	if (_filename.find(".feat") == _filename.size() - 5) {

		LOG_DEBUG(imagereaderlog) << "found simple image file, using own importer" << std::endl;

		uint32_t width, height;

		FILE* f = fopen(_filename.c_str(),"r");
		if (!fread(&width,sizeof(uint32_t),1,f))
			return;
		if (!fread(&height,sizeof(uint32_t),1,f))
			return;

		LOG_DEBUG(imagereaderlog) << "reading image of size " << width << "x" << height << std::endl;

		_imageData.reshape(vigra::MultiArray<2, float>::size_type(width, height));

		*_image = _imageData;

		float value;
		for (unsigned int y = 0; y < height; y++) {
			for (unsigned int x = 0; x < width; x++) {
				if (!fread(&value,sizeof(float),1,f))
					return;
				(*_image)(x, y) = value;
			}
		}

		fclose(f);

		return;
	}

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
