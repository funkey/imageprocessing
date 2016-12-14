#ifndef IMAGEPROCESSING_SKELETON_EDGE_MATCH_SCORES_H__
#define IMAGEPROCESSING_SKELETON_EDGE_MATCH_SCORES_H__

class SkeletonEdgeMatchScores {

public:

	SkeletonEdgeMatchScores(std::string name) : _name(name) {}

	void setSource(std::shared_ptr<Skeleton> source) { _source = source; }
	void setTarget(std::shared_ptr<Skeleton> target) { _target = target; }

	std::shared_ptr<Skeleton> getSource() const { return _source; }
	std::shared_ptr<Skeleton> getTarget() const { return _target; }

	void setScore(unsigned int e, unsigned int f, double score) {

		_scores[std::make_pair(e, f)] = score;
	}

	double getScore(unsigned int e, unsigned int f) const {

		if (!_scores.count(std::make_pair(e, f)))
			return 0;

		return _scores.at(std::make_pair(e,f));
	}

	double getMaxScore() const {

		double max = 0;
		for (auto p : _scores)
			max = std::max(max, p.second);

		return max;
	}

	std::string getName() const { return _name; }

private:

	std::shared_ptr<Skeleton> _source;
	std::shared_ptr<Skeleton> _target;

	std::map<std::pair<unsigned int, unsigned int>, double> _scores;

	std::string _name;
};

#endif // IMAGEPROCESSING_SKELETON_EDGE_MATCH_SCORES_H__

