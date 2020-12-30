#ifndef PARSER_HEADER
#define PARSER_HEADER

#define syntax_error(mess, ...) do { \
		fprintf(stderr, "Syntax Error at %ld:%ld: ", line, col); \
		fprintf(stderr, mess, ##__VA_ARGS__); \
		putc('\n', stderr); \
		exit(1); \
	} while (0)

#define syntax_error_at(line, col, mess, ...) do { \
		fprintf(stderr, "Syntax Error at %ld:%ld: ", line, col); \
		fprintf(stderr, mess, ##__VA_ARGS__); \
		putc('\n', stderr); \
		exit(1); \
	} while (0)

#define print_indent(size) for (size_t i = 0; i < size; i++) putc('\t', stdout)

#include <bits/stdc++.h>

#include "verbose.hpp"

using namespace std;

bool is_whitespace(char c)
{
	switch (c) {
		case ' ':
		case '\n':
		case '\t':
			return true;

		default:
			return false;
	}
}

bool is_valid_symbol_start(char c)
{
	if (c >= 'A' && c <= 'Z') return true;
	if (c >= 'a' && c <= 'z') return true;

	switch (c) {
		case '_':
		case '$':
			return true;

		default:
			return false;
	}
}

bool is_valid_symbol(char c)
{
	return is_valid_symbol_start(c) || (c >= '0' && c <= '9');
}

struct Tag {
	string name;
	unordered_map<string, string> attr;
	vector<Tag> body;
	size_t line;
	size_t col;
};

enum TagNameParsingMode {
	TAG_NAME_PARSE_BEFORE,
	TAG_NAME_PARSE_IN
};

enum AttrParsingMode {
	ATTR_PARSE_IDLE,
	ATTR_PARSE_NAME,
	ATTR_PARSE_VALUE
};

class Parser {
	public:
		FILE *file;
		size_t line = 1;
		size_t col = 0;
		Tag root_tag;

		Parser(const char *filename)
		{
			file = fopen(filename, "r");
			root_tag.name = "root";
			root_tag.body = parse_inner("root");
			root_tag.line = 1;
			root_tag.col = 1;
		}

		char next_char()
		{
			char c = fgetc(file);

			if (c == '\n') {
				line++;
				col = 0;
			} else col++;

			very_verbose("next_char() -> at %ld:%ld: '%c'", line, col, c);

			return c;
		}

		void push_back_char(char c)
		{
			ungetc(c, file);

			if (col != 0) col--;
			else {
				line--;

				// Todo: fix what col should become

				printf("Undefined behaviour incoming...\n");
			}
		}

		vector<Tag> parse_inner(string parent_tag_name)
		{
			vector<Tag> tags;
			char c;

			while (true) {
				c = next_char();

				if (c == EOF) {
					verbose("Found EOF, quitting...");
					break;
				}

				// Skip whitespace

				if (is_whitespace(c)) continue;

				if (c == '<') {
					// Match start of tag

					size_t tag_start_line = line;
					size_t tag_start_col = col;

					char maybe_end_tag = next_char();

					if (maybe_end_tag == '/') {
						// Scan tag name

						size_t end_tag_col = col;

						for (size_t i = 0; i < parent_tag_name.size(); i++) {
							if (next_char() != parent_tag_name[i]) {
								syntax_error_at(line, end_tag_col,
									"Found unexpected closing tag name. "
									"Expected a matching closing tag for \"%s\"",
									parent_tag_name.c_str());
							}
						}

						// Expect '>'

						if ((c = next_char()) != '>') {
							syntax_error("Unexpected '%c', "
								"expected end of closing tag ('>')", c);
						}

						verbose("Found end tag for \"%s\"", parent_tag_name.c_str());

						return tags;
					}

					push_back_char(maybe_end_tag);

					verbose("Found start of tag at %ld:%ld", line, col);

					Tag inner_tag;
					inner_tag.name = parse_tag_name();
					inner_tag.attr = parse_tag_attr();
					inner_tag.line = tag_start_line;
					inner_tag.col = tag_start_col;

					// Special tags which do not need a closing tag

					if (inner_tag.name != "br") {
						verbose("Parsing body of <%s> tag...", inner_tag.name.c_str());

						inner_tag.body = parse_inner(inner_tag.name);
						verbose("Parsed body of <%s> tag!", inner_tag.name.c_str());
					}

					tags.push_back(inner_tag);

					continue;
				} else {
					// Parse text

					Tag text_tag;
					text_tag.name = "text";
					text_tag.attr["value"] = parse_text_tag(c);
					text_tag.line = line;
					text_tag.col = col;

					tags.push_back(text_tag);

					continue;
				}

				// Unexpected char

				syntax_error("Unexpected '%c'", c);
			}

			return tags;
		}

		string parse_tag_name()
		{
			string tag_name = "";
			char c;
			enum TagNameParsingMode mode = TAG_NAME_PARSE_BEFORE;

			while (true) {
				c = next_char();

				if (c == EOF) syntax_error("Unexpected EOF");

				switch (mode) {
					case TAG_NAME_PARSE_BEFORE:
						if (is_whitespace(c)) continue;

						if (is_valid_symbol_start(c)) {
							verbose("Found start of tag name at %ld:%ld", line, col);

							tag_name += c;
							mode = TAG_NAME_PARSE_IN;

							continue;
						}

						// Unexpected char

						syntax_error("Unexpected '%c'", c);

					case TAG_NAME_PARSE_IN:
						if (is_whitespace(c) || c == '>') {
							verbose("Found end of tag name. Tag name = \"%s\"",
								tag_name.c_str());

							// Push the '>' back on the stream to support parse_tag_attr()

							if (c == '>') push_back_char('>');

							return tag_name;
						}

						if (is_valid_symbol(c)) {
							tag_name += c;
							continue;
						}

						// Unexpected char

						syntax_error("Unexpected '%c'", c);
				}
			}

			return tag_name;
		}

		unordered_map<string, string> parse_tag_attr()
		{
			unordered_map<string, string> attr;
			enum AttrParsingMode mode = ATTR_PARSE_IDLE;
			string attr_name;
			string attr_value;
			char c;

			while (true) {
				c = next_char();

				if (c == EOF) syntax_error("Unexpected EOF");

				switch (mode) {
					case ATTR_PARSE_IDLE:
						if (c == '>') return attr;
						if (is_whitespace(c)) continue;
						if (is_valid_symbol_start(c)) {
							attr_name += c;
							mode = ATTR_PARSE_NAME;
							continue;
						}

						// Unexpected char

						syntax_error("Unexpected '%c'", c);

					case ATTR_PARSE_NAME:
						if (c == '>') syntax_error("Unexpected '>'");

						if (is_whitespace(c)) syntax_error("Unexpected whitespace");

						if (c == '=') {
							if (next_char() != '"')
								syntax_error("Expected '\"', found '%c'", c);

							verbose("Found '=' after attr name. Attr name = \"%s\"",
								attr_name.c_str());

							mode = ATTR_PARSE_VALUE;
							continue;
						}

						if (is_valid_symbol(c)) {
							attr_name += c;
							continue;
						}

						// Unexpected char

						syntax_error("Unexpected '%c'", c);

					case ATTR_PARSE_VALUE:
						if (c == '"') {
							verbose("Found end of attr value. Attr value = \"%s\"",
								attr_value.c_str());

							attr[attr_name] = attr_value;
							attr_name = "";
							attr_value = "";

							mode = ATTR_PARSE_IDLE;

							continue;
						}

						attr_value += c;
				}
			}

			return attr;
		}

		string parse_text_tag(char first_char)
		{
			string text;
			text += first_char;

			char c;

			while (true) {
				c = next_char();

				if (c == '<') {
					push_back_char(c);
					verbose("Parsed text tag: \"%s\"", text.c_str());
					return text;
				}

				text += c;
			}
		}

		void print_tag(Tag& tag, size_t indent = 0)
		{
			print_indent(indent);
			printf("<%s> ", tag.name.c_str());

			if (tag.attr.size() != 0) {
				printf("attr: [ ");

				size_t i = 0;

				for (auto it = tag.attr.begin(); it != tag.attr.end(); it++, i++) {
					printf("\"%s\" -> \"%s\"", it->first.c_str(), it->second.c_str());
					if (i != tag.attr.size() - 1) printf(", ");
				}

				printf(" ]");
			}

			putc('\n', stdout);

			for (size_t i = 0; i < tag.body.size(); i++) {
				print_tag(tag.body[i], indent + 1);
			}
		}
};

#endif