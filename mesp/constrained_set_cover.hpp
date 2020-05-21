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
	D[0] = {{boost::dynamic_bitset<>(requirements.size()), {-1, boost::dynamic_bitset<>()}}};
	for (int i = 0; i < candidates.size(); i++) {
		for (auto [r, _] : D[i]) {
			if (res_candidate_id[i] != -1) {
				D[i + 1][r | psi(candidates[i][res_candidate_id[i]])] = {res_candidate_id[i], r};
				continue;
			}
			for (int j = 0; j < candidates[i].size(); j++) {
				D[i + 1][r | psi(candidates[i][j])] = {j, r};
			}
		}
	}
	boost::dynamic_bitset<> R;
	R.resize(requirements.size(), 1);
	for (int i = candidates.size(); i > 0; i--) {
		if (!D[i].count(R)) return false;
		res_candidate_id[i - 1] = D[i][R].candidate_id;
		R = D[i][R].prev;
	}
	return true;
}


#endif //IMPL_CONSTRAINED_SET_COVER_HPP
