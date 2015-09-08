#ifndef IMAGEPROCESSING_INTERSECT_H__
#define IMAGEPROCESSING_INTERSECT_H__

#include "ExplicitVolume.h"
#include <util/assert.h>

/**
 * Stores the result of intersecting two volumes A and B in C. The intersection 
 * is the smallest box containing voxels that are non-zero in both A and B. The 
 * values of voxels in C are the minimal values of the corresponding voxels in A 
 * and B.
 *
 * A and B need to have the same resolution.
 *
 * @param[in] a
 *             The first volume.
 * @param[in] b
 *             The second volume.
 * @param[out] c
 *             The result of intersecting the given volumes.
 */
template <typename T>
void intersect(
		const ExplicitVolume<T>& a,
		const ExplicitVolume<T>& b,
		ExplicitVolume<T>& c) {

	UTIL_ASSERT_REL(a.getResolution(), ==, b.getResolution());

	// discrete offset from a to b
	util::point<int, 3> offsetAB = (a.getOffset() - b.getOffset())/a.getResolution();

	// get discrete bounding box of c relative to discrete bounding box of a
	util::box<int, 3> c_dbb;
	for (int z = 0; z < a.depth();  z++)
	for (int y = 0; y < a.height(); y++)
	for (int x = 0; x < a.width();  x++) {

		T valueA = a(x, y, z);

		if (valueA == 0)
			continue;

		util::point<int, 3> bpos(
				x + offsetAB.x(),
				y + offsetAB.y(),
				z + offsetAB.z());

		if (b.getDiscreteBoundingBox().contains(bpos) && b[bpos] > 0)
			c_dbb.fit(util::box<int, 3>(x, y, z, x+1, y+1, z+1));
	}

	c.resize(c_dbb.width(), c_dbb.height(), c_dbb.depth());
	c.data() = 0;
	c.setResolution(a.getResolution());
	c.setOffset(a.getOffset() + c_dbb.min()*a.getResolution());

	util::point<int, 3> offsetAC = -c_dbb.min();
	util::point<int, 3> offsetBC = -offsetAB + offsetAC;

	// fill non-zero values of c
	for (int z = 0; z < c.depth();  z++)
	for (int y = 0; y < c.height(); y++)
	for (int x = 0; x < c.width();  x++) {

		util::point<int, 3> posA(
				x - offsetAC.x(),
				y - offsetAC.y(),
				z - offsetAC.z());

		T valueA = a[posA];

		if (valueA == 0)
			continue;

		util::point<int, 3> posB(
				x - offsetBC.x(),
				y - offsetBC.y(),
				z - offsetBC.z());

		T valueB = b[posB];

		c(x, y, z) = std::min(valueA, valueB);
	}
}

#endif // IMAGEPROCESSING_INTERSECT_H__

