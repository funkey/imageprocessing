#ifndef IMAGEPROCESSING_IMAGE_LEVEL_PARSER_H__
#define IMAGEPROCESSING_IMAGE_LEVEL_PARSER_H__

#include <stack>
#include <limits>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <vigra/transformimage.hxx>
#include <vigra/functorexpression.hxx>

#include <util/Logger.h>
#include "PixelList.h"
#include "Image.h"

extern logger::LogChannel imagelevelparserlog;

/**
 * Parses the pixels of an image in terms of the connected components of varying 
 * intensity thresholds in linear time. For each connected component and each 
 * threshold value, a user specified callback is invoked.
 *
 * The number of thresholds applied is given by the Precision template argument: 
 * The input image is discretized into the range of Precision, and all possible 
 * thresholds are applied (for example, unsigned char corresponds to 255 
 * thresholds).
 */
template <typename Precision = unsigned char>
class ImageLevelParser {

public:

	/**
	 * Create a new image level parser for the given image.
	 */
	ImageLevelParser(const Image& image);

	/**
	 * Parse the image. The provided visitor has to implement
	 *
	 *     void Visitor::componentFound(
	 *         Image::value_type            value,
	 *         PixelList::const_iterator    begin,
	 *         PixelList::const_iterator    end,
	 *         boost::shared_ptr<PixelList> pixelList);
	 *
	 * This method will be called for every connected component at every 
	 * threshold, where value is the original value (before discretization) of 
	 * the threshold, and begin and end are iterators into pixelList that span 
	 * all pixels of the connected component.
	 *
	 * The visitor can assume that the callback is invoked following a weak 
	 * ordering of the connected component according to the subset relation.
	 */
	template <typename Visitor>
	void parse(Visitor& visitor);

private:

	typedef util::point<unsigned int> point_type;

	/**
	 * Set the current location and level.
	 */
	void gotoLocation(const point_type& location);

	/**
	 * Fill the level at the current location. Returns true, if the level is 
	 * bounded (only higher levels surround it); otherwise false.
	 */
	bool fillLevel();

	/**
	 * In the current boundary locations, find the highest level that is smaller 
	 * than the current level and go there.
	 */
	void gotoLowerLevel();

	/**
	 * In the current boundary locations, try to find the lowest level that is 
	 * higher than the current level and go there. If such a level exists, all 
	 * the open components until this level are closed (and the visitor 
	 * informed) and true is returned. Otherwise, all remaining open components 
	 * are closed (including the one for MaxValue) and false is returned.
	 */
	template <typename Visitor>
	bool gotoHigherLevel(Visitor& visitor);

	/**
	 * Check if there are open boundary locations.
	 */
	// TODO: needed?
	bool haveOpenBoundary();

	/**
	 * Put a boundary location with its level on the stack of open boundary 
	 * locations.
	 */
	void pushBoundaryLocation(const point_type& location, Precision level);

	/**
	 * Get the next open boundary location that has a level smaller than the 
	 * given level.
	 *
	 * @param level
	 *              The reference level.
	 *
	 * @param boundaryLocation
	 *              [out] The next boundary location with level smaller then 
	 *              level.
	 *
	 * @param boundaryLevel
	 *              [out] The level of the found boundary location.
	 *
	 * @return true, if there is such a boundary location
	 */
	bool popLowerBoundaryLocation(
			Precision   level,
			point_type& boundaryLocation,
			Precision&  boundaryLevel);

	/**
	 * Get the next open boundary location that has a level higher than the 
	 * given level.
	 *
	 * @param level
	 *              The reference level.
	 *
	 * @param boundaryLocation
	 *              [out] The next boundary location with level higher then 
	 *              level.
	 *
	 * @param boundaryLevel
	 *              [out] The level of the found boundary location.
	 *
	 * @return true, if there is such a boundary location
	 */
	bool popHigherBoundaryLocation(
			Precision   level,
			point_type& boundaryLocation,
			Precision&  boundaryLevel);

	/**
	 * Begin a new connected component at the current location for the given 
	 * level.
	 */
	void beginComponent(Precision level);

	/**
	 * End a connected component at the current location.
	 *
	 * @return The begin and end iterator of the new connected component in the 
	 * pixel list.
	 */
	template <typename Visitor>
	void endComponent(Precision level, Visitor& visitor);

	/**
	 * Discretized the input image into the range defined by Precision.
	 */
	void discretizeImage(const Image& image);

	/**
	 * Get the orignal value that corresponds to the given discretized value.
	 */
	float getOriginalValue(Precision value);

	static const Precision MaxValue;

	// discretized version of the input image
	vigra::MultiArray<2, Precision> _image;

	// the current location of the parsing algorithm
	point_type _currentLocation;
	Precision  _currentLevel;

	// the pixel list, shared ownership with visitors
	boost::shared_ptr<PixelList> _pixelList;

	// stacks of open boundary locations
	std::vector<std::stack<point_type> > _boundaryLocations;

	// number of open boundary locations
	// TODO: needed?
	size_t _numOpenLocations;

	// stack of component begin iterators (with the level they have been 
	// generated for)
	std::stack<std::pair<Precision, PixelList::iterator> > _componentBegins;
};

template <typename Precision>
const Precision ImageLevelParser<Precision>::MaxValue = std::numeric_limits<Precision>::max();

template <typename Precision>
ImageLevelParser<Precision>::ImageLevelParser(const Image& image) :
	_pixelList(boost::make_shared<PixelList>(image.size())),
	_boundaryLocations(MaxValue + 1),
	_numOpenLocations(0) {

	LOG_DEBUG(imagelevelparserlog) << "initializing for image of size " << image.size() << std::endl;

	this->discretizeImage(image);
}

template <typename Precision>
template <typename Visitor>
void
ImageLevelParser<Precision>::parse(Visitor& visitor) {

	LOG_DEBUG(imagelevelparserlog) << "parsing image" << std::endl;

	// We initialize the process by adding the upper left pixel to the stack of 
	// open boundary pixels...
	pushBoundaryLocation(point_type(0, 0), _image(0, 0));

	// ...pretending we have opened a component for MaxValue already...
	beginComponent(MaxValue);
	_currentLevel = MaxValue;

	// ...and going to the next lower level (which is our initial pixel). This 
	// way we make sure enough components are put on the stack.
	gotoLowerLevel();

	LOG_DEBUG(imagelevelparserlog)
			<< "starting at " << _currentLocation
			<< " with level " << _currentLevel
			<< std::endl;

	// loop through the image
	while (true) {

		// try to fill the current level
		bool bounded = fillLevel();

		// there are no lower levels around us
		if (bounded) {

			LOG_DEBUG(imagelevelparserlog)
					<< "filled current level"
					<< std::endl;

			// try go to the smallest higher level, according to our open 
			// boundary list
			if (!gotoHigherLevel(visitor)) {

				LOG_DEBUG(imagelevelparserlog)
						<< "there are no more higher levels -- we are done"
						<< std::endl;

				// if there are no higher levels, we are done
				return;
			}

		} else {

			LOG_DEBUG(imagelevelparserlog)
					<< "current level is not bounded, yet"
					<< std::endl;

			// there are boundary locations that have lower levels -- go there 
			// and continue filling
			gotoLowerLevel();
		}
	}
}

template <typename Precision>
void
ImageLevelParser<Precision>::gotoLocation(const point_type& location) {

	_currentLocation = location;
	_currentLevel    = _image(location.x, location.y);
}

template <typename Precision>
bool
ImageLevelParser<Precision>::fillLevel() {

	// TODO
	return true;
}

template <typename Precision>
void
ImageLevelParser<Precision>::gotoLowerLevel() {

	assert(_currentLevel != 0);

	point_type newLocation;
	Precision  newLevel;

	LOG_ALL(imagelevelparserlog)
			<< "trying to find highest boundary location lower then "
			<< (int)_currentLevel << std::endl;

	// find the highest boundary location lower than the current level
	bool found = popLowerBoundaryLocation(_currentLevel, newLocation, newLevel);
	assert(found);

	LOG_ALL(imagelevelparserlog)
			<< "found boundary location " << newLocation
			<< " with level " << (int)newLevel << std::endl;

	// begin a new component for each level that we descend
	for (Precision level = _currentLevel - 1;; level--) {

		beginComponent(level);

		if (level == newLevel)
			break;
	}

	gotoLocation(newLocation);

	assert(_currentLevel == newLevel);
}

template <typename Precision>
template <typename Visitor>
bool
ImageLevelParser<Precision>::gotoHigherLevel(Visitor& visitor) {

	point_type newLocation;
	Precision  newLevel;

	LOG_ALL(imagelevelparserlog)
			<< "trying to find smallest boundary location higher then "
			<< (int)_currentLevel << std::endl;

	// find the lowest boundary location higher then the current level
	bool found = popHigherBoundaryLocation(_currentLevel, newLocation, newLevel);

	if (!found) {

		LOG_ALL(imagelevelparserlog) << "nothing found, finishing up" << std::endl;

		// There are no more higher levels, we are done. End all the remaining 
		// open components (which are at least the component for level 
		// MaxValue).
		for (Precision level = _currentLevel;; level++) {

			endComponent(level, visitor);

			if (level == MaxValue)
				return false;
		}
	}

	LOG_ALL(imagelevelparserlog)
			<< "found boundary location " << newLocation
			<< " with level " << (int)newLevel << std::endl;

	// close one component for each level that we ascend
	for (Precision level = _currentLevel;; level++) {

		endComponent(level, visitor);

		if (level == newLevel - 1)
			break;
	}

	gotoLocation(newLocation);

	assert(_currentLevel == newLevel);

	return true;
}

template <typename Precision>
bool
ImageLevelParser<Precision>::haveOpenBoundary() {

	return _numOpenLocations > 0;
}

template <typename Precision>
void
ImageLevelParser<Precision>::pushBoundaryLocation(const point_type& location, Precision level) {

	_boundaryLocations[level].push(location);
	_numOpenLocations++;
}

template <typename Precision>
bool
ImageLevelParser<Precision>::popLowerBoundaryLocation(
		Precision   level,
		point_type& boundaryLocation,
		Precision&  boundaryLevel) {

	if (level == 0)
		return false;

	for (Precision l = level - 1;; l--) {

		if (!_boundaryLocations[l].empty()) {

			boundaryLocation = _boundaryLocations[l].top();
			boundaryLevel    = l;

			_boundaryLocations[l].pop();
			_numOpenLocations--;

			return true;
		}

		if (l== 0)
			return false;
	}
}

template <typename Precision>
bool
ImageLevelParser<Precision>::popHigherBoundaryLocation(
		Precision   level,
		point_type& boundaryLocation,
		Precision&  boundaryLevel) {

	if (level == MaxValue)
		return false;

	for (Precision l = level + 1;; l++) {

		if (!_boundaryLocations[l].empty()) {

			boundaryLocation = _boundaryLocations[l].top();
			boundaryLevel    = l;

			_boundaryLocations[l].pop();
			_numOpenLocations--;

			return true;
		}

		if (l == MaxValue)
			return false;
	}
}

template <typename Precision>
void
ImageLevelParser<Precision>::beginComponent(Precision level) {

	_componentBegins.push(std::make_pair(level, _pixelList->end()));
}

template <typename Precision>
template <typename Visitor>
void
ImageLevelParser<Precision>::endComponent(Precision level, Visitor& visitor) {

	assert(_componentBegins.size() > 0);

	std::pair<Precision, PixelList::iterator> levelBegin = _componentBegins.top();
	_componentBegins.pop();

	PixelList::iterator begin = levelBegin.second;
	PixelList::iterator end   = _pixelList->end();

	assert(levelBegin.first == level);

	LOG_ALL(imagelevelparserlog) << "ending component with level " << (int)level << std::endl;

	visitor.componentFound(
			getOriginalValue(level),
			begin, end,
			_pixelList);
}

template <typename Precision>
void
ImageLevelParser<Precision>::discretizeImage(const Image& image) {

	_image.reshape(image.shape());

	vigra::transformImage(
			srcImageRange(image),
			destImage(_image),
			vigra::functor::Arg1()*vigra::functor::Param(MaxValue));
}

template <typename Precision>
float
ImageLevelParser<Precision>::getOriginalValue(Precision value) {

	return static_cast<float>(value)/MaxValue;
}

#endif // IMAGEPROCESSING_IMAGE_LEVEL_PARSER_H__

