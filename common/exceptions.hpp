#ifndef IMPL_EXCEPTIONS_H
#define IMPL_EXCEPTIONS_H


class presentable_exception : public std::exception {
public:
	virtual std::string message() const noexcept = 0;
};


class invalid_input_exception : public presentable_exception {};


class graph_input_exception : public invalid_input_exception {
	std::string message() const noexcept override {
		return "Invalid graph input.";
	}
};

class disjoint_paths_input_exception : public invalid_input_exception {
	std::string message() const noexcept override {
		return "Invalid modulator to disjoint paths input.";
	}
};


class invalid_argument_exception : public invalid_input_exception {
private:
	std::string name;
	std::string value;
	std::string comment;

public:
	explicit invalid_argument_exception(const std::string &name, const std::string &value, const std::string &comment):
		name(name),
		value(value),
		comment(comment)
	{}

	std::string message() const noexcept override {
		return "Invalid " + name + " `" + value + "`. " + comment;
	}
};


class unknown_argument_exception : public invalid_input_exception {
private:
	std::string value;

public:
	explicit unknown_argument_exception(const std::string &value): value(value) {}

	std::string message() const noexcept override {
		return "Unknown argument`" + value + "`.";
	}
};


class missing_arguments_exception : public invalid_input_exception {
public:
	std::string message() const noexcept override {
		return "Missing arguments.";
	}
};


class open_file_exception : public presentable_exception {
private:
	std::string filename;
	std::string reason;

public:
	open_file_exception(const std::string &filename, const std::string &reason):
		filename(filename),
		reason(reason)
	{}

	std::string message() const noexcept override {
		return "Failed to open file `" + filename + "`: " + reason;
	}
};


class implementation_exception : std::exception {};


#endif //IMPL_EXCEPTIONS_H
