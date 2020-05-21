#ifndef IMPL_MESP_INNER_HPP
#define IMPL_MESP_INNER_HPP

#include <boost/dynamic_bitset.hpp>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../common/common.hpp"
#include "../common/graph.hpp"
#include "constrained_set_cover.hpp"


class mesp_inner {
public:
	int k;
	int pi_first, pi_last;
	std::shared_ptr<const graph> G;
	std::shared_ptr<const std::unordered_set<int>> C;
	path solution;

private:
	std::unordered_set<int> L;
	std::vector<int> pi;
	std::unordered_map<int, int> e;

public:
	mesp_inner(std::shared_ptr<const graph> G, std::shared_ptr<const std::unordered_set<int>> &C, int k, int pi_first = -1, int pi_last = -1):
		G(G),
		C(C),
		k(k),
		pi_first(pi_first),
		pi_last(pi_last)
	{}


	bool solve()
	{
		init_L();
		do {
			init_pi();
			do {
				if (!can_pi()) continue;
				init_e();
				do {
					if (solve_inner()) return true;
				} while (next_e());
			} while (next_pi());
		} while (next_L());
		return false;
	}


private:
	bool solve_inner()
	{
		auto candidate_segments = get_segments();
		if (!candidate_segments.has_value()) return false;
		std::unordered_set<int> I(L.begin(), L.end());
		std::vector<int> h_inv((*candidate_segments).size(), -1);
		std::vector<std::vector<path>> candidates;
		for (int i = 0; i < candidate_segments->size(); i++) {
			if ((*candidate_segments)[i].size() == 1) {
				for (int u : (*candidate_segments)[i][0]) I.insert(u);
				continue;
			}
			candidates.emplace_back(std::move((*candidate_segments)[i]));
			h_inv[i] = candidates.size() - 1;
		}

		std::unordered_set<int> U;
		for (int v = 0; v < G->n; v++) {
			if (C->count(v) || I.count(v)) continue;
			if (estimate_path_dst(v) > k + 1) return false;
			if (estimate_path_dst(v) == k + 1) U.insert(v);
		}
		if (U.size() > 2 * (pi.size() - 1)) return false;

		std::vector<int> requirements;
		for (int u = 0; u < G->n; u++) {
			if (L.count(u)) continue;
			if (!C->count(u) && !U.count(u)) continue;
			int need_dst = e.count(u) ? e[u] : k;
			if (G->distance(u, I) <= need_dst) continue;
			requirements.push_back(u);
		}
		const std::function<boost::dynamic_bitset<>(const path &)> psi = [this, &requirements] (const path &segment) {
			boost::dynamic_bitset<> res(requirements.size(), 0);
			if (segment.empty()) return res;
			for (int i = 0; i < requirements.size(); i++) {
				int u = requirements[i];
				int need_dst = e.count(u) ? e[u] : k;
				if (G->distance(u, segment) <= need_dst) {
					res[i] = true;
				}
			}
			return res;
		};
		auto true_segment_id = constrained_set_cover(requirements, candidates, psi);
		if (!true_segment_id.has_value()) return false;

		solution.clear();
		for (int i = 0; i < pi.size() - 1; i++) {
			solution.push_back(pi[i]);
			path &segment = h_inv[i] == -1 ? (*candidate_segments)[i][0] : candidates[h_inv[i]][(*true_segment_id)[h_inv[i]]];
			for (int s : segment) solution.push_back(s);
		}
		solution.push_back(pi.back());
		return G->ecc(solution) <= k;
	}


	std::optional<std::vector<std::vector<path>>> get_segments() const
	{
		std::vector<std::vector<path>> candidate_segments(pi.size() - 1);
		for (int i = 0; i < pi.size() - 1; i++) {
			std::queue<int> q;
			q.push(pi[i + 1]);
			std::unordered_map<int, int> dst = {{pi[i + 1], 0}};
			while (!q.empty()) {
				int u = q.front();
				q.pop();
				if (u == pi[i]) break;
				for (int v : G->neighbors(u)) {
					if (dst.count(v)) continue;
					if (C->count(v) && v != pi[i]) continue;
					dst[v] = dst[u] + 1;
					q.push(v);
				}
			}
			if (!dst.count(pi[i])) return std::nullopt;
			if (dst[pi[i]] != G->distance(pi[i + 1], pi[i])) return std::nullopt;
			std::vector<path> Sigma;
			std::vector<int> K;
			for (int u : G->neighbors(pi[i])) {
				if (!dst.count(u) || dst[u] != dst[pi[i]] - 1) continue;
				if (u == pi[i + 1]) {
					Sigma.emplace_back();
					break;
				}
				for (int v : G->neighbors(u)) {
					if (!dst.count(v) || dst[v] != dst[u] - 1) continue;
					Sigma.push_back({u});
					int K_added = 0;
					while (v != pi[i + 1]) {
						int p = Sigma.back().back();
						if (estimate_path_dst(v) > k) {
							if (K_added++ < 2) K.push_back(v);
							if (K.size() > 4) return std::nullopt;
						}
						Sigma.back().push_back(v);
						for (int n : G->neighbors(v)) {
							if (!dst.count(n) || dst[n] != dst[v] - 1) continue;
							if (n == p) continue;
							v = n;
							break;
						}
					}
				}
			}
			for (auto &segment : Sigma) {
				std::vector<int> K_sat(K.size(), 0);
				for (int j = 0; j < K.size(); j++) {
					K_sat[j] |= G->distance(K[j], segment) <= k;
				}
				bool is_sat = true;
				for (bool s : K_sat) is_sat |= s;
				if (is_sat) {
					candidate_segments[i].emplace_back(std::move(segment));
				}
			}
		}
		return candidate_segments;
	}


	void init_L()
	{
		L.clear();
		auto C_iter = C->begin();
		if (pi_first == -1) {
			L.insert(*C_iter);
			++C_iter;
		} else {
			L.insert(pi_first);
		}
		if (pi_last == -1) {
			L.insert(*C_iter);
		} else {
			L.insert(pi_last);
		}
	}


	bool next_L()
	{
		int max_size = C->size();
		if (pi_first != -1) max_size++;
		if (pi_last != -1) max_size++;
		if (L.size() == max_size) return false;
		do {
			for (int v : *C) {
				if (L.count(v)) {
					L.erase(v);
				} else {
					L.insert(v);
					break;
				}
			}
		} while (L.size() < 2);
		return true;
	}


	void init_pi()
	{
		pi.clear();
		pi.reserve(L.size());
		if (pi_first != -1) pi.push_back(pi_first);
		for (int u : L) {
			if (u == pi_first || u == pi_last) continue;
			pi.push_back(u);
		}
		if (pi_last != -1) pi.push_back(pi_last);
		std::sort(pi.begin() + (pi_first == -1 ? 0 : 1), pi.end() - (pi_last == -1 ? 0 : 1));
	}


	bool can_pi() const
	{
		int length = 0;
		for (int i = 0; i < pi.size() - 1; i++) {
			length += G->distance(pi[i], pi[i + 1]);
		}
		return length == G->distance(pi[0], pi.back());
	}


	bool next_pi()
	{
		int first = pi_first == -1 ? 0 : 1;
		int last = (int) pi.size() - (pi_last == -1 ? 1 : 2);
		if (first >= last) return false;
		int m = last - 1;
		while (m >= first && pi[m] > pi[m + 1]) m--;
		if (m < first) return false;
		int k = last;
		while (pi[m] > pi[k]) k--;
		std::swap(pi[m], pi[k]);
		int p = m + 1;
		int q = last;
		while (p < q) {
			std::swap(pi[p], pi[q]);
			p++;
			q--;
		}
		return true;
	}


	void init_e()
	{
		e.clear();
		for (int v : *C) {
			if (L.count(v)) continue;
			e[v] = 1;
		}
	}


	bool next_e()
	{
		int cnt = 0;
		for (int v : *C) {
			if (L.count(v)) continue;
			if (e[v] < k) {
				e[v]++;
				break;
			} else {
				e[v] = 1;
				cnt++;
			}
		}
		return cnt < e.size();
	}


	int estimate_path_dst(int u) const
	{
		int res = INF;
		if (pi_first != -1) {
			res = std::min(res, G->distance(u, pi_first));
		}
		if (pi_last != -1) {
			res = std::min(res, G->distance(u, pi_last));
		}
		for (int v : *C) {
			int e_v = e.count(v) ? e.at(v) : 0;
			res = std::min(res, G->distance(u, v) + e_v);
		}
		return res;
	}

};


#endif //IMPL_MESP_INNER_HPP
