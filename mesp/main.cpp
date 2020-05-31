#include <boost/chrono.hpp>
#include <vector>
#include "../common/input.hpp"
#include "../common/executor.hpp"
#include "../common/templates.hpp"
#include "mesp_multithread.hpp"

using boost::asio::thread_pool;
using boost::chrono::duration_cast;
using boost::chrono::milliseconds;
using boost::chrono::system_clock;
using std::make_shared;
using std::optional;
using std::string;


class app : public executor {
public:
	app(int argc, const char **argv): executor(argc, argv) {}

protected:
	void print_usage() const override {
		out->print(
			"Usage: " + cmd_name() + " [<options>...] [<graph-file> <disjoint-paths-file>]\n"
			"Finds the minimum eccentricity shortest path in a given graph.\n"
			"If no <graph-file> and <disjoint-paths-file> are provided, attempts to read from stdin.\n"
			"\n"
			"Options:\n"
			"  -j <jobs>, --parallel <jobs>\t\tUse <jobs> threads. Default value is 8.\n"
			"  -o <file>, --output <file>\t\tWrite the solution to <file> instead of stdout.\n"
			"\n"
			"Input graph format:\n" +
			graph_format_desc() + "\n"
			"Input disjoint paths format:\n" +
			disjoint_paths_format_desc() +
			"\n"
			"Output format:\n" +
			mesp_format_desc() + ""
		);
	}

	int impl() const override {
		if (args.size() < 2) {
			print_usage();
			return EXIT_SUCCESS;
		}

		optional<string> threads_count;
		optional<string> output_filename;
		optional<string> graph_filename;
		optional<string> dp_filename;

		for (size_t i = 1; i < args.size(); i++) {
			if (args[i] == "-j" || args[i] == "--parallel") {
				threads_count = args[++i];
			} else if (args[i] == "-o" || args[i] == "--output") {
				output_filename = args[++i];
			} else if (!graph_filename.has_value()) {
				graph_filename = args[i];
			} else if (!dp_filename.has_value()) {
				dp_filename = args[i];
			} else {
				throw unknown_argument_exception(args[i]);
			}
		}

		if (!graph_filename.has_value() xor !dp_filename.has_value()) {
			throw missing_arguments_exception();
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

		auto graph_input = in;
		if (graph_filename.has_value()) {
			graph_input = make_shared<reader>(open(*graph_filename, "r"));
		}

		auto dp_input = in;
		if (dp_filename.has_value()) {
			dp_input = make_shared<reader>(open(*dp_filename, "r"));
		}

		auto sol = out;
		if (output_filename.has_value()) {
			sol = make_shared<writer>(open(*output_filename, "w"));
		}


		auto G = read_graph(*graph_input);
		auto C = read_disjoint_paths(*dp_input);

		auto time0 = system_clock::now();
		thread_pool pool(threads);
		G->calculate_distances();

		auto solution = mesp_multithread(G, C, pool, [this, time0] (int k, double percent) {
			double duration_sec = (double) duration_cast<milliseconds>(system_clock::now() - time0).count() / 1000;
			out->print_tty("\rk = %d\t%6.2f %%\t%.2f s", k, percent, duration_sec);
		});

		out->print_tty("\n\nMESP found for k = %d.\n", solution.k);
		if (sol == out) out->print("\n");
		sol->print("%zu %d\n", solution.P.size(), solution.k);
		for (int u : solution.P) sol->print("%d ", u);
		sol->print("\n");

		pool.join();
		return EXIT_SUCCESS;
	}
};


int main(int argc, const char **argv)
{
	app app(argc, argv);
	return app.run();
}
