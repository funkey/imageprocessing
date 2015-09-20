#include "ComponentTreeExtractor.h"

// explicit template instantiations
template class ComponentTreeExtractor<unsigned char>;
template class ComponentTreeExtractor<unsigned short>;

logger::LogChannel componenttreeextractorlog("componenttreeextractorlog", "[ComponentTreeExtractor] ");
