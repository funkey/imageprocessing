#include <util/Logger.h>
#include "ImageStackHdf5Reader.h"

#ifdef HAVE_HDF5

logger::LogChannel imagestackhdf5readerlog("imagestackhdf5readerlog", "[ImageStackHdf5Reader] ");

ImageStackHdf5Reader::ImageStackHdf5Reader(
		const std::string& filename,
		const std::string& groupname,
		const std::string& datasetname,
		unsigned int originSection,
		unsigned int targetSection,
		unsigned int minX,
		unsigned int maxX,
		unsigned int minY,
		unsigned int maxY) :
	_filename(filename),
	_groupname(groupname),
	_datasetname(datasetname),
	_minX(minX),
	_maxX(maxX),
	_minY(minY),
	_maxY(maxY),
	_originSection(originSection),
	_targetSection(targetSection) {

	registerOutput(_stack, "stack");
}

void
ImageStackHdf5Reader::updateOutputs() {

	readImages();
}

void
ImageStackHdf5Reader::readImages() {

	// open file
	H5::H5File file(_filename, H5F_ACC_RDONLY);

	// clear the previous stack
	_stack->clear();

	H5::Group group;
	std::vector<unsigned char> data;

	for (unsigned int i = _originSection; i <= _targetSection; i++) {

		std::stringstream finalgroupname;

		finalgroupname << _groupname << "/" << i;

		LOG_DEBUG(imagestackhdf5readerlog) << "readImage from group " << finalgroupname.str().c_str() << std::endl;

		// open group
		group = file.openGroup(finalgroupname.str().c_str());

		data = hdf5::read<unsigned char>(group, _datasetname);

		// get the dimensions
		std::vector<size_t> dims = hdf5::dimensions(group, _datasetname);

		unsigned int height    = dims[0];
		unsigned int width     = dims[1];

		unsigned int roiwidth;
		unsigned int roiheight;

		if ( !_minX && !_minY && !_maxX && !_maxY) {
			roiwidth = width;
			roiheight = height;
		} else {
			roiwidth = (_maxX - _minX) + 1;
			roiheight = (_maxY - _minY) + 1;
		}

		// create the section image
		boost::shared_ptr<Image> section = boost::make_shared<Image>(roiwidth, roiheight);

		LOG_DEBUG(imagestackhdf5readerlog) << "section width and height " << width << ", " << height << std::endl;
		LOG_DEBUG(imagestackhdf5readerlog) << "roi width and height " << roiwidth << ", " << roiheight << std::endl;

		for (unsigned int j = 0; j < roiwidth*roiheight; j++) {
			(*section)[j] = (float)data[ ((_minY + j / roiwidth) * width + _minX) + j % roiwidth ]/255.0;
			/*if( (*section)[j] > 0.39)
				LOG_DEBUG(imagestackhdf5readerlog) << (*section)[j] << std::endl;*/
			//LOG_DEBUG(imagestackhdf5readerlog) << "((_minY + j / roiwidth) * width + _minX) + j percent roiwidth " <<","<< _minY<<","<<j<<","<<roiwidth<<","<<width<<","<<_minX<<","<<j<<","<<roiwidth << std::endl;
			//LOG_DEBUG(imagestackhdf5readerlog) << "value " << ((_minY + j / 
			//roiwidth) * width + _minX) + j % roiwidth << "," << (*section)[j] 
			//<< std::endl;
		}

		_stack->add(section);

	}

	LOG_DEBUG(imagestackhdf5readerlog) << "read sections from " << _originSection << " to " << _targetSection << std::endl;
}

#endif // HAVE_HDF5
