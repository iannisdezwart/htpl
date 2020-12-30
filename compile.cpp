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

	string input_file_name = argv[1];
	string output_file_name = argv[2];
	string c_file_name = output_file_name + ".c";

	Parser parser(input_file_name);

	#ifdef VERBOSE
	printf("Parsed structure:\n");
	parser.print_tag(parser.root_tag);
	#endif

	Transpiler transpiler(parser.root_tag, c_file_name);

	printf("Sucessfully transpiled, compiling...\n");

	string command = "gcc " + c_file_name + " -o " + output_file_name;
	system(command.c_str());
}