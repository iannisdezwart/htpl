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

Sample HTPL program source code:

```xml
<include>stdio.h</include>

<function name="main" returntype="int">
	<params>
		<param type="int">argc</param>
	</params>

	<body>
		<declare type="int">my_int</declare>

		<call callee="scanf">
			<arg>
				<string>
					%d
				</string>
			</arg>
			<arg>
				<symbol>&my_int</symbol>
			</arg>
		</call>

		<call callee="printf">
			<arg>
				<string>
					Hello, World!
					<br>
					You passed %d args!
					<br>
					You typed %d
					<br>
				</string>
			</arg>
			<arg>
				<symbol>argc</symbol>
			</arg>
			<arg>
				<symbol>my_int</symbol>
			</arg>
		</call>

		<return>
			<number>0</number>
		</return>
	</body>
</function>
```
