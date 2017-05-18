#ifndef IMAGEPROCESSING_IMAGE_STACK_H__
#define IMAGEPROCESSING_IMAGE_STACK_H__

#include "Image.h"
#include "DiscreteVolume.h"

class ImageStack : public DiscreteVolume {

	// Image objects are shared between ImageStack
	typedef std::vector<boost::shared_ptr<Image> > sections_type;

public:

	typedef Image::value_type               value_type;

	typedef sections_type::iterator         iterator;

	typedef sections_type::const_iterator   const_iterator;

	/**
	 * Remove all sections.
	 */
	void clear();

	/**
	 * Add a single section to this set of sections.
	 */
	void add(boost::shared_ptr<Image> section);

	/**
	 * Add a set of sections to this set of sections.
	 */
	void addAll(boost::shared_ptr<ImageStack> stack);

	const const_iterator begin() const { return _sections.begin(); }

	iterator begin() { return _sections.begin(); }

	const const_iterator end() const { return _sections.end(); }

	iterator end() { return _sections.end(); }

	boost::shared_ptr<Image> operator[](unsigned int i) { return _sections[i]; }

	boost::shared_ptr<const Image> operator[](unsigned int i) const { return _sections[i]; }

	unsigned int size() const { return _sections.size(); }

	unsigned int width() const { return (size() > 0 ? _sections[0]->width() : 0); }

	unsigned int height() const { return (size() > 0 ? _sections[0]->height() : 0); }

private:

	sections_type _sections;
};

#endif // IMAGEPROCESSING_IMAGE_STACK_H__

