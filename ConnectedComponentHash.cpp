#include <algorithm>
#include <boost/functional/hash.hpp>
#include <util/foreach.h>
#include "ConnectedComponent.h"
#include "ConnectedComponentHash.h"

ConnectedComponentHash
hash_value(const ConnectedComponent& component) {

	ConnectedComponentHash hash = 0;

	foreach (const util::point<unsigned int>& pixel, component.getPixels()) {

		boost::hash_combine(hash, boost::hash_value(pixel.x));
		boost::hash_combine(hash, boost::hash_value(pixel.y));
	}

	return hash;
}
