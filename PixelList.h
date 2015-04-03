#ifndef IMAGEPROCESSING_PIXEL_LIST_H__
#define IMAGEPROCESSING_PIXEL_LIST_H__

#include <util/point.hpp>

/**
 * A list of pixel locations of known size. Adding pixels and clearing does not 
 * invalidate iterators into the list.
 */
class PixelList {

	typedef std::vector<util::point<unsigned int,2> > pixel_list_type;

public:

	typedef pixel_list_type::iterator       iterator;
	typedef pixel_list_type::const_iterator const_iterator;

	/**
	 * Create a new pixel list of the given size.
	 */
	PixelList(size_t size) :
		_pixelList(size),
		_next(_pixelList.begin()) {}

	/**
	 * Add a pixel to the pixel list. Existing iterators are not invalidated.
	 */
	void add(const util::point<unsigned int,2>& pixel) {

		// don't add more pixels than you said you would
		assert(_next != _pixelList.end());

		*_next = pixel;
		_next++;
	}

	/**
	 * Clear the pixel list. Existing iterators are not invalidated.
	 */
	void clear() { _next = _pixelList.begin(); }

	/**
	 * Iterator access.
	 */
	iterator       begin() { return _pixelList.begin(); }
	const_iterator begin() const { return _pixelList.begin(); }
	iterator       end() { return _next; }
	const_iterator end() const { return _next; }

	/**
	 * The number of pixels that have been added to this pixel list.
	 */
	size_t size() const { return (_next - _pixelList.begin()); }

private:

	// a non-resizing vector of pixel locations
	pixel_list_type _pixelList;

	// the next free position in the pixel list
	iterator _next;
};

#endif // IMAGEPROCESSING_PIXEL_LIST_H__

