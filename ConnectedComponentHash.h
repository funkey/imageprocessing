#ifndef IMAGEPROCESSING_CONNECTED_COMPONENT_HASH_H__
#define IMAGEPROCESSING_CONNECTED_COMPONENT_HASH_H__

#include <cstddef>

typedef std::size_t ConnectedComponentHash;

// forward declaration
class ConnectedComponent;

ConnectedComponentHash hash_value(const ConnectedComponent& component);

#endif // IMAGEPROCESSING_CONNECTED_COMPONENT_HASH_H__

