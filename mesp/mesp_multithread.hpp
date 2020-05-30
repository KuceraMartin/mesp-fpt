#ifndef IMPL_MESP_H
#define IMPL_MESP_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <unordered_set>
#include <vector>
#include "mesp_inner.hpp"


std::optional<path> check_path(const graph &G)
{
	int q = -1;
	for (int u = 0; u < G.n; u++) {
		if (G.neighbors(u).size() > 2) return std::nullopt;
		if (G.neighbors(u).size() == 1) q = u;
	}
	if (q == -1) return std::nullopt;
	path res;
	res.reserve(G.n);
	std::vector<int> visited(G.n, 0);
	int p = q;
	visited[p] = 1;
	do {
		res.push_back(q);
		for (int v : G.neighbors(q)) {
			if (v == p) continue;
			p = q;
			q = v;
			break;
		}
		visited[q] = 1;
	} while (G.neighbors(q).size() > 1);
	res.push_back(q);
	for (int v : visited) if (!v) return std::nullopt;
	return res;
}


struct mesp_solution {
	int k;
	path P;
};


mesp_solution mesp_multithread(
	const std::shared_ptr<const graph> &G,
	const std::shared_ptr<const std::unordered_set<int>> &C,
	boost::asio::thread_pool &pool,
	const std::function<void(int, double)> &report_progress = [](int, double) {}
) {
	class threads_status {
	private:
		boost::mutex mtx;
		int cnt_finished = 0;
		std::optional<path> solution;

	public:
		void report_solution(path &&s) {
			boost::mutex::scoped_lock lock(mtx);
			cnt_finished++;
			if (solution.has_value()) return;
			solution = std::move(s);
		}

		void report_no_solution() {
			boost::mutex::scoped_lock lock(mtx);
			cnt_finished++;
		}

		bool is_solved() const {
			return solution.has_value();
		}

		int attempts() const {
			return cnt_finished;
		}

		std::vector<int> && get_solution() {
			return move(*solution);
		}
	};


	class consumer {
	private:
		std::shared_ptr<threads_status> current_status;
		mesp_inner inner;

	public:
		consumer(const std::shared_ptr<threads_status> &current_status, mesp_inner &&inner) :
				current_status(current_status),
				inner(std::move(inner)) {}

		void operator()() {
			if (current_status->is_solved()) return;
			if (inner.solve()) {
				current_status->report_solution(move(inner.solution));
			} else {
				current_status->report_no_solution();
			}
		}
	};

	auto path = check_path(*G);
	if (path.has_value()) {
		return {0, *path};
	}

	for (int k = 1; k <= G->n; k++) {
		auto status = std::make_shared<threads_status>();
		int attempts = 0;
		if (C->size() >= 2) {
			post(pool, consumer(status, mesp_inner(G, C, k)));
			attempts++;
		}
		for (int pi_first = 0; pi_first < G->n; pi_first++) {
			if (C->count(pi_first)) continue;
			if (C->size() >= 2) {
				post(pool, consumer(status, mesp_inner(G, C, k, pi_first)));
				attempts++;
			}
			for (int pi_last = pi_first + 1; pi_last < G->n; pi_last++) {
				if (C->count(pi_last)) continue;
				post(pool, consumer(status, mesp_inner(G, C, k, pi_first, pi_last)));
				attempts++;
			}
		}
		while (status->attempts() < attempts && !status->is_solved()) {
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			report_progress(k, 100.0 * status->attempts() / attempts);
		}
		if (status->is_solved()) return {k, std::move(status->get_solution())};
	}
	throw implementation_exception(); // should not reach here
}

#endif //IMPL_MESP_H
