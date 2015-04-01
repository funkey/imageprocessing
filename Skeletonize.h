#ifndef IMAGEPROCESSING_TUBES_SKELETONIZE_H__
#define IMAGEPROCESSING_TUBES_SKELETONIZE_H__

#include <imageprocessing/ExplicitVolume.h>
#define WITH_LEMON
#include <lemon/dijkstra.h>
#include "Skeleton.h"

class NoNodeFound : public Exception {};

class Skeletonize {

	typedef vigra::MultiArray<3, unsigned char> VolumeType;
	typedef VolumeType::difference_type         Position;
	typedef lemon::ListGraph                    Graph;
	typedef Graph::NodeMap<Position>            PositionMap;
	typedef Graph::EdgeMap<double>              DistanceMap;

public:

	/**
	 * Create a skeletonizer for the given volume. Inside voxels are assumed to 
	 * be labelled with 1, background with 0.
	 */
	Skeletonize(ExplicitVolume<unsigned char>& volume);

	/**
	 * Extract the skeleton from the given volume.
	 */
	Skeleton getSkeleton();

private:

	enum VoxelLabel {

		Background = 0,
		Inside     = 1, /* ordinary inside voxels and initial values */
		Boundary   = 2, /* inside voxels on boundary */
		Explained  = 3, /* boundary voxels that are within a threshold distance to skeleton voxels */
		OnSkeleton = 4, /* skeleton voxels */
		Visited    = 5  /* skeleton voxels that have been added to Skeleton datastructure (eventually, all OnSkeleton voxels)*/
	};

	/**
	 * Set the volume to process, try to downsample it on the fly.
	 */
	void setVolume(const ExplicitVolume<unsigned char>& volume);

	void downsampleVolume(const ExplicitVolume<unsigned char>& volume);

	/**
	 * Create the voxel graph to use for skeletonization.
	 */
	void createVoxelGraph();

	/**
	 * Initialize the edge map, such that initial edges inside the volume are 
	 * Euclidean distance plus boundary distance penalty, and all other ones 
	 * infinite.
	 */
	void initializeEdgeMap();

	/**
	 * Find the root node as the furthest point from the highest boundary 
	 * distance point.
	 */
	Graph::Node findRoot();

	/**
	 * Set the root node of the skeleton. This should be a point with maximal 
	 * distance to some internal point.
	 */
	void setRoot(Graph::Node root) { _root = root; }

	/**
	 * Compute or update the shortest paths from the root node to all other 
	 * points. Takes the current edge map for distances.
	 */
	void findShortestPaths();

	/**
	 * Find the furthest point and walk backwards along the shortest path to the 
	 * current skeleton. This marks all points on the path as being part of the 
	 * skeleton, and all points in the vicinity as beeing processed. The edge 
	 * values along the shortest path will be set to zero. Additionally, the 
	 * segment will be added to the passed skeleton.
	 *
	 * Returns true, if a path that is far enough from the existing skeleton was 
	 * found.
	 */
	bool extractLongestSegment();

	/**
	 * Draw a sphere around the current point, marking all boundary points 
	 * within it as explained.
	 */
	void drawExplanationSphere(const Position& center);

	/**
	 * Convert grid positions to volume positions.
	 */
	Skeleton::Position gridToVolume(Position pos) {

		return Skeleton::Position(
				_volume.getBoundingBox().minX + (float)pos[0]*_volume.getResolutionX(),
				_volume.getBoundingBox().minY + (float)pos[1]*_volume.getResolutionY(),
				_volume.getBoundingBox().minZ + (float)pos[2]*_volume.getResolutionZ());
	}

	/**
	 * Compute the boundary penalty term.
	 */
	double boundaryPenalty(double boundaryDistance);

	/**
	 * Extract a Skeleton from the annotated volume.
	 */
	Skeleton parseVolumeSkeleton();

	/**
	 * Recursively discover the skeleton graph from the volume annotations.
	 */
	void traverse(const Position& pos, Skeleton& skeleton);

	/**
	 * The number of neighbors of a skeleton position in the volume.
	 */
	int numNeighbors(const Position& pos);

	// reference to the volume to process
	ExplicitVolume<unsigned char> _volume;

	bool _downSampleVolume;

	vigra::MultiArray<3, float> _boundaryDistance;

	// lemon graph compatible datastructures for Dijkstra
	Graph       _graph;
	PositionMap _positionMap;
	DistanceMap _distanceMap;

	double _boundaryWeight;

	lemon::Dijkstra<Graph, DistanceMap> _dijkstra;

	Graph::Node _root;
	Graph::Node _center;

	std::vector<Graph::Node> _boundary;

	float _maxBoundaryDistance2;

	double _minSegmentLength;
	double _minSegmentLengthRatio;

	bool   _skipExplainedNodes;
	double _explanationWeight;
};

#endif // IMAGEPROCESSING_TUBES_SKELETONIZE_H__

