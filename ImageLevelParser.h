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
	 * Parameters of the image level parser.
	 */
	struct Parameters {

		Parameters() : darkToBright(true) {}

		// start processing the dark regions
		bool darkToBright;
	};

	/**
	 * Base class and interface definition of visitors that are accepted by the 
	 * parse methods. Visitors don't need to inherit from this class (as long as 
	 * they implement the same interface). This class is provided for 
	 * convenience with no-op methods.
	 */
	class Visitor {

	public:

		/**
		 * Invoked whenever a new component is added as a child of the current 
		 * component, starting from the root (the whole image component) in a 
		 * depth first manner.  Indicates that we go down by one level in the 
		 * component tree and make the new child the current component.
		 *
		 * @param value
		 *              The threshold value of the new child.
		 */
		void newChildComponent(Image::value_type value) {}

		/**
		 * Invoked whenever the current component was extracted entirely.  
		 * Indicates that we go up by one level in the component tree and make 
		 * the parent of the current component the new current component.
		 *
		 * @param value
		 *              The threshold value of the current component.
		 *
		 * @param begin, end
		 *              Iterators into the pixel list that define the pixels of 
		 *              the current component.
		 *
		 * @param pixelList
		 *              A pixel list shared between all components.
		 */
		void finalizeComponent(
				Image::value_type            value,
				PixelList::const_iterator    begin,
				PixelList::const_iterator    end,
				boost::shared_ptr<PixelList> pixelList) {}
	};

	/**
	 * Create a new image level parser for the given image with the given 
	 * parameters.
	 */
	ImageLevelParser(const Image& image, const Parameters& parameters = Parameters());

	/**
	 * Parse the image. The provided visitor has to implement the interface of 
	 * Visitor (but does not need to inherit from it).
	 *
	 * This method will be called for every connected component at every 
	 * threshold, where value is the original value (before discretization) of 
	 * the threshold, and begin and end are iterators into pixelList that span 
	 * all pixels of the connected component.
	 *
	 * The visitor can assume that the callback is invoked following a weak 
	 * ordering of the connected component according to the subset relation.
	 */
	template <typename VisitorType>
	void parse(VisitorType& visitor);

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
	template <typename VisitorType>
	void gotoLowerLevel(VisitorType& visitor);

	/**
	 * In the current boundary locations, try to find the lowest level that is 
	 * higher than the current level and go there. If such a level exists, all 
	 * the open components until this level are closed (and the visitor 
	 * informed) and true is returned. Otherwise, all remaining open components 
	 * are closed (including the one for MaxValue) and false is returned.
	 */
	template <typename VisitorType>
	bool gotoHigherLevel(VisitorType& visitor);

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
	 * Get the next open boundary location with the given level. Returns false 
	 * if there is none.
	 *
	 * @param level
	 *              The reference level.
	 *
	 * @param boundaryLocation
	 *              [out] The next boundary location with level smaller then 
	 *              level.
	 */
	bool popBoundaryLocation(
			Precision   level,
			point_type& boundaryLocation);

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
			unsigned int level, // not Precision, we have to pass MaxValue + 1
			point_type&  boundaryLocation,
			Precision&   boundaryLevel);

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
	template <typename VisitorType>
	void beginComponent(Precision level, VisitorType& visitor);

	/**
	 * End a connected component at the current location.
	 *
	 * @return The begin and end iterator of the new connected component in the 
	 * pixel list.
	 */
	template <typename VisitorType>
	void endComponent(Precision level, VisitorType& visitor);

	/**
	 * Find the neighbor of the current position in the given direction. Returns 
	 * false, if the neighbor is not valid (out of bounds or already visited).  
	 * Otherwise, neighborLocation and neighborLevel are set and true is 
	 * returned.
	 */
	typedef unsigned char Direction;
	static const Direction Right;
	static const Direction Down;
	static const Direction Left;
	static const Direction Up;
	bool findNeighbor(Direction direction, point_type& neighborLocation, Precision& neighborLevel);

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

	// parameters of the parsing algorithm
	Parameters _parameters;

	// the current location of the parsing algorithm
	point_type   _currentLocation;
	unsigned int _currentLevel; // not Precision, since we have to be able to express MaxValue + 1

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

	// visited flag for each pixel
	vigra::MultiArray<2, bool> _visited;
};

template <typename Precision>
const Precision ImageLevelParser<Precision>::MaxValue = std::numeric_limits<Precision>::max();
template <typename Precision>
const typename ImageLevelParser<Precision>::Direction ImageLevelParser<Precision>::Right = 0;
template <typename Precision>
const typename ImageLevelParser<Precision>::Direction ImageLevelParser<Precision>::Down  = 1;
template <typename Precision>
const typename ImageLevelParser<Precision>::Direction ImageLevelParser<Precision>::Left  = 2;
template <typename Precision>
const typename ImageLevelParser<Precision>::Direction ImageLevelParser<Precision>::Up    = 3;

template <typename Precision>
ImageLevelParser<Precision>::ImageLevelParser(const Image& image, const Parameters& parameters) :
	_parameters(parameters),
	_pixelList(boost::make_shared<PixelList>(image.size())),
	_boundaryLocations(MaxValue + 1),
	_numOpenLocations(0) {

	_visited.reshape(image.shape());
	_visited = false;

	LOG_DEBUG(imagelevelparserlog) << "initializing for image of size " << image.size() << std::endl;

	this->discretizeImage(image);
}

template <typename Precision>
template <typename VisitorType>
void
ImageLevelParser<Precision>::parse(VisitorType& visitor) {

	LOG_DEBUG(imagelevelparserlog) << "parsing image" << std::endl;

	// We initialize the process by adding the upper left pixel to the stack of 
	// open boundary pixels...
	pushBoundaryLocation(point_type(0, 0), _image(0, 0));

	// ...pretending we come from level MaxValue + 1...
	_currentLevel = MaxValue + 1;

	// ...and going to the next lower level (which is our initial pixel). This 
	// way we make sure enough components are put on the stack.
	gotoLowerLevel(visitor);

	LOG_DEBUG(imagelevelparserlog)
			<< "starting at " << _currentLocation
			<< " with level " << (int)_currentLevel
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
			gotoLowerLevel(visitor);
		}
	}
}

template <typename Precision>
void
ImageLevelParser<Precision>::gotoLocation(const point_type& location) {

	_currentLocation = location;
	_currentLevel    = _image(location.x, location.y);

	assert(_visited(location.x, location.y) == false);

	_visited(location.x, location.y) = true;
	_pixelList->add(location);
}

template <typename Precision>
bool
ImageLevelParser<Precision>::fillLevel() {

	// we are supposed to fill all adjacent pixels of the current pixels that 
	// have the same level
	Precision fillLevel = _currentLevel;

	LOG_ALL(imagelevelparserlog) << "filling level " << (int)fillLevel << std::endl;

	point_type neighborLocation;
	Precision  neighborLevel;

	// walk around...
	while (true) {

		LOG_ALL(imagelevelparserlog) << "I am at " << _currentLocation << std::endl;

		bool smallerNeighborFound = false;

		// look at all valid neighbors
		for (Direction direction = 0; direction < 4; direction++) {

			// is this a valid neighbor?
			if (!findNeighbor(direction, neighborLocation, neighborLevel))
				continue;

			if (neighborLevel < fillLevel) {

				LOG_ALL(imagelevelparserlog)
						<< "neighbor is smaller (" << (int)neighborLevel
						<< "), will go down" << std::endl;

				// We found a smaller neighbor. Remember the neighbor and return 
				// false, since we are not bounded. This will cause filling of 
				// the lower levels.
				pushBoundaryLocation(neighborLocation, neighborLevel);
				smallerNeighborFound = true;

			} else if (neighborLevel > fillLevel) {

				LOG_ALL(imagelevelparserlog)
						<< "neighbor is larger (" << (int)neighborLevel
						<< "), will remember it" << std::endl;

				// we found a larger neighbor -- remember it
				pushBoundaryLocation(neighborLocation, neighborLevel);

			} else {

				LOG_ALL(imagelevelparserlog)
						<< "neighbor is equal (" << (int)neighborLevel
						<< "), will remember it" << std::endl;

				// we found an equal neighbor -- remember it
				pushBoundaryLocation(neighborLocation, neighborLevel);
			}
		}

		// continue filling the smaller level
		if (smallerNeighborFound)
			return false;

		// try to find the next non-visited boundary location of the current 
		// level
		while (true) {

			point_type newLocation;
			bool found = popBoundaryLocation(fillLevel, newLocation);

			// if there aren't any other boundary locations of the current 
			// level, we are done and bounded
			if (!found) {

				LOG_ALL(imagelevelparserlog)
						<< "no more boundary locations for the current level"
						<< std::endl;

				return true;
			}

			LOG_ALL(imagelevelparserlog)
					<< "found location " << newLocation
					<< " on the boundary" << std::endl;

			// continue searching, if the boundary location was not visited 
			// already
			if (_visited(newLocation.x, newLocation.y)) {

				LOG_ALL(imagelevelparserlog) << "this location was visited already" << std::endl;
				continue;
			}

			LOG_ALL(imagelevelparserlog) << "going to the new location" << std::endl;

			// we found a not-yet-visited boundary location of the current 
			// level -- continue filling with it
			gotoLocation(newLocation);
			break;
		}
	}

	return true;
}

template <typename Precision>
template <typename VisitorType>
void
ImageLevelParser<Precision>::gotoLowerLevel(VisitorType& visitor) {

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

		beginComponent(level, visitor);

		if (level == newLevel)
			break;
	}

	gotoLocation(newLocation);

	assert(_currentLevel == newLevel);
}

template <typename Precision>
template <typename VisitorType>
bool
ImageLevelParser<Precision>::gotoHigherLevel(VisitorType& visitor) {

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

	LOG_ALL(imagelevelparserlog)
			<< "ending all components in the range "
			<< (int)_currentLevel << " - " << ((int)newLevel - 1)
			<< std::endl;

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
ImageLevelParser<Precision>::popBoundaryLocation(
		Precision   level,
		point_type& boundaryLocation) {

	if (_boundaryLocations[level].empty())
		return false;

	boundaryLocation = _boundaryLocations[level].top();
	_boundaryLocations[level].pop();
	_numOpenLocations--;

	return true;
}

template <typename Precision>
bool
ImageLevelParser<Precision>::popLowerBoundaryLocation(
		unsigned int level,
		point_type&  boundaryLocation,
		Precision&   boundaryLevel) {

	if (level == 0)
		return false;

	for (Precision l = level - 1;; l--) {

		if (popBoundaryLocation(l, boundaryLocation)) {

			boundaryLevel = l;
			return true;
		}

		if (l == 0)
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

		if (popBoundaryLocation(l, boundaryLocation)) {

			boundaryLevel = l;
			return true;
		}

		if (l == MaxValue)
			return false;
	}
}

template <typename Precision>
template <typename VisitorType>
void
ImageLevelParser<Precision>::beginComponent(Precision level, VisitorType& visitor) {

	_componentBegins.push(std::make_pair(level, _pixelList->end()));

	visitor.newChildComponent(getOriginalValue(level));
}

template <typename Precision>
template <typename VisitorType>
void
ImageLevelParser<Precision>::endComponent(Precision level, VisitorType& visitor) {

	assert(_componentBegins.size() > 0);

	std::pair<Precision, PixelList::iterator> levelBegin = _componentBegins.top();
	_componentBegins.pop();

	PixelList::iterator begin = levelBegin.second;
	PixelList::iterator end   = _pixelList->end();

	assert(levelBegin.first == level);

	//LOG_ALL(imagelevelparserlog) << "ending component with level " << (int)level << std::endl;

	visitor.finalizeComponent(
			getOriginalValue(level),
			begin, end,
			_pixelList);
}

template <typename Precision>
bool
ImageLevelParser<Precision>::findNeighbor(
		Direction   direction,
		point_type& neighborLocation,
		Precision&  neighborLevel) {


	if (direction == Left) {

		//LOG_ALL(imagelevelparserlog) << "trying to go left" << std::endl;

		if (_currentLocation.x == 0) {

			//LOG_ALL(imagelevelparserlog) << "\tout of bounds" << std::endl;
			return false;
		}

		neighborLocation = _currentLocation + point_type(-1,  0);
	}
	if (direction == Up) {

		//LOG_ALL(imagelevelparserlog) << "trying to go up" << std::endl;

		if (_currentLocation.y == 0) {

			//LOG_ALL(imagelevelparserlog) << "\tout of bounds" << std::endl;
			return false;
		}

		neighborLocation = _currentLocation + point_type( 0, -1);
	}
	if (direction == Right) {

		//LOG_ALL(imagelevelparserlog) << "trying to go right" << std::endl;
		neighborLocation = _currentLocation + point_type( 1,  0);
	}

	if (direction == Down) {

		//LOG_ALL(imagelevelparserlog) << "trying to go down" << std::endl;
		neighborLocation = _currentLocation + point_type( 0,  1);
	}

	// out of bounds?
	if (neighborLocation.x >= _image.width() ||
		neighborLocation.y >= _image.height()) {

		//LOG_ALL(imagelevelparserlog) << "\tlocation " << neighborLocation << " is out of bounds" << std::endl;
		return false;
	}

	// already visited?
	if (_visited(neighborLocation.x, neighborLocation.y)) {

		//LOG_ALL(imagelevelparserlog) << "\talready visited" << std::endl;
		return false;
	}

	//LOG_ALL(imagelevelparserlog)
			//<< "succeeded -- valid neighbor is "
			//<< neighborLocation << std::endl;

	// we're good
	neighborLevel = _image(neighborLocation.x, neighborLocation.y);

	return true;
}

template <typename Precision>
void
ImageLevelParser<Precision>::discretizeImage(const Image& image) {

	_image.reshape(image.shape());

	if (_parameters.darkToBright)
		vigra::transformImage(
				srcImageRange(image),
				destImage(_image),
				vigra::functor::Arg1()*vigra::functor::Param(MaxValue));
	else // invert the image on-the-fly
		vigra::transformImage(
				srcImageRange(image),
				destImage(_image),
				vigra::functor::Param(MaxValue) - vigra::functor::Arg1()*vigra::functor::Param(MaxValue));
}

template <typename Precision>
float
ImageLevelParser<Precision>::getOriginalValue(Precision value) {

	if (_parameters.darkToBright)
		return static_cast<float>(value)/MaxValue;
	else
		return static_cast<float>(MaxValue - value)/MaxValue;
}

#endif // IMAGEPROCESSING_IMAGE_LEVEL_PARSER_H__

