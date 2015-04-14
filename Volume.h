#ifndef IMAGEPROCESSING_VOLUME_H__
#define IMAGEPROCESSING_VOLUME_H__

#include <util/box.hpp>

/**
 * Base for classes representing a 3D volume.
 */
class Volume {

public:

	Volume() :
		_boundingBoxDirty(true) {}

	Volume(const Volume& other) :
		_boundingBox(other._boundingBox),
		_boundingBoxDirty(other._boundingBoxDirty) {}

	/**
	 * Get the bounding box of this volume.
	 */
	const util::box<float,3>& getBoundingBox() const {

		if (_boundingBoxDirty) {

			_boundingBox = computeBoundingBox();
			_boundingBoxDirty = false;
		}

		return _boundingBox;
	}

	/**
	 * Indicate that the bounding box changed and needs to be recomputed the 
	 * next time it is queried.
	 */
	void setBoundingBoxDirty() { _boundingBoxDirty = true; }

protected:

	/**
	 * To be overwritten by subclasses to compute the bounding box after it was 
	 * set dirty.
	 */
	virtual util::box<float,3> computeBoundingBox() const = 0;

private:

	/**
	 * Since we want the bounding box to be computed as needed, even in a const 
	 * setting, we make it mutable.
	 */
	mutable util::box<float,3> _boundingBox;

	/**
	 * Same for the dirty flag.
	 */
	mutable bool _boundingBoxDirty;
};


#endif // IMAGEPROCESSING_VOLUME_H__

