#include <limits>
#include "Skeletonize.h"
#include <vigra/multi_distance.hxx>
#include <vigra/multi_gridgraph.hxx>
#include <vigra/multi_labeling.hxx>
#include <vigra/multi_impex.hxx>
#include <util/timing.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>

#if defined(USE_PROGRAM_OPTIONS)
util::ProgramOption optionSkeletonBoundaryWeight(
		util::_long_name        = "skeletonBoundaryWeight",
		util::_description_text = "The weight of the boundary term to find the tube's skeletons.",
		util::_default_value    = 1);

util::ProgramOption optionSkeletonMaxNumSegments(
		util::_long_name        = "skeletonMaxNumSegments",
		util::_description_text = "The maximal number of segments to extract for a skeleton.",
		util::_default_value    = 10);

util::ProgramOption optionSkeletonMinSegmentLength(
		util::_long_name        = "skeletonMinSegmentLength",
		util::_description_text = "The mininal length of a segment (including the boundary penalty) to extract for a skeleton.",
		util::_default_value    = 0);

util::ProgramOption optionSkeletonMinSegmentLengthRatio(
		util::_long_name        = "skeletonMinSegmentLengthRatio",
		util::_description_text = "The mininal length of a segment (including the boundary penalty) as the ratio of the largest segment extracted for a skeleton.",
		util::_default_value    = 1);

util::ProgramOption optionSkeletonSkipExplainedNodes(
		util::_long_name        = "skeletonSkipExplainedNodes",
		util::_description_text = "Don't add segments to nodes that are already explained by the current skeleton. "
		                          "Nodes are explained, if they fall within a sphere around any current skeleton node. "
		                          "The size of the sphere is determined by boundary distance * skeletonExplanationWeight.");

util::ProgramOption optionSkeletonExplanationWeight(
		util::_long_name        = "skeletonExplanationWeight",
		util::_description_text = "A factor to multiply with the boundary distance to create 'explanation spheres'. "
		                          "See skeletonSkipExplainedNodes.",
		util::_default_value    = 1);
#endif

logger::LogChannel skeletonizelog("skeletonizelog", "[Skeletonize] ");

Skeletonize::Skeletonize(const GraphVolume& graphVolume) :
	_boundaryDistance(
			vigra::Shape3(
					graphVolume.getDiscreteBoundingBox().width()  + 2,
					graphVolume.getDiscreteBoundingBox().height() + 2,
					graphVolume.getDiscreteBoundingBox().depth()  + 2
			)),
	_graphVolume(graphVolume),
	_distanceMap(_graphVolume.graph()),
	_dijkstra(_graphVolume.graph(), _distanceMap),
	_nodeLabels(_graphVolume.graph(), Inside)
	{ 	
	#if defined(USE_PROGRAM_OPTIONS)
	_parameters.boundaryWeight = optionSkeletonBoundaryWeight;
	_parameters.minSegmentLength = optionSkeletonMinSegmentLength;
	_parameters.minSegmentLengthRatio = optionSkeletonMinSegmentLengthRatio;
	_parameters.skipExplainedNodes = optionSkeletonSkipExplainedNodes;
	_parameters.explanationWeight = optionSkeletonExplanationWeight;
	_parameters.maxNumSegments = optionSkeletonMaxNumSegments;
	#endif
	}

Skeletonize::Skeletonize(const GraphVolume& graphVolume, Parameters user_parameters) :
 	_boundaryDistance(
			vigra::Shape3(
 					graphVolume.getDiscreteBoundingBox().width()  + 2,
 					graphVolume.getDiscreteBoundingBox().height() + 2,
 					graphVolume.getDiscreteBoundingBox().depth()  + 2
 			)),
	_graphVolume(graphVolume),
	_distanceMap(_graphVolume.graph()),
	_dijkstra(_graphVolume.graph(), _distanceMap),
	_nodeLabels(_graphVolume.graph(), Inside),
	_parameters(user_parameters) {}


Skeleton
Skeletonize::getSkeleton() {

	UTIL_TIME_METHOD;

	findBoundaryNodes();

	initializeEdgeMap();

	findRoot();

 	for (int i = 0; i < _parameters.maxNumSegments; ++i) {
 		if (!extractLongestSegment()) break;
	}
	return parseVolumeSkeleton();
}

void
Skeletonize::findBoundaryNodes() {

	for (GraphVolume::NodeIt node(_graphVolume.graph()); node != lemon::INVALID; ++node) {

		int numNeighbors = 0;
		for (GraphVolume::IncEdgeIt e(_graphVolume.graph(), node); e != lemon::INVALID; ++e)
			numNeighbors++;

		if (numNeighbors != GraphVolume::NumNeighbors) {

			_boundary.push_back(node);
			_nodeLabels[node] = Boundary;
		}
	}
}

void
Skeletonize::initializeEdgeMap() {

	// the pitch is the number of units per voxel dimension
	float pitch[3];
	pitch[0] = _graphVolume.getResolutionX();
	pitch[1] = _graphVolume.getResolutionY();
	pitch[2] = _graphVolume.getResolutionZ();

	_boundaryDistance = 0;
	for (GraphVolume::NodeIt n(_graphVolume.graph()); n != lemon::INVALID; ++n)
		boundaryDistance(_graphVolume.positions()[n]) = 1.0;

	if (_graphVolume.getDiscreteBoundingBox().depth() == 1) {

		LOG_DEBUG(skeletonizelog) << "performing 2D distance transform for boundary penalty" << std::endl;

		// perform 2D distance transform if depth is 1
		vigra::separableMultiDistSquared(
				_boundaryDistance.bind<2>(1), // only on center section (0 and 2 are padded)
				_boundaryDistance.bind<2>(1),
				false,  /* compute distance from object (non-zero) to background (0) */
				pitch);

	} else {

		vigra::separableMultiDistSquared(
				_boundaryDistance,
				_boundaryDistance,
				false,  /* compute distance from object (non-zero) to background (0) */
				pitch);
	}

	// find center point with maximal boundary distance
	_maxBoundaryDistance2 = 0;
	for (GraphVolume::NodeIt node(_graphVolume.graph()); node != lemon::INVALID; ++node) {

		const Position& pos = _graphVolume.positions()[node];
		if (boundaryDistance(pos) > _maxBoundaryDistance2) {

			_center = node;
			_maxBoundaryDistance2 = boundaryDistance(pos);
		}
	}

	// create initial edge map from boundary penalty
	for (GraphVolume::EdgeIt e(_graphVolume.graph()); e != lemon::INVALID; ++e)
		_distanceMap[e] = boundaryPenalty(
				0.5*(
						boundaryDistance(_graphVolume.positions()[_graphVolume.graph().u(e)]) +
						boundaryDistance(_graphVolume.positions()[_graphVolume.graph().v(e)])));

	// multiply with Euclidean node distances
	//
	// The TEASAR paper suggests to add the Euclidean distances. However, for 
	// the penalty to be meaningful in anistotropic volumes, it should be 
	// multiplied with the Euclidean distance between the nodes (otherwise, it 
	// is more expensive to move in the high-resolution dimensions). Therefore, 
	// the final value is
	//
	//   penalty*euclidean + euclidean = euclidean*(penalty + 1)

	float nodeDistances[8];
	nodeDistances[0] = 0;
	nodeDistances[1] = _graphVolume.getResolutionZ();
	nodeDistances[2] = _graphVolume.getResolutionY();
	nodeDistances[3] = sqrt(pow(_graphVolume.getResolutionY(), 2) + pow(_graphVolume.getResolutionZ(), 2));
	nodeDistances[4] = _graphVolume.getResolutionX();
	nodeDistances[5] = sqrt(pow(_graphVolume.getResolutionX(), 2) + pow(_graphVolume.getResolutionZ(), 2));
	nodeDistances[6] = sqrt(pow(_graphVolume.getResolutionX(), 2) + pow(_graphVolume.getResolutionY(), 2));
	nodeDistances[7] = sqrt(pow(_graphVolume.getResolutionX(), 2) + pow(_graphVolume.getResolutionY(), 2) + pow(_graphVolume.getResolutionZ(), 2));

	for (GraphVolume::EdgeIt e(_graphVolume.graph()); e != lemon::INVALID; ++e) {

		Position u = _graphVolume.positions()[_graphVolume.graph().u(e)];
		Position v = _graphVolume.positions()[_graphVolume.graph().v(e)];

		int i = 0;
		if (u[0] != v[0]) i |= 4;
		if (u[1] != v[1]) i |= 2;
		if (u[2] != v[2]) i |= 1;

		_distanceMap[e] = nodeDistances[i]*(_distanceMap[e] + 1);
	}
}

void
Skeletonize::findRoot() {

	_dijkstra.run(_center);

	// find furthest point on boundary
	_root = GraphVolume::NodeIt(_graphVolume.graph());
	float maxValue = -1;
	for (GraphVolume::Node n : _boundary) {
		if (_dijkstra.distMap()[n] > maxValue) {

			_root    = n;
			maxValue = _dijkstra.distMap()[n];
		}
	}

	if (maxValue == -1)
		UTIL_THROW_EXCEPTION(
				NoNodeFound,
				"could not find a root boundary point");

	// mark root as being part of skeleton
	_nodeLabels[_root] = OnSkeleton;
}

bool
Skeletonize::extractLongestSegment() {

	_dijkstra.run(_root);

	// find furthest point on boundary
	GraphVolume::Node furthest = GraphVolume::NodeIt(_graphVolume.graph());
	float maxValue = -1;
	for (GraphVolume::Node n : _boundary) {

		if (_parameters.skipExplainedNodes && _nodeLabels[n] == Explained)
			continue;

		if (_dijkstra.distMap()[n] > maxValue) {

			furthest = n;
			maxValue = _dijkstra.distMap()[n];
		}
	}

	// no more points or length smaller then min segment length
	if (maxValue == -1 || maxValue < _parameters.minSegmentLength)
		return false;

	LOG_DEBUG(skeletonizelog) << "extracting segment with length " << maxValue << std::endl;

	GraphVolume::Node n = furthest;

	// walk backwards to next skeleton point
	while (_nodeLabels[n] != OnSkeleton) {

		_nodeLabels[n] = OnSkeleton;

		if (_parameters.skipExplainedNodes)
			drawExplanationSphere(_graphVolume.positions()[n]);

		GraphVolume::Edge pred = _dijkstra.predMap()[n];
		GraphVolume::Node u = _graphVolume.graph().u(pred);
		GraphVolume::Node v = _graphVolume.graph().v(pred);

		n = (u == n ? v : u);

		_distanceMap[pred] = 0.0;
	}

	// first segment?
	if (n == _root) {

		LOG_DEBUG(skeletonizelog) << "longest segment has length " << maxValue << std::endl;

		_parameters.minSegmentLength = std::max(_parameters.minSegmentLength, 					_parameters.minSegmentLengthRatio*maxValue);

		LOG_DEBUG(skeletonizelog) << "setting min segment length to " << _parameters.minSegmentLength << 			std::endl;
	}

	return true;
}

void
Skeletonize::drawExplanationSphere(const Position& center) {

	double radius2 = boundaryDistance(center)*pow(_parameters.explanationWeight, 2);

	double resX2 = pow(_graphVolume.getResolutionX(), 2);
	double resY2 = pow(_graphVolume.getResolutionY(), 2);
	double resZ2 = pow(_graphVolume.getResolutionZ(), 2);

	for (GraphVolume::Node n : _boundary) {

		const Position& pos = _graphVolume.positions()[n];
		double distance2 =
 				resX2*pow(static_cast<double>(pos[0]) - static_cast<double>(center[0]), 2) +
 				resY2*pow(static_cast<double>(pos[1]) - static_cast<double>(center[1]), 2) +
 				resZ2*pow(static_cast<double>(pos[2]) - static_cast<double>(center[2]), 2);
		if (distance2 <= radius2)
			if (_nodeLabels[n] != OnSkeleton)
				_nodeLabels[n] = Explained;
	}
}

double
Skeletonize::boundaryPenalty(double boundaryDistance) {

	// penalty = w*(1.0 - bd/max_bd)
	//
	//   w     : boundary weight
	//   bd    : boundary distance
	//   max_bd: max boundary distance
	return _parameters.boundaryWeight*(1.0 - sqrt(boundaryDistance/_maxBoundaryDistance2));
}

Skeleton
Skeletonize::parseVolumeSkeleton() {

	Skeleton skeleton;

	skeleton.setOffset(_graphVolume.getOffset());
	skeleton.setResolution(_graphVolume.getResolution());

	traverse(_root, skeleton);

	return skeleton;
}

// The old version of this was recursing directly and deeply, leading to stack
// overflows in stack restricted environments (e.g. multithreading).  The
// current version is a fairly direct iteratization of the old, directly
// recursive version.
void
Skeletonize::traverse(const GraphVolume::Node& root, Skeleton& skeleton) {
	// DFS of nodes from root.  Data-wise, Nodes are just integer values.
	std::stack<GraphVolume::Node> traversal;
	traversal.push(root);
	while (!traversal.empty()) {
		const GraphVolume::Node n = traversal.top();
		const int nNeighbors = numNeighbors(n);

		// Special nodes that open new segments.
		const bool isOpeningNode = n == _root || nNeighbors != 2;

		// The second time we see a node, we are in back-traversal, popping from
		// traversal stack and potentially closing segments.
		if (_nodeLabels[n] == Visited) {
			if (isOpeningNode) skeleton.closeSegment();
			traversal.pop();
			continue;
		}

		// Otherwise, we're seeing the node for the first time, so opening /
		// extending segment.
		_nodeLabels[n] = Visited;
		const Position pos = _graphVolume.positions()[n];
		const float boundDist = sqrt(boundaryDistance(pos));
		if (isOpeningNode) {
			skeleton.openSegment(pos, 2*boundDist);
		} else {
			skeleton.extendSegment(pos, 2*boundDist);
		}

		// Iterate through neighbors and put unseen ones onto traversal stack.  The
		// loop checks against nNeighbors to allow early termination.
		GraphVolume::IncEdgeIt e(_graphVolume.graph(), n);
		for (int i = 0; i < nNeighbors; ++e /* increment e, not i */) {
			assert(e != lemon::INVALID);  // Should never occur.

			// Only increment i if we are using this edge.
			if (_distanceMap[e] != 0.0) continue;
			++i;

			const GraphVolume::Node neighbor = (_graphVolume.graph().u(e) == n ? _graphVolume.graph().v(e) : _graphVolume.graph().u(e));
			if (_nodeLabels[neighbor] != Visited) traversal.push(neighbor);
		}
	}
}

int
Skeletonize::numNeighbors(const GraphVolume::Node& n) {

	int num = 0;

	for (GraphVolume::IncEdgeIt e(_graphVolume.graph(), n); e != lemon::INVALID; ++e)
		if (_distanceMap[e] == 0.0)
			num++;

	return num;
}

