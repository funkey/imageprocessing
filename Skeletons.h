#ifndef IMAGEPROCESSING_SKELETONS_H__
#define IMAGEPROCESSING_SKELETONS_H__

#include "Volume.h"
#include "Skeleton.h"

class Skeletons : public Volume {

public:

	void add(unsigned int id, std::shared_ptr<Skeleton> skeleton, int color = -1) {

		_skeletons[id] = skeleton;
		_colors[id] = (color < 0 ? id : color);
		_ids.push_back(id);

		setBoundingBoxDirty();
	}

	void remove(unsigned int id) {

		_skeletons.erase(id);
		_colors.erase(id);
		_ids.erase(std::find(_ids.begin(), _ids.end(), id));

		setBoundingBoxDirty();
	}

	std::shared_ptr<Skeleton> get(unsigned int id) {

		if (_skeletons.count(id))
			return _skeletons[id];

		return std::shared_ptr<Skeleton>();
	}

	int getColor(unsigned int id) {

		return _colors[id];
	}

	const std::vector<unsigned int>& getSkeletonIds() const {

		return _ids;
	}

	void clear() { _skeletons.clear(); _colors.clear(); _ids.clear(); setBoundingBoxDirty(); }

	bool contains(unsigned int id) const { return _skeletons.count(id); }

	std::size_t size() const { return _skeletons.size(); }

protected:

	util::box<float,3> computeBoundingBox() const override {

		util::box<float,3> bb;

		for (auto& p : _skeletons)
			bb += p.second->getBoundingBox();

		return bb;
	}

	std::map<unsigned int, std::shared_ptr<Skeleton> > _skeletons;
	std::map<unsigned int, int >                       _colors;

	std::vector<unsigned int> _ids;
};

#endif // IMAGEPROCESSING_SKELETONS_H__

