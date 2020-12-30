#ifndef TRANSPILER_HEADER
#define TRANSPILER_HEADER

#define semantic_error(tag, mess, ...) do { \
		fprintf(stderr, "Semantic Error in \"%s\" tag at %ld:%ld: ", \
			tag.name.c_str(), tag.line, tag.col); \
		fprintf(stderr, mess, ##__VA_ARGS__); \
		putc('\n', stderr); \
		exit(1); \
	} while (0)

#include <bits/stdc++.h>

#include "parser.hpp"

using namespace std;

class Transpiler {
	public:
		FILE *file;

		Transpiler(Tag& root_tag, const char *file_path)
		{
			file = fopen(file_path, "w");

			for (size_t i = 0; i < root_tag.body.size(); i++) {
				transpile_tag(root_tag.body[i], 0);
			}

			fclose(file);
		}

		#define write(str) fwrite(str.c_str(), 1, str.size(), file)

		void transpile_tag(Tag& tag, size_t indent)
		{
			if (tag.name == "include") {
				// Expect text tag

				if (tag.body.size() != 1 || tag.body[0].name != "text") {
					semantic_error(tag, "Expected a file path\n"
						"Example: <include>stdio.h</include>");
				}

				if (tag.body[0].attr["value"] == "") {
					semantic_error(tag, "File path cannot be empty\n"
						"Example: <include>stdio.h</include>");
				}

				// Todo: check if file path exists

				string s = "#include \"" + tag.body[0].attr["value"] + "\"\n";
				verbose("Transpiled <include> tag at %ld:%ld: %s",
					tag.line, tag.col, s.c_str());
				write(s);

				return;
			}

			if (tag.name == "function") {
				Tag *body_tag = NULL;
				Tag *params_tag = NULL;

				verbose("Transpiling <function> tag at %ld:%ld...",
					tag.line, tag.col);

				for (size_t i = 0; i < tag.body.size(); i++) {
					if (tag.body[i].name == "body") {
						body_tag = &tag.body[i];
						continue;
					}

					if (tag.body[i].name == "params") {
						params_tag = &tag.body[i];
						continue;
					}

					semantic_error(tag, "Only <body> and <params> tags are valid as a "
					"direct child of a <function> tag");
				}

				if (body_tag == NULL) {
					semantic_error(tag, "A <function> tag must have a "
						"<body> tag inside");
				}

				if (!tag.attr.count("returntype")) {
					semantic_error(tag, "A <function> tag must have a \"returntype\" "
						"attribute.\nExample: <function returntype=\"void\" ...>");
				}

				if (!tag.attr.count("name")) {
					semantic_error(tag, "A <function> tag must have a \"name\" "
						"attribute.\nExample: <function name=\"main\" ...>");
				}

				string s = tag.attr["returntype"] + ' ' + tag.attr["name"] + '(';

				if (params_tag != NULL) {
					for (size_t i = 0; i < params_tag->body.size(); i++) {
						Tag& param = params_tag->body[i];

						if (param.name != "param") {
							semantic_error(param,
								"Only <param> tags are valid inside <params>");
						}

						if (!param.attr.count("type")) {
							semantic_error(param,
								"<param> tags must have a \"type\" attribute\n"
								"Example: <param type=\"int\">argc</param>");
						}

						if (param.body.size() != 1 || param.body[0].name != "text") {
							semantic_error(param,
								"Expected a symbol name for the parameter\n"
								"Example: <param type=\"int\">argc</param>");
						}

						s += param.attr["type"] + ' ' + param.body[0].attr["value"];

						if (i != params_tag->body.size() - 1) s += ", ";
					}
				}

				s += ")\n{\n";
				write(s);

				transpile_tag(*body_tag, indent + 1);

				s = "}\n";
				verbose("Transpiled <function> tag at %ld:%ld: %s",
					tag.line, tag.col, s.c_str());
				write(s);

				return;
			}

			if (tag.name == "body") {
				verbose("Transpiling <body> tag at %ld:%ld...",
					tag.line, tag.col);

				for (size_t i = 0; i < tag.body.size(); i++) {
					transpile_tag(tag.body[i], indent);
				}

				verbose("Finished transpiling <body> tag at %ld:%ld",
					tag.line, tag.col);

				return;
			}

			if (tag.name == "call") {
				verbose("Transpiling <call> tag at %ld:%ld...",
					tag.line, tag.col);

				if (!tag.attr.count("callee")) {
					semantic_error(tag, "Expected \"callee\" attribute on <call> tag\n"
						"Example: <call callee=\"my_function\"></call>");
				}

				string s = string(indent, '\t') + tag.attr["callee"] + '(';

				for (size_t i = 0; i < tag.body.size(); i++) {
					Tag& arg_tag = tag.body[i];

					if (arg_tag.name != "arg") {
						semantic_error(arg_tag,
							"Only <arg> tags are valid inside an <args> tag");
					}

					if (arg_tag.body.size() != 1) {
						semantic_error(arg_tag, "<arg> tags expect a single inner tag\n"
						 "Found %ld tags instead\nExample: <arg><number>0</number></arg>",
							arg_tag.body.size());
					}

					s += transpile_literal(arg_tag.body[0]);
					if (i != tag.body.size() - 1) s += ", ";
				}

				s += ");\n";
				verbose("Transpiled <call> tag at %ld:%ld: %s",
					tag.line, tag.col, s.c_str());
				write(s);

				return;
			}

			if (tag.name == "return") {
				if (tag.body.size() != 1) {
					semantic_error(tag, "<return> tags expect a single inner tag\n"
						"Found %ld tags instead\nExample: "
						"<return><number>0</number></return>",
						tag.body.size());
				}

				string s = string(indent, '\t') + "return ";
				s += transpile_literal(tag.body[0]);
				s += ";\n";
				verbose("Transpiled <return> tag at %ld:%ld: %s",
					tag.line, tag.col, s.c_str());
				write(s);

				return;
			}

			// Unknown tag name

			semantic_error(tag, "Unknown tag name \"%s\"", tag.name.c_str());
		}

		string transpile_literal(Tag& tag)
		{
			if (tag.name == "symbol") {
				if (tag.body.size() != 1 || tag.body[0].name != "text") {
					semantic_error(tag, "Expected symbol name inside <symbol> tag\n"
						"Example: <symbol>argc</symbol>");
				}

				return tag.body[0].attr["value"];
			}

			if (tag.name == "number") {
				if (tag.body.size() != 1 || tag.body[0].name != "text") {
					semantic_error(tag, "Expected a number inside <number> tag\n"
						"Example: <number>123</number>");
				}

				// Todo: do number validity checking

				return tag.body[0].attr["value"];
			}

			if (tag.name == "string") {
				if (tag.body.size() == 0) {
					semantic_error(tag, "<string> tag cannot be empty");
				}

				string s = "\"";

				for (size_t i = 0; i < tag.body.size(); i++) {
					Tag& inner_tag = tag.body[i];

					if (inner_tag.name == "text") {
						s += inner_tag.attr["value"];
						continue;
					}

					if (inner_tag.name == "br") {
						s += "\\n";
						continue;
					}

					semantic_error(inner_tag,
						"Unexpected <%s> tag inside <string> tag\n"
						"Only text and <br> tags are valid",
						inner_tag.name.c_str());
				}

				s += '\"';

				return s;
			}

			printf("Uh oh, Transpiler::transpile_literal() was called on a "
				"<%s> tag, which is not a literal.\nI messed something up...",
				tag.name.c_str());
			exit(1);
		}
};

#endif