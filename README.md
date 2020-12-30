# HyperText Programming Language

\#MakeHTMLAProgrammingLanguageAgain

Compile the HTPL Compiler with:

```
$ g++ compiler.cpp -o compiler
```

Then you can compile HTPL files as follows:

```
$ ./compile input_file.htpl output_file
```

The transpiled C source code file will be kept as `output_file.c`.
GCC is needed as a dependency.