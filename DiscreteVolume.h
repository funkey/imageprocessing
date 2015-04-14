#ifndef IMAGEPROCESSING_DISCRETIZATION_H__
#define IMAGEPROCESSING_DISCRETIZATION_H__

#include <imageprocessing/Volume.h>

/**
 * A discretization of a volume. Points within the bounding box of the volume 
 * are discretized and given zero-based coordinates.
 */
class DiscreteVolume : public Volume {

public:

	/**
	 * Default constructor. Creates a discretized volume with a resolution of 
	 * one unit per direction and an offset of (0,0,0).
	 */
	DiscreteVolume() :
		_res(1.0, 1.0, 1.0),
		_discreteBoundingBoxDirty(true) {}

	DiscreteVolume(const DiscreteVolume& other) :
		Volume(other),
		_res(other._res),
		_offset(other._offset),
		_discreteBoundingBox(other._discreteBoundingBox),
		_discreteBoundingBoxDirty(other._discreteBoundingBoxDirty) {}

	/**
	 * Create a new discrete volume.
	 */
	DiscreteVolume(float resX, float resY, float resZ) :
		_res(resX, resY, resZ),
		_discreteBoundingBoxDirty(true) {}

	/**
	 * Set the resolution of this discretized volume.
	 */
	void setResolution(float resX, float resY, float resZ) { _res = util::point<float,3>(resX, resY, resZ); Volume::setBoundingBoxDirty(); }
	void setResolution(const util::point<float,3>& res)    { _res = res; Volume::setBoundingBoxDirty(); }

	/**
	 * Get the resolution of this discretized volume.
	 */
	float getResolutionX() const { return _res.x(); }
	float getResolutionY() const { return _res.y(); }
	float getResolutionZ() const { return _res.z(); }
	const util::point<float,3>& getResolution() const { return _res; }

	/**
	 * Set the volume location which the discrete coordinates (0,0,0) would 
	 * have.
	 */
	void setOffset(float x, float y, float z)          { setOffset(util::point<float,3>(x, y, z)); }
	void setOffset(const util::point<float,3>& offset) { _offset = offset; Volume::setBoundingBoxDirty(); }

	/**
	 * Get the volume location which the discrete coordinates (0,0,0) would 
	 * have.
	 */
	const util::point<float,3>& getOffset() const { return _offset; }

	/**
	 * Transform a real-valued volume location into discrete coordinates.
	 */
	void getDiscreteCoordinates(
			float x, float y, float z,
			unsigned int& dx, unsigned int& dy, unsigned int& dz) const {

		// remove offset
		x -= getBoundingBox().min().x();
		y -= getBoundingBox().min().y();
		z -= getBoundingBox().min().z();

		// discretize
		dx = x/_res.x();
		dy = y/_res.y();
		dz = z/_res.z();
	}

	/**
	 * Transform discrete coordinates into a real-valued location.
	 */
	void getRealLocation(
			unsigned int dx, unsigned int dy, unsigned int dz,
			float& x, float& y, float& z)  const{

		x = dx*_res.x() + _offset.x();
		y = dy*_res.y() + _offset.y();
		z = dz*_res.z() + _offset.z();
	}

	/**
	 * Get the discrete bounding box of this volume.
	 */
	const util::box<unsigned int,3>& getDiscreteBoundingBox() const {

		if (_discreteBoundingBoxDirty) {

			_discreteBoundingBox = computeDiscreteBoundingBox();
			_discreteBoundingBoxDirty = false;
		}

		return _discreteBoundingBox;
	}

	/**
	 * Indicate that the bounding box changed and needs to be recomputed the 
	 * next time it is queried.
	 */
	void setDiscreteBoundingBoxDirty() { _discreteBoundingBoxDirty = true; Volume::setBoundingBoxDirty(); }

	/**
	 * Fallback for subclasses.
	 */
	void setBoundingBoxDirty() { setDiscreteBoundingBoxDirty(); }

protected:

	/**
	 * To be overwritten by subclasses to compute the discrete bounding box 
	 * after it was set dirty.
	 */
	virtual util::box<unsigned int,3> computeDiscreteBoundingBox() const = 0;

	util::box<float,3> computeBoundingBox() const override final {

		const util::box<float,3>& bb = getDiscreteBoundingBox();

		return bb*_res + _offset;
	}

private:

	util::point<float,3> _res;
	util::point<float,3> _offset;

	/**
	 * Since we want the bounding box to be computed as needed, even in a const 
	 * setting, we make it mutable.
	 */
	mutable util::box<unsigned int,3> _discreteBoundingBox;

	/**
	 * Same for the dirty flag.
	 */
	mutable bool _discreteBoundingBoxDirty;
};

#endif // IMAGEPROCESSING_DISCRETIZATION_H__

