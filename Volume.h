#ifndef IMAGEPROCESSING_VOLUME_H__
#define IMAGEPROCESSING_VOLUME_H__

/**
 * Base for classes representing a 3D volume.
 */
class Volume {

public:

	Volume() :
		_resX(1.0),
		_resY(1.0),
		_resZ(1.0),
		_minX(0.0),
		_minY(0.0),
		_minZ(0.0) {}

	void setResolution(float resX, float resY, float resZ) { _resX = resX; _resY = resY; _resZ = resZ; }

	float getResolutionX() { return _resX; }
	float getResolutionY() { return _resY; }
	float getResolutionZ() { return _resZ; }

	void setOffset(float minX, float minY, float minZ) { _minX = minX; _minY = minY; _minZ = minZ; }

	float getOffsetX() { return _minX; }
	float getOffsetY() { return _minY; }
	float getOffsetZ() { return _minZ; }

private:

	float _resX;
	float _resY;
	float _resZ;

	float _minX;
	float _minY;
	float _minZ;
};


#endif // IMAGEPROCESSING_VOLUME_H__

