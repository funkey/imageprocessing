#include <limits>
#include "Skeletonize.h"
#include <vigra/multi_distance.hxx>
#include <vigra/multi_gridgraph.hxx>
#include <vigra/multi_labeling.hxx>
#include <vigra/multi_impex.hxx>
#include <util/timing.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>

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
		util::_description_text = "The mininal length of a segment (including the boundary penalty) as a ration of the largest segment to extract for a skeleton.",
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

util::ProgramOption optionSkeletonDownsampleVolume(
		util::_long_name        = "skeletonDownsampleVolume",
		util::_description_text = "downsample the volume dimensions by the largest power of two that does not change connectivity.");

logger::LogChannel skeletonizelog("skeletonizelog", "[Skeletonize] ");

Skeletonize::Skeletonize(ExplicitVolume<unsigned char>& volume) :
	_downSampleVolume(optionSkeletonDownsampleVolume),
	_positionMap(_graph),
	_distanceMap(_graph),
	_boundaryWeight(optionSkeletonBoundaryWeight),
	_dijkstra(_graph, _distanceMap),
	_minSegmentLength(optionSkeletonMinSegmentLength),
	_minSegmentLengthRatio(optionSkeletonMinSegmentLengthRatio),
	_skipExplainedNodes(optionSkeletonSkipExplainedNodes),
	_explanationWeight(optionSkeletonExplanationWeight) {

	Timer t(__FUNCTION__);

	setVolume(volume);
	createVoxelGraph();
}

void
Skeletonize::setVolume(const ExplicitVolume<unsigned char>& volume) {

	if (_downSampleVolume)
		downsampleVolume(volume);
	else
		_volume = volume;

	_boundaryDistance = vigra::MultiArray<3, float>(_volume.data().shape(), 0.0);
}

void
Skeletonize::downsampleVolume(const ExplicitVolume<unsigned char>& volume) {

	vigra::TinyVector<float, 3> origRes = {
			volume.getResolutionX(),
			volume.getResolutionY(),
			volume.getResolutionZ()};


	vigra::TinyVector<int, 3> origSize = {
			(int)volume.width(),
			(int)volume.height(),
			(int)volume.depth()};

	float finestRes = -1;
	int   finestDimension;
	for (int d = 0; d < 3; d++)
		if (finestRes < 0 || finestRes > origRes[d]) {

			finestRes = origRes[d];
			finestDimension = d;
		}

	// the largest downsample factor to consider
	int downsampleFactor = 8;

	bool goodLevelFound = false;
	while (!goodLevelFound) {

		LOG_DEBUG(skeletonizelog)
				<< "trying to downsample finest dimension by factor "
				<< downsampleFactor << std::endl;

		vigra::TinyVector<int, 3>   factors;
		vigra::TinyVector<float, 3> targetRes;
		vigra::TinyVector<int, 3>   targetSize;

		factors[finestDimension]    = downsampleFactor;
		targetRes[finestDimension]  = origRes[finestDimension]*downsampleFactor;
		targetSize[finestDimension] = origSize[finestDimension]/downsampleFactor;

		// the target resolution of the finest dimension, when downsampled with 
		// current factor
		float targetFinestRes = finestRes*downsampleFactor;

		// for each other dimension, find best downsample factor
		for (int d = 0; d < 3; d++) {

			if (d == finestDimension)
				continue;

			int bestFactor = 0;
			float minResDiff = 0;

			for (int f = downsampleFactor; f != 0; f /= 2) {

				float targetRes = origRes[d]*f;
				float resDiff = std::abs(targetFinestRes - targetRes);

				if (bestFactor == 0 || resDiff < minResDiff) {

					bestFactor = f;
					minResDiff = resDiff;
				}
			}

			factors[d]    = bestFactor;
			targetRes[d]  = origRes[d]*bestFactor;
			targetSize[d] = origSize[d]/bestFactor;
		}

		LOG_DEBUG(skeletonizelog)
				<< "best downsampling factors for each dimension are "
				<< factors << std::endl;

		_volume = ExplicitVolume<unsigned char>(targetSize[0], targetSize[1], targetSize[2]);
		_volume.setResolution(targetRes[0], targetRes[1], targetRes[2]);
		_volume.setBoundingBox(volume.getBoundingBox());

		// copy volume
		for (int z = 0; z < targetSize[2]; z++)
		for (int y = 0; y < targetSize[1]; y++)
		for (int x = 0; x < targetSize[0]; x++)
			_volume(x, y, z) = volume(x*factors[0], y*factors[1], z*factors[2]);

		int numRegions;
		try {

			// check for downsampling errors
			vigra::MultiArray<3, unsigned int> labels(_volume.data().shape());
			numRegions = vigra::labelMultiArrayWithBackground(
					_volume.data(),
					labels);

		} catch (vigra::InvariantViolation& e) {

			LOG_DEBUG(skeletonizelog)
					<< "downsampled image contains more than 255 connected components"
					<< std::endl;

			numRegions = 2;
		}

		LOG_DEBUG(skeletonizelog)
				<< "downsampled image contains " << numRegions
				<< " connected components" << std::endl;

		if (numRegions == 1)
			goodLevelFound = true;
		else
			downsampleFactor /= 2;
	}
}

void
Skeletonize::createVoxelGraph() {

	vigra::MultiArray<3, Graph::Node> nodeIds(_volume.data().shape());
	vigra::GridGraph<3> grid(_volume.data().shape(), vigra::IndirectNeighborhood);

	// add all non-background nodes
	for (vigra::GridGraph<3>::NodeIt node(grid); node != lemon::INVALID; ++node) {

		if (_volume[node] == Background)
			continue;

		Graph::Node n   = _graph.addNode();
		nodeIds[node]   = n;
		_positionMap[n] = *node;
	}

	// add all edges between non-background nodes and label boundary nodes 
	// on-the-fly
	for (vigra::GridGraph<3>::EdgeIt edge(grid); edge != lemon::INVALID; ++edge) {

		int insideVoxels = (_volume[grid.u(edge)] != Background) + (_volume[grid.v(edge)] != Background);

		if (insideVoxels == 1) {

			if (_volume[grid.u(edge)] != Background)
				_volume[grid.u(edge)] = Boundary;
			if (_volume[grid.v(edge)] != Background)
				_volume[grid.v(edge)] = Boundary;
		}

		if (insideVoxels != 2)
			continue;

		Graph::Node u = nodeIds[grid.u(edge)];
		Graph::Node v = nodeIds[grid.v(edge)];

		_graph.addEdge(u, v);
	}

	// get a list of boundary nodes
	for (Graph::NodeIt node(_graph); node != lemon::INVALID; ++node)
		if (_volume[_positionMap[node]] == Boundary)
			_boundary.push_back(node);
}

Skeleton
Skeletonize::getSkeleton() {

	Timer t(__FUNCTION__);

	initializeEdgeMap();

	Graph::Node root = findRoot();

	setRoot(root);

	int maxNumSegments = optionSkeletonMaxNumSegments;
	int segmentsFound = 0;
	while (extractLongestSegment() && ++segmentsFound < maxNumSegments) {}

	return parseVolumeSkeleton();
}

void
Skeletonize::initializeEdgeMap() {

	Timer t(__FUNCTION__);

	// We assume the pitch vigra needs is the number of measurements per unit. 
	// Our units are nm, and the volume tells us via getResolution?() the size 
	// of a pixel. Hence, the number of measurements per nm in either direction 
	// is 1/resolution of this direction.
	float pitch[3];
	pitch[0] = 1.0/_volume.getResolutionX();
	pitch[1] = 1.0/_volume.getResolutionY();
	pitch[2] = 1.0/_volume.getResolutionZ();

	vigra::separableMultiDistSquared(
			_volume.data(),
			_boundaryDistance,
			false,  /* compute distance from object (non-zero) to background (0) */
			pitch);

	// find center point with maximal boundary distance
	_maxBoundaryDistance2 = 0;
	for (Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		const Position& pos = _positionMap[node];
		if (_boundaryDistance[pos] > _maxBoundaryDistance2) {

			_center = node;
			_maxBoundaryDistance2 = _boundaryDistance[pos];
		}
	}

	using namespace vigra::functor;

	// create initial edge map from boundary penalty
	for (Graph::EdgeIt e(_graph); e != lemon::INVALID; ++e)
		_distanceMap[e] = boundaryPenalty(
				0.5*(
						_boundaryDistance[_positionMap[_graph.u(e)]] +
						_boundaryDistance[_positionMap[_graph.v(e)]]));

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
	nodeDistances[1] = _volume.getResolutionZ();
	nodeDistances[2] = _volume.getResolutionY();
	nodeDistances[3] = sqrt(pow(_volume.getResolutionY(), 2) + pow(_volume.getResolutionZ(), 2));
	nodeDistances[4] = _volume.getResolutionX();
	nodeDistances[5] = sqrt(pow(_volume.getResolutionX(), 2) + pow(_volume.getResolutionZ(), 2));
	nodeDistances[6] = sqrt(pow(_volume.getResolutionX(), 2) + pow(_volume.getResolutionY(), 2));
	nodeDistances[7] = sqrt(pow(_volume.getResolutionX(), 2) + pow(_volume.getResolutionY(), 2) + pow(_volume.getResolutionZ(), 2));

	for (Graph::EdgeIt e(_graph); e != lemon::INVALID; ++e) {

		Position u = _positionMap[_graph.u(e)];
		Position v = _positionMap[_graph.v(e)];

		int i = 0;
		if (u[0] != v[0]) i |= 4;
		if (u[1] != v[1]) i |= 2;
		if (u[2] != v[2]) i |= 1;

		_distanceMap[e] = nodeDistances[i]*(_distanceMap[e] + 1);
	}
}

Skeletonize::Graph::Node
Skeletonize::findRoot() {

	Timer t(__FUNCTION__);

	_dijkstra.run(_center);

	// find furthest point on boundary
	Graph::Node root = Graph::NodeIt(_graph);
	float maxValue = -1;
	for (Graph::Node n : _boundary)
		if (_dijkstra.distMap()[n] > maxValue) {

			root     = n;
			maxValue = _dijkstra.distMap()[n];
		}

	if (maxValue == -1)
		UTIL_THROW_EXCEPTION(
				NoNodeFound,
				"could not find a root boundary point");

	// mark root as being part of skeleton
	_volume[_positionMap[root]] = OnSkeleton;

	return root;
}

bool
Skeletonize::extractLongestSegment() {

	Timer t(__FUNCTION__);

	_dijkstra.run(_root);

	// find furthest point on boundary
	Graph::Node furthest = Graph::NodeIt(_graph);
	float maxValue = -1;
	for (Graph::Node n : _boundary) {

		if (_skipExplainedNodes && _volume[_positionMap[n]] == Explained)
			continue;

		if (_dijkstra.distMap()[n] > maxValue) {

			furthest = n;
			maxValue = _dijkstra.distMap()[n];
		}
	}

	// no more points or length smaller then min segment length
	if (maxValue == -1 || maxValue < _minSegmentLength)
		return false;

	LOG_DEBUG(skeletonizelog) << "extracting segment with length " << maxValue << std::endl;

	Graph::Node n = furthest;

	// walk backwards to next skeleton point
	while (_volume[_positionMap[n]] != OnSkeleton) {

		_volume[_positionMap[n]] = OnSkeleton;

		if (_skipExplainedNodes)
			drawExplanationSphere(_positionMap[n]);

		Graph::Edge pred = _dijkstra.predMap()[n];
		Graph::Node u = _graph.u(pred);
		Graph::Node v = _graph.v(pred);

		n = (u == n ? v : u);

		_distanceMap[pred] = 0.0;
	}

	// first segment?
	if (n == _root) {

		LOG_DEBUG(skeletonizelog) << "longest segment has length " << maxValue << std::endl;

		_minSegmentLength = std::max(_minSegmentLength, _minSegmentLengthRatio*maxValue);

		LOG_DEBUG(skeletonizelog) << "setting min segment length to " << _minSegmentLength << std::endl;
	}

	return true;
}

void
Skeletonize::drawExplanationSphere(const Position& center) {

	double radius2 = _boundaryDistance[center]*pow(_explanationWeight, 2);

	double resX2 = pow(_volume.getResolutionX(), 2);
	double resY2 = pow(_volume.getResolutionY(), 2);
	double resZ2 = pow(_volume.getResolutionZ(), 2);

	for (Graph::Node n : _boundary) {

		const Position& pos = _positionMap[n];
		double distance2 =
				resX2*pow(pos[0] - center[0], 2) +
				resY2*pow(pos[1] - center[1], 2) +
				resZ2*pow(pos[2] - center[2], 2);

		if (distance2 <= radius2)
			if (_volume[pos] != OnSkeleton)
				_volume[pos] = Explained;
	}
}

double
Skeletonize::boundaryPenalty(double boundaryDistance) {

	// penalty = w*(1.0 - bd/max_bd)
	//
	//   w     : boundary weight
	//   bd    : boundary distance
	//   max_bd: max boundary distance
	return _boundaryWeight*(1.0 - sqrt(boundaryDistance/_maxBoundaryDistance2));
}

Skeleton
Skeletonize::parseVolumeSkeleton() {

	Skeleton skeleton;

	skeleton.setBoundingBox(_volume.getBoundingBox());

	traverse(_positionMap[_root], skeleton);

	return skeleton;
}

void
Skeletonize::traverse(const Position& pos, Skeleton& skeleton) {

	_volume[pos] = Visited;

	float x, y, z;
	_volume.getRealLocation(pos[0], pos[1], pos[2], x, y, z);
	Skeleton::Position realPos(x, y, z);

	int neighbors = numNeighbors(pos);
	bool isNode = (neighbors != 2);

	if (isNode)
		skeleton.openNode(realPos);
	else
		skeleton.extendEdge(realPos);

	int sx = (pos[0] == 0 ? 0 : -1);
	int sy = (pos[1] == 0 ? 0 : -1);
	int sz = (pos[2] == 0 ? 0 : -1);
	int ex = (pos[0] == _volume.width()  - 1 ? 0 : 1);
	int ey = (pos[1] == _volume.height() - 1 ? 0 : 1);
	int ez = (pos[2] == _volume.depth()  - 1 ? 0 : 1);

	// as soon as 'neighbors' is negative, we know that there are no more 
	// neighbors left to test (0 is not sufficient, since we count ourselves as 
	// well)
	for (int dz = sz; dz <= ez && neighbors >= 0; dz++)
	for (int dy = sy; dy <= ey && neighbors >= 0; dy++)
	for (int dx = sx; dx <= ex && neighbors >= 0; dx++) {

		vigra::Shape3 p = pos + vigra::Shape3(dx, dy, dz);

		if (_volume[p] >= OnSkeleton) {

			neighbors--;

			if (_volume[p] != Visited)
				traverse(p, skeleton);
		}
	}

	if (isNode)
		skeleton.closeNode();
}

int
Skeletonize::numNeighbors(const Position& pos) {

	int num = 0;

	int sx = (pos[0] == 0 ? 0 : -1);
	int sy = (pos[1] == 0 ? 0 : -1);
	int sz = (pos[2] == 0 ? 0 : -1);
	int ex = (pos[0] == _volume.width()  - 1 ? 0 : 1);
	int ey = (pos[1] == _volume.height() - 1 ? 0 : 1);
	int ez = (pos[2] == _volume.depth()  - 1 ? 0 : 1);

	for (int dz = sz; dz <= ez; dz++)
	for (int dy = sy; dy <= ey; dy++)
	for (int dx = sx; dx <= ex; dx++) {

		if (_volume(pos[0] + dx, pos[1] + dy, pos[2] + dz) >= OnSkeleton)
			num++;
	}

	if (_volume[pos] >= OnSkeleton)
		num--;

	return num;
}

