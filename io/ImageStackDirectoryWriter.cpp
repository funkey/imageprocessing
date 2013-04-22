#include <boost/lexical_cast.hpp>
#include <vigra/impex.hxx>

#include <util/Logger.h>

#include "ImageStackDirectoryWriter.h"

logger::LogChannel imagestackdirectorywriterlog("imagestackdirectorywriterlog", "[ImageStackDirectoryWriter] ");

ImageStackDirectoryWriter::ImageStackDirectoryWriter(std::string directory, std::string basename) :
	_directory(directory),
	_basename(basename),
	_dirty(false) {

	registerInput(_stack, "image stack");

	_stack.registerBackwardCallback(&ImageStackDirectoryWriter::onModified, this);
}

void
ImageStackDirectoryWriter::onModified(const pipeline::Modified&) {

	boost::mutex::scoped_lock lock(_dirtyMutex);

	LOG_DEBUG(imagestackdirectorywriterlog) << "stack changed" << std::endl;

	_dirty = true;
}

bool
ImageStackDirectoryWriter::write(std::string basename) {

	{
		boost::mutex::scoped_lock lock(_dirtyMutex);

		if (!_dirty)
			return false;

		_dirty = false;
	}

	LOG_DEBUG(imagestackdirectorywriterlog) << "attempting to write stack" << std::endl;

	if (!_stack) {

		LOG_ERROR(imagestackdirectorywriterlog) << "no input image stack set" << std::endl;
		return false;
	}

	LOG_DEBUG(imagestackdirectorywriterlog) << "requesting image update" << std::endl;

	updateInputs();

	boost::unique_lock<boost::shared_mutex> lock(_stack->getMutex());

	// save to file

	unsigned int i = 0;

	foreach (boost::shared_ptr<Image> image, *_stack) {

		std::stringstream number;
		number << std::setw(8) << std::setfill('0');
		number << i;

		std::string filename = _directory + "/" + _basename + basename + number.str() + ".png";
		vigra::exportImage(vigra::srcImageRange(*image), vigra::ImageExportInfo(filename.c_str()));

		i++;
	}

	LOG_DEBUG(imagestackdirectorywriterlog) << "images written" << std::endl;

	return true;
}
