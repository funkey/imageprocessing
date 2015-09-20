#include <gui/OpenGl.h>
#include <util/point.hpp>
#include <util/foreach.h>
#include <util/Logger.h>
#include "ComponentTreePainter.h"

static logger::LogChannel componenttreepainterlog("componenttreepainterlog", "[ComponentTreePainter] ");

void
ComponentTreePainter::setComponentTree(boost::shared_ptr<ComponentTree> componentTree) {

	_componentTree = componentTree;

	updateRecording();

	LOG_DEBUG(componenttreepainterlog) << "update size to " << _componentTree->getBoundingBox() << std::endl;

	setSize(_componentTree->getBoundingBox());
}

void
ComponentTreePainter::updateRecording() {

	if (!_componentTree)
		return;

	// make sure OpenGl operations are save
	gui::OpenGl::Guard guard;

	startRecording();

	// enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// always draw the components
	glDisable(GL_CULL_FACE);

	// draw each connected component except for the fake root node
	ComponentPaintVisitor painter;

	foreach (boost::shared_ptr<ComponentTree::Node> node, _componentTree->getRoot()->getChildren())
		_componentTree->visit(node, painter);

	// use the root node to draw the whole pixel list
	drawPixelList(_componentTree->getRoot());

	glDisable(GL_BLEND);

	stopRecording();
}

void
ComponentTreePainter::drawPixelList(boost::shared_ptr<ComponentTree::Node> root) {

	boost::shared_ptr<ConnectedComponent> component = root->getComponent();

	glDisable(GL_DEPTH_TEST);

	util::point<double> previous(0, 0);
	foreach (util::point<unsigned int> p, component->getPixels()) {

		// go almost all the way
		util::point<double> target = previous + (util::point<double>(p) - previous)*0.9;

		glColor4f(1, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2d(previous.x  + 0.5, previous.y + 0.5);
		glVertex2d(target.x  + 0.5, target.y + 0.5);
		glColor4f(0, 1, 0, 1);
		glVertex2d(target.x  + 0.5, target.y + 0.5);
		glVertex2d(p.x  + 0.5, p.y + 0.5);
		glEnd();

		previous = p;
	}
}

ComponentTreePainter::ComponentPaintVisitor::ComponentPaintVisitor() :
	_zScale(-50.0) {}

void
ComponentTreePainter::ComponentPaintVisitor::visitNode(boost::shared_ptr<ComponentTree::Node> node) {

	boost::shared_ptr<ConnectedComponent> component = node->getComponent();

	double value = component->getValue();

	LOG_ALL(componenttreepainterlog) << "drawing component with " << (component->getPixels().second - component->getPixels().first) << " pixels" << std::endl;

	glEnable(GL_DEPTH_TEST);
	glColor4f(value, value, value, 0.5);

	foreach (util::point<unsigned int> p, component->getPixels()) {

		glBegin(GL_QUADS);
		glVertex3d(p.x,     p.y,     value*_zScale);
		glVertex3d(p.x + 1, p.y,     value*_zScale);
		glVertex3d(p.x + 1, p.y + 1, value*_zScale);
		glVertex3d(p.x,     p.y + 1, value*_zScale);
		glEnd();
	}
}

void
ComponentTreePainter::ComponentPaintVisitor::visitEdge(
		boost::shared_ptr<ComponentTree::Node> parent,
		boost::shared_ptr<ComponentTree::Node> child) const {

	double parentValue = parent->getComponent()->getValue();
	double childValue  = child->getComponent()->getValue();

	const util::point<double> parentCenter = parent->getComponent()->getCenter();
	const util::point<double> childCenter  = child->getComponent()->getCenter();

	LOG_ALL(componenttreepainterlog) << "drawing edge from " << parentCenter << " to " << childCenter << std::endl;

	glColor4f(0.0, 0.0, 0.0, 1.0);

	glBegin(GL_LINES);
	glVertex3d(parentCenter.x, parentCenter.y, parentValue*_zScale);
	glVertex3d(childCenter.x,  childCenter.y,   childValue*_zScale);
	glEnd();
}
