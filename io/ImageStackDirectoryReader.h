#ifndef IMAGEPROCESSING_IMAGE_STACK_DIRECTORY_READER_H__
#define IMAGEPROCESSING_IMAGE_STACK_DIRECTORY_READER_H__

#include <string>

#include <boost/filesystem.hpp>

#include <pipeline/all.h>
#include <imageprocessing/Image.h>
#include <imageprocessing/ImageStack.h>

// forward declarations
class Image;
class ImageStack;

class ImageStackDirectoryReader : public pipeline::ProcessNode {

public:

	ImageStackDirectoryReader(const std::string& directory);

private:

	class StackAssembler : public pipeline::SimpleProcessNode<> {

	public:

		StackAssembler();

		void setResolution(float x, float y, float z) { _resX = x; _resY = y; _resZ = z; }

	private:

		void updateOutputs();

		pipeline::Inputs<Image> _images;

		pipeline::Output<ImageStack> _stack;

		float _resX;
		float _resY;
		float _resZ;
	};

	void processMetaData(boost::filesystem::path file);

	boost::shared_ptr<StackAssembler> _stackAssembler;

	std::string _directory;
};

#endif // IMAGEPROCESSING_IMAGE_STACK_DIRECTORY_READER_H__

