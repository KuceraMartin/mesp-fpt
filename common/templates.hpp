#ifndef IMPL_TEMPLATES_H
#define IMPL_TEMPLATES_H


std::string graph_format_desc()
{
	return
		"<vertex-count> <edge-count>\n"
		"<vertex-a> <vertex-b>\n"
		"...\n"
		"\n"
		"Vertices must be integers in range [0, <vertex-count> - 1].\n"
	;
}


std::string disjoint_paths_format_desc()
{
	return
		"<vertex-count>\n"
		"<vertex-1> <vertex-2> ...\n"
	;
}


std::string mesp_format_desc()
{
	return
		"<vertex-count> <path-ecc>\n"
		"<vertex-1> <vertex-2> ...\n"
	;
}


#endif //IMPL_TEMPLATES_H
