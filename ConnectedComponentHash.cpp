#include <algorithm>
#include <boost/functional/hash.hpp>
#include <util/foreach.h>
#include "ConnectedComponent.h"
#include "ConnectedComponentHash.h"

ConnectedComponentHash
hash_value(const ConnectedComponent& component) {

	ConnectedComponentHash hash = 0;

	typedef util::point<unsigned int,2> point2;
	foreach (const point2& pixel, component.getPixels()) {

		boost::hash_combine(hash, boost::hash_value(pixel.x()));
		boost::hash_combine(hash, boost::hash_value(pixel.y()));
	}

	const ConnectedComponent::bitmap_type& bitmap      = component.getBitmap();
	const util::rect<unsigned int>&        boundingBox = component.getBoundingBox();

	for (int x = 0; x < bitmap.width(); ++x) {
		for (int y = 0; y < bitmap.height(); ++y) {
			if (bitmap(x, y)) {

				boost::hash_combine(hash, boost::hash_value(x + boundingBox.minX));
				boost::hash_combine(hash, boost::hash_value(y + boundingBox.minY));
			}
		}
	}

	return hash;
}
