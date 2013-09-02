#ifndef IMAGEPROCESSING_IO_IMAGE_STACK_HDF5_READER_H__
#define IMAGEPROCESSING_IO_IMAGE_STACK_HDF5_READER_H__

#include <config.h>

#ifdef HAVE_HDF5

#include <util/hdf5.h>

#include <pipeline/all.h>
#include <imageprocessing/ImageStack.h>

class ImageStackHdf5Reader : public pipeline::SimpleProcessNode<> {

public:

	ImageStackHdf5Reader(
		const std::string& filename,
		const std::string& groupname,
		const std::string& datasetname,
		unsigned int originSection,
		unsigned int targetSection,
		unsigned int minX,
		unsigned int maxX,
		unsigned int minY,
		unsigned int maxY);

private:

	void updateOutputs();

	void readImages();

	// the name of the hdf5 file
	std::string _filename;

	// the name of the group that contains the dataset
	std::string _groupname;

	// the name of the dataset that contains the image stack
	std::string _datasetname;

	// bounding box of Region of Interest
	unsigned int _minX;
	unsigned int _maxX;
	unsigned int _minY;
	unsigned int _maxY;

	unsigned int _originSection;
	unsigned int _targetSection;

	pipeline::Output<ImageStack> _stack;
};

#endif // IMAGEPROCESSING_IO_IMAGE_STACK_HDF5_READER_H__

#endif // HAVE_HDF5
