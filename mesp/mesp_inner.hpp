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
		std::vector<std::vector<path>> candidate_segments;
		std::vector<int> true_segment_id;
		if (!check_segments(candidate_segments, true_segment_id)) return false;

		std::unordered_set<int> I;
		for (int u : L) {
			I.insert(u);
		}
		for (int i = 0; i < true_segment_id.size(); i++) {
			if (true_segment_id[i] == -1) continue;
			for (int u : candidate_segments[i][true_segment_id[i]]) {
				I.insert(u);
			}
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
		if (!constrained_set_cover(requirements, candidate_segments, psi, true_segment_id)) return false;

		solution.clear();
		for (int i = 0; i < pi.size() - 1; i++) {
			solution.push_back(pi[i]);
			for (int s : candidate_segments[i][true_segment_id[i]]) {
				solution.push_back(s);
			}
		}
		solution.push_back(pi.back());
		return true;
	}


	bool check_segments(std::vector<std::vector<path>> &candidate_segments, std::vector<int> &true_segment_id) const
	{
		candidate_segments.resize(pi.size() - 1);
		true_segment_id.resize(pi.size() - 1, -1);
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
			if (!dst.count(pi[i])) return false;
			if (dst[pi[i]] != G->distance(pi[i + 1], pi[i])) return false;
			int segment_id = -1;
			for (int u : G->neighbors(pi[i])) {
				if (!dst.count(u) || dst[u] != dst[pi[i]] - 1) continue;
				if (u == pi[i + 1]) {
					candidate_segments[i].emplace_back();
					break;
				}
				for (int v : G->neighbors(u)) {
					if (!dst.count(v) || dst[v] != dst[u] - 1) continue;
					candidate_segments[i].push_back({u});
					segment_id++;
					while (v != pi[i + 1]) {
						int p = candidate_segments[i].back().back();
						candidate_segments[i].back().push_back(v);
						if (estimate_path_dst(v) > k) {
							if (true_segment_id[i] == -1) {
								true_segment_id[i] = segment_id;
							} else if (true_segment_id[i] != segment_id) {
								return false;
							}
						}
						for (int n : G->neighbors(v)) {
							if (!dst.count(n) || dst[n] != dst[v] - 1) continue;
							if (n == p) continue;
							v = n;
							break;
						}
					}
				}
			}
			if (candidate_segments[i].size() == 1) {
				true_segment_id[i] = 0;
			}
		}
		return true;
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
