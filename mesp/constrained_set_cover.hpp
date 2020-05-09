#ifndef IMPL_CONSTRAINED_SET_COVER_HPP
#define IMPL_CONSTRAINED_SET_COVER_HPP

#include <boost/dynamic_bitset.hpp>
#include <functional>
#include <unordered_map>
#include <vector>


template <typename Requirement, typename Candidate>
bool constrained_set_cover(
		const std::vector<Requirement> &requirements,
		const std::vector<std::vector<Candidate>> &candidates,
		const std::function<boost::dynamic_bitset<>(const Candidate &)> &psi,
		std::vector<int> &res_candidate_id
) {
	struct satisfied_by {
		int candidate_id;
		boost::dynamic_bitset<> prev;
	};
	std::vector<std::unordered_map<boost::dynamic_bitset<>, satisfied_by>> D(candidates.size() + 1);
	boost::dynamic_bitset<> satisfied(requirements.size(), 0);
	for (int i = 0; i < candidates.size(); i++) {
		if (res_candidate_id[i] == -1) continue;
		satisfied |= psi(candidates[i][res_candidate_id[i]]);
	}
	D[0] = {{satisfied, {-1, boost::dynamic_bitset<>()}}};
	int prev = 0;
	for (int i = 0; i < candidates.size(); i++) {
		if (res_candidate_id[i] != -1) continue;
		for (int j = 0; j < candidates[i].size(); j++) {
			auto &candidate = candidates[i][j];
			for (auto [r, _] : D[prev]) {
				D[i + 1][r | psi(candidate)] = {j, r};
			}
		}
		prev = i + 1;
	}
	boost::dynamic_bitset<> R;
	R.resize(requirements.size(), 1);
	for (prev = candidates.size(); prev >= 1 && res_candidate_id[prev - 1] != -1; prev--);
	for (int i = prev; i > 0; i--) {
		if (res_candidate_id[i - 1] != -1) continue;
		if (!D[i].count(R)) return false;
		res_candidate_id[i - 1] = D[i][R].candidate_id;
		R = D[i][R].prev;
	}
	return D[0].count(R);
}


#endif //IMPL_CONSTRAINED_SET_COVER_HPP
