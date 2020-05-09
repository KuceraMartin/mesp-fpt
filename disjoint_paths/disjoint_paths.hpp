#ifndef IMPL_INNER_SOLVER_HPP
#define IMPL_INNER_SOLVER_HPP

#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>
#include "../common/graph.hpp"


std::shared_ptr<std::unordered_set<int>> modulator_to_disjoint_paths(std::shared_ptr<const graph> G, const std::function<void(int)> &report_progress = [](int){})
{
	struct inner_solver {
		std::shared_ptr<const graph> G;
		std::vector<int> deg;
		std::shared_ptr<std::unordered_set<int>> res;

		explicit inner_solver(std::shared_ptr<const graph> &G):
			G(G),
			res(std::make_shared<std::unordered_set<int>>())
		{
			deg.resize(G->n);
			for (int i = 0; i < G->n; i++) {
				deg[i] = G->neighbors(i).size();
			}
		}

		bool solve(int c) {
			if (c < 0) return false;
			int b = -1;
			for (int u = 0; u < G->n; u++) {
				if (res->count(u)) continue;
				if (deg[u] <= 2) continue;
				if (c == 0) return false;
				if (b == -1 || deg[u] > deg[b]) b = u;
			}

			if (b == -1) {
				std::unordered_set<int> to_res;
				std::vector<int> visited(G->n, 0);
				for (int u = 0; u < G->n; u++) {
					if (res->count(u)) continue;
					if (visited[u]) continue;
					visited[u] = 1;
					bool is_cycle = false;
					int p = u;
					for (int v : G->neighbors(u)) {
						if (res->count(v)) continue;
						while (true) {
							if (visited[v]) {
								is_cycle = true;
								break;
							}
							visited[v] = 1;
							if (deg[v] == 1) break;
							for (int n : G->neighbors(v)) {
								if (n == p || res->count(n)) continue;
								p = v;
								v = n;
								break;
							}
						}
					}
					if (is_cycle) {
						to_res.insert(u);
					}
				}
				if (to_res.size() > c) return false;
				res->insert(to_res.begin(), to_res.end());
				return true;
			}

			std::unordered_set<int> to_res;
			for (auto n : G->neighbors(b)) {
				if (res->count(n)) continue;
				to_res.insert(n);
			}
			for (int keep_1 = 0; keep_1 < G->neighbors(b).size(); keep_1++) {
				if (res->count(G->neighbors(b)[keep_1])) continue;
				to_res.erase(G->neighbors(b)[keep_1]);
				if (deg[b] == 3) {
					res_insert(to_res);
					if (solve(c - (int) to_res.size())) return true;
					res_remove(to_res);
				} else {
					for (int keep_2 = keep_1 + 1; keep_2 < G->neighbors(b).size(); keep_2++) {
						if (res->count(G->neighbors(b)[keep_2])) continue;
						to_res.erase(G->neighbors(b)[keep_2]);
						res_insert(to_res);
						if (solve(c - (int) to_res.size())) return true;
						res_remove(to_res);
						to_res.insert(G->neighbors(b)[keep_2]);
					}
				}
				to_res.insert(G->neighbors(b)[keep_1]);
			}

			res_insert(b);
			if (solve(c - 1)) return true;
			res_remove(b);

			return false;
		}


		void res_insert(int u)
		{
			res->insert(u);
			for (int v : G->neighbors(u)) deg[v]--;
		}


		void res_insert(const std::unordered_set<int> &vertices)
		{
			for (int u : vertices) res_insert(u);
		}


		void res_remove(int u)
		{
			res->erase(u);
			for (int v : G->neighbors(u)) deg[v]++;
		}


		void res_remove(const std::unordered_set<int> &vertices)
		{
			for (int u : vertices) res_remove(u);
		}

	};


	inner_solver solver(G);
	for (int c = 0; c < G->n; c++) {
		report_progress(c);
		if (solver.solve(c)) return solver.res;
	}
	throw implementation_exception(); // should not reach here
}


#endif //IMPL_INNER_SOLVER_HPP
