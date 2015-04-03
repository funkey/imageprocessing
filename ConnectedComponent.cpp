#include <boost/make_shared.hpp>

#include <imageprocessing/exceptions.h>
#include "ConnectedComponent.h"

ConnectedComponent::ConnectedComponent() :
	_boundingBox(0, 0, 0, 0),
	_center(0, 0) {}

ConnectedComponent::ConnectedComponent(
		boost::shared_ptr<Image> source,
		double value,
		boost::shared_ptr<pixel_list_type> pixelList,
		pixel_list_type::const_iterator begin,
		pixel_list_type::const_iterator end) :

	_pixels(pixelList),
	_value(value),
	_boundingBox(0, 0, 0, 0),
	_center(0, 0),
	_source(source),
	_pixelRange(begin, end) {

	// if there is at least one pixel
	if (begin != end) {

		_boundingBox.minX = begin->x();
		_boundingBox.maxX = begin->x() + 1;
		_boundingBox.minY = begin->y();
		_boundingBox.maxY = begin->y() + 1;
	}

	typedef util::point<unsigned int,2> point2;
	foreach (const point2& pixel, getPixels()) {

		_boundingBox.minX = std::min(_boundingBox.minX, (int)pixel.x());
		_boundingBox.maxX = std::max(_boundingBox.maxX, (int)pixel.x() + 1);
		_boundingBox.minY = std::min(_boundingBox.minY, (int)pixel.y());
		_boundingBox.maxY = std::max(_boundingBox.maxY, (int)pixel.y() + 1);

		_center += pixel;
	}

	_center /= getSize();

	_bitmap.reshape(bitmap_type::size_type(_boundingBox.width(), _boundingBox.height()), false);

	typedef util::point<int,2> pointi2;
	foreach (const pointi2& pixel, getPixels())
		_bitmap(pixel.x() - _boundingBox.minX, pixel.y() - _boundingBox.minY) = true;
}

double
ConnectedComponent::getValue() const {

	return _value;
}

const util::point<double,2>&
ConnectedComponent::getCenter() const {

	return _center;
}

const std::pair<ConnectedComponent::const_iterator, ConnectedComponent::const_iterator>&
ConnectedComponent::getPixels() const {

	return _pixelRange;
}

const boost::shared_ptr<ConnectedComponent::pixel_list_type>
ConnectedComponent::getPixelList() const {

	return _pixels;
}

unsigned int
ConnectedComponent::getSize() const {

	return _pixelRange.second - _pixelRange.first;
}

const util::rect<int>&
ConnectedComponent::getBoundingBox() const {

	return _boundingBox;
}

const ConnectedComponent::bitmap_type&
ConnectedComponent::getBitmap() const {

	return _bitmap;
}

bool
ConnectedComponent::operator<(const ConnectedComponent& other) const {

	return getSize() < other.getSize();
}

ConnectedComponent
ConnectedComponent::translate(const util::point<int,2>& pt)
{
	boost::shared_ptr<pixel_list_type> translation = boost::make_shared<pixel_list_type>(getSize());
	
	typedef util::point<unsigned int,2> point2;
	foreach (const point2& pixel, getPixels())
	{
		translation->add(pixel + pt);
	}
	
	return ConnectedComponent(_source, _value, translation, translation->begin(), translation->end());
}


ConnectedComponent
ConnectedComponent::intersect(const ConnectedComponent& other) {

	// find the intersection pixels
	std::vector<util::point<unsigned int,2> > intersectionPixels;
	bitmap_type::size_type size = _bitmap.shape();
	typedef util::point<unsigned int,2> point2;
	foreach (const point2& pixel, other.getPixels())
		if (_boundingBox.contains(pixel)) {

			unsigned int x = pixel.x() - _boundingBox.minX;
			unsigned int y = pixel.y() - _boundingBox.minY;

			if (x >= size[0] || y >= size[1])
				continue;

			if (_bitmap(x, y))
				intersectionPixels.push_back(pixel);
		}

	// create a pixel list for them
	boost::shared_ptr<pixel_list_type> intersection =
			boost::make_shared<pixel_list_type>(intersectionPixels.size());
	foreach (const point2& pixel, intersectionPixels)
		intersection->add(pixel);

	return ConnectedComponent(_source, _value, intersection, intersection->begin(), intersection->end());
}

bool ConnectedComponent::intersects(const ConnectedComponent& other)
{
	if (_boundingBox.intersects(other.getBoundingBox()))
	{
		bitmap_type::size_type size = _bitmap.shape();

		typedef util::point<unsigned int,2> point2;
		foreach (const point2& pixel, other.getPixels())
		{
			if (_boundingBox.contains(pixel)) {

				unsigned int x = pixel.x() - _boundingBox.minX;
				unsigned int y = pixel.y() - _boundingBox.minY;

				if (x >= size[0] || y >= size[1])
					continue;

				if (_bitmap(x, y))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}


bool
ConnectedComponent::operator==(const ConnectedComponent& other) const
{
	util::rect<int> thisBound = getBoundingBox();
	util::rect<int> otherBound = other.getBoundingBox();

	if (thisBound == otherBound)
	{
		// If this bound equals that bound
		bitmap_type thisBitmap = getBitmap();
		bitmap_type otherBitmap = other.getBitmap();
		
		//Check that the other's bitmap contains all of our pixels.
		typedef util::point<unsigned int,2> point2;
		foreach (const point2& pixel, getPixels())
		{
			if (!otherBitmap(pixel.x() - thisBound.minX, pixel.y() - thisBound.minY))
			{
				return false;
			}
		}
		
		//Check that our bitmap contains all of the other's pixels.
		foreach (const point2& pixel, other.getPixels())
		{
			if (!thisBitmap(pixel.x() - otherBound.minX, pixel.y() - otherBound.minY))
			{
				return false;
			}
		}
		
		//If both conditions are true, both components contain each other, and are therefore equal.
		return true;
	}
	else
	{
		// If our bound is unequal to the other's bound, then we're unequal.
		return false;
	}
}
