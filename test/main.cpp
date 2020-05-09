#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <cstdio>
#include "../common/executor.hpp"
#include "../disjoint_paths/disjoint_paths.hpp"
#include "../mesp/mesp_multithread.hpp"

using boost::asio::thread_pool;
using boost::chrono::duration_cast;
using boost::chrono::milliseconds;
using boost::chrono::system_clock;
using boost::filesystem::directory_options;
using boost::filesystem::file_type::regular_file;
using boost::filesystem::filesystem_error;
using boost::filesystem::recursive_directory_iterator;
using std::optional;
using std::string;
using std::to_string;
using std::vector;


class app : public executor {
public:
	app(int argc, const char **argv): executor(argc, argv) {}

protected:
	void print_usage() const override {
		out->print(
			"Usage: " + cmd_name() + " [<options>...] <dir>...\n"
			"Tests the mesp program against sample inputs.\n"
			"\n"
			"Looks for test case files in the provided directories.\n"
			"Files with extension `.in` are expected to describe an input graph.\n"
			"For each `<name>.in` the resulting eccentricity is checked against `<name>.ecc`.\n"
			"\n"
			 "Options:\n"
			 "  -j <jobs>, --parallel <jobs>\t\tUse <jobs> threads. Default value is 8.\n"
		);
	}

	int impl() const override {
		if (args.size() < 2) {
			print_usage();
			return EXIT_SUCCESS;
		}

		optional<string> threads_count;
		vector<string> paths;

		for (size_t i = 1; i < args.size(); i++) {
			if (args[i] == "-j" || args[i] == "--parallel") {
				threads_count = args[++i];
			} else {
				paths.push_back(args[i]);
			}
		}

		int threads = 8;
		if (threads_count.has_value()) {
			try {
				threads = std::stoi(*threads_count);
			} catch (std::exception &e) {
				throw invalid_argument_exception("threads", *threads_count, "Must be a positive integer.");
			}
			if (threads <= 0) {
				throw invalid_argument_exception("threads", *threads_count, "Must be a positive integer.");
			}
		}

		vector<recursive_directory_iterator> dirs;
		dirs.reserve(paths.size());
		for (auto &path : paths) {
			try {
				dirs.emplace_back(path, directory_options::follow_directory_symlink);
			} catch (filesystem_error &e) {
				throw open_file_exception(path, e.what());
			}
		}

		thread_pool pool(threads);
		vector<string> errors;
		auto time0 = system_clock::now();
		int cnt = 0;
		int failures = 0;

		for (auto &it : dirs) {
			for (auto &entry : it) {
				if (entry.status().type() != regular_file) continue;
				if (entry.path().extension() != ".in") continue;
				cnt++;

				auto G = read_graph(open(entry.path(), "r"));
				G->calculate_distances();
				auto C = modulator_to_disjoint_paths(G);
				auto mesp = mesp_multithread(G, C, pool);
				int k = G->ecc(mesp.P);

				bool success = true;

				if (mesp.k != k) {
					success = false;
					errors.push_back(
						entry.path().string() + "\t\t(reported eccentricity) " + to_string(mesp.k) + " != " +
						to_string(k) + " (actual eccentricity)");
				}

				if (mesp.P.size() - 1 != G->distance(mesp.P[0], mesp.P.back())) {
					success = false;
					errors.push_back(entry.path().string() + "\t\tnot a shortest path");
				}

				auto ecc_file = entry.path().parent_path() / entry.path().stem() += ".ecc";
				if (exists(ecc_file) && is_regular_file(ecc_file)) {
					int expected;
					reader r(open(ecc_file, "r"));
					r.scan("%d", &expected);
					if (mesp.k != expected) {
						success = false;
						errors.push_back(
							entry.path().string() + "\t\t(reported eccentricity) " + to_string(mesp.k) + " != " +
							to_string(expected) + " (expected eccentricity)");
					}
				}

				if (success) {
					out->print(".");
				} else {
					out->print("F");
					failures++;
				}
			}
		}

		pool.join();
		double duration_sec = (double) duration_cast<milliseconds>(system_clock::now() - time0).count() / 1000;

		out->print("\n\n");
		if (!errors.empty()) {
			for (string &e : errors) {
				out->print("%s\n", e.c_str());
			}
			out->print("\nFAILURES! (%d tests, %zu failures, %.2f seconds)\n", cnt, failures, duration_sec);
			return EXIT_FAILURE;
		}

		out->print("OK (%zu tests, %.2f seconds)\n", cnt, duration_sec);
		return EXIT_SUCCESS;
	}
};


int main(int argc, const char **argv) {
	app app(argc, argv);
	return app.run();
}
