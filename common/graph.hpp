#ifndef IMPL_GRAPH_HPP
#define IMPL_GRAPH_HPP

#include <algorithm>
#include <queue>
#include <vector>
#include "common.hpp"
#include "graph.hpp"


class graph {
public:
	const int n;

private:
	std::vector<std::vector<int>> neighborhood;
	std::vector<std::vector<int>> distances;


public:
	explicit graph(int n):
		n(n)
	{
		neighborhood.resize(n);
	}


	void add_edge(int u, int v)
	{
		neighborhood[u].push_back(v);
		neighborhood[v].push_back(u);
	}


	void calculate_distances()
	{
		distances.resize(n, std::vector<int>(n, -1));
		for (int i = 0; i < n; i++) {
			distances[i][i] = 0;
			std::queue<int> q;
			q.push(i);
			while (!q.empty()) {
				int u = q.front();
				q.pop();
				for (int v : neighbors(u)) {
					if (distances[i][v] != -1) continue;
					distances[i][v] = distances[i][u] + 1;
					q.push(v);
				}
			}
		}
	}


	const std::vector<int> & neighbors(int u) const
	{
		return neighborhood[u];
	}


	int distance(int u, int v) const
	{
		return distances[u][v];
	}


	template<typename Container>
	int distance(int u, const Container &S) const
	{
		static_assert(std::is_same<typename Container::value_type, int>::value);
		int res = INF;
		for (int v : S) {
			res = std::min(res, distance(u, v));
		}
		return res;
	}


	int ecc(const std::vector<int> &S) const {
		std::queue<int> q;
		std::vector<int> dst(n, -1);
		for (int u : S) {
			q.push(u);
			dst[u] = 0;
		}
		int res = 0;
		while (!q.empty()) {
			int u = q.front();
			q.pop();
			res = std::max(res, dst[u]);
			for (int v : neighbors(u)) {
				if (dst[v] != -1) continue;
				dst[v] = dst[u] + 1;
				q.push(v);
			}
		}
		return res;
	}

};


#endif //IMPL_GRAPH_HPP
