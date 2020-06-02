#ifndef IMPL_INPUT_HPP
#define IMPL_INPUT_HPP

#include <cstring>
#include <boost/filesystem.hpp>
#include <memory>
#include <unistd.h>
#include <unordered_set>
#include <vector>
#include "common.hpp"
#include "graph.hpp"


class file {
public:
	FILE *stream;

private:
	bool do_close = false;

public:
	file(const std::string &name, const char *mode):
		stream(fopen(name.c_str(), mode))
	{
		do_close = stream != NULL;
	}


	file(FILE *f):
		stream(f)
	{}


	~file()
	{
		if (do_close) {
			fclose(stream);
		}
	}
};


std::shared_ptr<const file> open(const std::string &name, const char *mode)
{
	auto f = std::make_shared<file>(name, mode);
	if (f->stream == nullptr) {
		throw open_file_exception(name, strerror(errno));
	}
	return f;
}


std::shared_ptr<const file> open(const boost::filesystem::path &path, const char *mode)
{
	return open(path.string(), mode);
}


class writer {
private:
	std::shared_ptr<const file> f;

public:
	writer(std::shared_ptr<const file> f):
		f(f)
	{}


	template<typename...Ts>
	int print(const std::string &format, Ts&&...params) const
	{
		int res = fprintf(f->stream, format.c_str(), std::forward<Ts>(params)...);
		fflush(f->stream);
		return res;
	}


	template<typename...Ts>
	int print_tty(const std::string &format, Ts&&...params) const
	{
		if (isatty(fileno(f->stream))) {
			return print(format, std::forward<Ts>(params)...);
		}
		return 0;
	}

};


class reader {
private:
	std::shared_ptr<const file> f;

public:
	reader(std::shared_ptr<const file> f):
		f(f)
	{}


	template<typename...Ts>
	int scan(const char *format, Ts&&...params) const
	{
		return fscanf(f->stream, format, std::forward<Ts>(params)...);
	}
};


std::shared_ptr<graph> read_graph(const reader &r)
{
	int n, m;
	if (r.scan("%d %d", &n, &m) != 2) throw graph_input_exception();
	auto G = std::make_shared<graph>(n);
	for (int i = 0; i < m; i++) {
		int u, v;
		if (r.scan("%d %d", &u, &v) != 2) throw graph_input_exception();
		G->add_edge(u, v);
	}
	return G;
}


std::shared_ptr<std::unordered_set<int>> read_disjoint_paths(const reader &r)
{
	auto C = std::make_shared<std::unordered_set<int>>();
	int c;
	r.scan("%d", &c);
	for (int i = 0; i < c; i++) {
		int u;
		r.scan("%d", &u);
		C->insert(u);
	}
	return C;
}


#endif //IMPL_INPUT_HPP
