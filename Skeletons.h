#ifndef IMAGEPROCESSING_SKELETONS_H__
#define IMAGEPROCESSING_SKELETONS_H__

#include "Volume.h"
#include "Skeleton.h"

class Skeletons : public Volume {

public:

	void add(uint64_t id, std::shared_ptr<Skeleton> skeleton, int color = -1) {

		_skeletons[id] = skeleton;
		_colors[id] = (color < 0 ? id : color);
		_ids.push_back(id);

		setBoundingBoxDirty();
	}

	void remove(uint64_t id) {

		if (!contains(id))
			return;

		_skeletons.erase(id);
		_colors.erase(id);
		_ids.erase(std::find(_ids.begin(), _ids.end(), id));

		setBoundingBoxDirty();
	}

	std::shared_ptr<Skeleton> get(uint64_t id) {

		if (_skeletons.count(id))
			return _skeletons[id];

		return std::shared_ptr<Skeleton>();
	}

	int getColor(uint64_t id) {

		return _colors[id];
	}

	const std::vector<uint64_t>& getSkeletonIds() const {

		return _ids;
	}

	void clear() { _skeletons.clear(); _colors.clear(); _ids.clear(); setBoundingBoxDirty(); }

	bool contains(uint64_t id) const { return _skeletons.count(id); }

	std::size_t size() const { return _skeletons.size(); }

protected:

	util::box<float,3> computeBoundingBox() const override {

		util::box<float,3> bb;

		for (auto& p : _skeletons)
			bb += p.second->getBoundingBox();

		return bb;
	}

	std::map<uint64_t, std::shared_ptr<Skeleton> > _skeletons;
	std::map<uint64_t, int >                       _colors;

	std::vector<uint64_t> _ids;
};

#endif // IMAGEPROCESSING_SKELETONS_H__

