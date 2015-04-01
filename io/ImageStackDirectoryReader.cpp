#include <fstream>
#include <boost/program_options.hpp>
#include <imageprocessing/Image.h>
#include <imageprocessing/ImageStack.h>
#include <imageprocessing/io/ImageReader.h>
#include "ImageStackDirectoryReader.h"

logger::LogChannel imagestackdirectoryreaderlog("imagestackdirectoryreaderlog", "[ImageStackDirectoryReader] ");

ImageStackDirectoryReader::ImageStackDirectoryReader(const std::string& directory) :
	_stackAssembler(boost::make_shared<StackAssembler>()),
	_directory(directory) {

	LOG_DEBUG(imagestackdirectoryreaderlog) << "reading from directory " << _directory << std::endl;

	boost::filesystem::path dir(_directory);

	if (!boost::filesystem::exists(dir))
		BOOST_THROW_EXCEPTION(IOError() << error_message(_directory + " does not exist"));

	std::vector<boost::filesystem::path> files;

	if (!boost::filesystem::is_directory(dir)) {

		files.push_back(dir);

	} else {

		// get a sorted list of image files
		std::copy(
				boost::filesystem::directory_iterator(dir),
				boost::filesystem::directory_iterator(),
				back_inserter(files));
		std::sort(files.begin(), files.end());
	}

	LOG_DEBUG(imagestackdirectoryreaderlog) << "directory contains " << files.size() << " entries" << std::endl;

	// for every image file in the given directory
	foreach (boost::filesystem::path file, files) {

		if (file.filename() == "META") {

			processMetaData(file);
			continue;
		}

		if (boost::filesystem::is_regular_file(file)) {

			LOG_DEBUG(imagestackdirectoryreaderlog) << "creating reader for " << file << std::endl;

			// add an input to the stack assembler
			boost::shared_ptr<ImageReader> reader = boost::make_shared<ImageReader>(file.string());

			_stackAssembler->addInput(reader->getOutput());
		}
	}

	// expose the result of the stack assembler
	registerOutput(_stackAssembler->getOutput(), "stack");
}

void
ImageStackDirectoryReader::processMetaData(boost::filesystem::path file) {

	boost::program_options::options_description desc("image stack directory META options");

	desc.add_options()
		("resX", boost::program_options::value<float>()->default_value(1.0), "x resolution")
		("resY", boost::program_options::value<float>()->default_value(1.0), "y resolution")
		("resZ", boost::program_options::value<float>()->default_value(1.0), "z resolution");

	std::ifstream config(file.c_str());

	if (!config.good()) {

		LOG_ERROR(imagestackdirectoryreaderlog) << "ERROR: can't open config file: " << file << std::endl;
		return;
	}

	boost::program_options::variables_map map;
	boost::program_options::store(boost::program_options::parse_config_file(config, desc), map);
	boost::program_options::notify(map);

	float resX = map["resX"].as<float>();
	float resY = map["resY"].as<float>();
	float resZ = map["resZ"].as<float>();

	_stackAssembler->setResolution(resX, resY, resZ);
}

ImageStackDirectoryReader::StackAssembler::StackAssembler() :
	_stack(new ImageStack()),
	_resX(1.0),
	_resY(1.0),
	_resZ(1.0) {

	registerInputs(_images, "images");
	registerOutput(_stack, "stack");
}

void
ImageStackDirectoryReader::StackAssembler::updateOutputs() {

	_stack->clear();

	foreach (boost::shared_ptr<Image> image, _images)
		_stack->add(image);

	_stack->setResolution(_resX, _resY, _resZ);
	_stack->setBoundingBox(
			util::box<float>(
					0, 0, 0,
					_stack->width()*_resX, _stack->height()*_resY, _images.size()*_resZ));
}
