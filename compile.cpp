#include <bits/stdc++.h>

#define VERBOSE
#define VERY_VERBOSE

#include "parser.hpp"
#include "transpiler.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Usage: ./compile <input_file_name> <output_file_name>");
		exit(1);
	}

	const char *input_file_name = argv[1];
	const char *output_file_name = argv[2];

	Parser parser(input_file_name);
	parser.print_tag(parser.root_tag);

	Transpiler transpiler(parser.root_tag, output_file_name);
}