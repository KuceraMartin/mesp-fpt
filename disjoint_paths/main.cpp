#include <boost/chrono.hpp>
#include "../common/executor.hpp"
#include "../common/templates.hpp"
#include "disjoint_paths.hpp"

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
			"Usage: " + cmd_name() + "\n"
			"Finds the smallest modulator to disjoint paths for the graph provided in stdin.\n"
			"\n"
			"Options:\n"
			"  -o <file>, --output <file>\t\tWrite the solution to <file> instead of stdout.\n"
			"Input graph format:\n" +
			graph_format_desc() +
			"\n"
			"Output set format:\n" +
			disjoint_paths_format_desc()
		);
	}

	int impl() const override {
		optional<string> output_filename;

		for (size_t i = 1; i < args.size(); i++) {
			if (args[i] == "-o" || args[i] == "--output") {
				output_filename = args[++i];
			} else {
				throw unknown_argument_exception(args[i]);
			}
		}

		auto sol = out;
		if (output_filename.has_value()) {
			sol = make_shared<writer>(open(*output_filename, "w"));
		}

		auto G = read_graph(*in);

		auto time0 = system_clock::now();

		auto res = modulator_to_disjoint_paths(G, [this, time0] (int c) {
			double duration_sec = (double) duration_cast<milliseconds>(system_clock::now() - time0).count() / 1000;
			out->print_tty("\rc = %d\t %.2f s", c, duration_sec);
		});

		out->print_tty("\n\nDistance to disjoint paths is %zu.\n", res->size());
		if (sol == out) out->print("\n");
		sol->print("%zu\n", res->size());
		for (int u : *res) sol->print("%d ", u);
		sol->print("\n");

		return EXIT_SUCCESS;
	}
};


int main(int argc, const char **argv) {
	app app(argc, argv);
	return app.run();
}
