#ifndef IMAGEPROCESSING_IO_IMAGE_STACK_DIRECTORY_WRITER_H__
#define IMAGEPROCESSING_IO_IMAGE_STACK_DIRECTORY_WRITER_H__

#include <string>

#include <pipeline/all.h>
#include <imageprocessing/ImageStack.h>

class ImageStackDirectoryWriter : public pipeline::SimpleProcessNode<> {

public:

	ImageStackDirectoryWriter(std::string directory, std::string basename = "");

	bool write(std::string basename = "");

private:

	void updateOutputs() {}

	void onModified(const pipeline::Modified&);

	pipeline::Input<ImageStack> _stack;

	std::string _directory;
	std::string _basename;

	bool         _dirty;
	boost::mutex _dirtyMutex;
};

#endif // IMAGEPROCESSING_IO_IMAGE_STACK_DIRECTORY_WRITER_H__

