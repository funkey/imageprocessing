#ifndef IMAGEPROCESSING_DISCRETIZATION_H__
#define IMAGEPROCESSING_DISCRETIZATION_H__

#include "Volume.h"

/**
 * A discretization of a volume. Points within the bounding box of the volume 
 * are discretized and given zero-based coordinates.
 */
class DiscreteVolume : public Volume {

public:

	/**
	 * Default constructor. Creates a discretized volume with a resolution of 
	 * one unit per direction.
	 */
	DiscreteVolume() :
		_resX(1.0),
		_resY(1.0),
		_resZ(1.0) {}

	/**
	 * Create a new discrete volume.
	 */
	DiscreteVolume(float resX, float resY, float resZ) :
		_resX(resX),
		_resY(resY),
		_resZ(resZ) {}

	/**
	 * Set the resolution of this discretized volume.
	 */
	void setResolution(float resX, float resY, float resZ) { _resX = resX; _resY = resY; _resZ = resZ; }

	/**
	 * Get the resolution of this discretized volume.
	 */
	float getResolutionX() const { return _resX; }
	float getResolutionY() const { return _resY; }
	float getResolutionZ() const { return _resZ; }

	/**
	 * Transform a real-valued volume location into discrete coordinates.
	 */
	void getDiscreteCoordinates(
			float x, float y, float z,
			unsigned int& dx, unsigned int& dy, unsigned int& dz) const {

		// remove offset
		x -= getBoundingBox().getMinX();
		y -= getBoundingBox().getMinY();
		z -= getBoundingBox().getMinZ();

		// discretize
		dx = x/getResolutionX();
		dy = y/getResolutionY();
		dz = z/getResolutionZ();
	}

	/**
	 * Transform discrete coordinates into a real-valued location.
	 */
	void getRealLocation(
			unsigned int dx, unsigned int dy, unsigned int dz,
			float& x, float& y, float& z)  const{

		x = dx*getResolutionX() + getBoundingBox().getMinX();
		y = dy*getResolutionY() + getBoundingBox().getMinY();
		z = dz*getResolutionZ() + getBoundingBox().getMinZ();
	}

private:

	float _resX;
	float _resY;
	float _resZ;
};

#endif // IMAGEPROCESSING_DISCRETIZATION_H__

