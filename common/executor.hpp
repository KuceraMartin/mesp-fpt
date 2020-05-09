#ifndef IMPL_EXECUTOR_HPP
#define IMPL_EXECUTOR_HPP

#include <boost/filesystem.hpp>
#include <vector>
#include "input.hpp"


class executor {
protected:
	std::vector<std::string> args;
	std::shared_ptr<reader> in;
	std::shared_ptr<writer> out;
	std::shared_ptr<writer> err;

public:
	executor(int argc, const char **argv):
		in(std::make_shared<reader>(std::make_shared<file>(stdin))),
		out(std::make_shared<writer>(std::make_shared<file>(stdout))),
		err(std::make_shared<writer>(std::make_shared<file>(stderr)))
	{
		args.reserve(argc);
		for (int i = 0; i < argc; i++) {
			args.push_back(argv[i]);
		}
	}


	std::string cmd_name() const
	{
		return boost::filesystem::path(args[0]).filename().string();
	}


	virtual int run() const
	{
		if (args.size() > 1 && args[1] == "--help") {
			print_usage();
			return EXIT_SUCCESS;
		}
		try {
			return impl();
		} catch (invalid_input_exception &e) {
			err->print("%s\nTry `%s --help` for more information.\n", e.message().c_str(), cmd_name().c_str());
		} catch (presentable_exception &e) {
			err->print("%s\n", e.message().c_str());
		} catch (std::exception &e) {
			err->print("Unexpected error occurred: %s\n", e.what());
		} catch (...) {
			err->print("Unexpected error occurred.\n");
		}
		return EXIT_FAILURE;
	}


protected:
	virtual void print_usage() const = 0;
	virtual int impl() const = 0;
};

#endif //IMPL_EXECUTOR_HPP
