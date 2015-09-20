#include <util/Logger.h>
#include "ComponentTreeView.h"

static logger::LogChannel componenttreeviewlog("componenttreeviewlog", "[ComponentTreeView] ");

ComponentTreeView::ComponentTreeView() {

	registerInput(_componentTree, "component tree");
	registerOutput(_painter, "painter");

	_painter.registerSlot(_contentChanged);
	_painter.registerSlot(_sizeChanged);
}

void
ComponentTreeView::updateOutputs() {

	if (!_painter)
		_painter = new ComponentTreePainter();

	util::rect<double> oldSize = _painter->getSize();

	_painter->setComponentTree(_componentTree);

	if (_painter->getSize() == oldSize)
		_contentChanged();
	else
		_sizeChanged(_painter->getSize());
}
