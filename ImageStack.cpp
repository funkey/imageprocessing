#include "ImageStack.h"

void
ImageStack::clear() {

	_sections.clear();
}

void
ImageStack::add(std::shared_ptr<Image> section) {

	_sections.push_back(section);
}

void
ImageStack::addAll(std::shared_ptr<ImageStack> sections) {

	_sections.insert(_sections.end(), sections->begin(), sections->end());
}
