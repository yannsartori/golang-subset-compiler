# Introduction
This project represents a compiler for a subset of the Go language, GoLite. This project was done as a semester-long project for a
compilers class at McGill University, [COMP 520 Compilers Design](https://www.cs.mcgill.ca/~cs520/2020/). This project was achieved by
[Neil Pereira](https://github.com/npereira97), [Denali Relles](https://github.com/sq-relles), and myself, where we contributed to 
non-assignment statements, statements, and expressions, respectively (this was not, of course, incredibly strict designations). 
The documentation for the language can be found [here](https://www.cs.mcgill.ca/~cs520/2020//slides/10-golite.pdf).

Read our report at doc/finalreport.pdf, where we share our thoughts on the project and design decisions

If you are currently in the class, I guess don't steal our stuff

# How to use
First, run the build script with no arguments. Then run the run script with two arguments: The compiler phase mode along with the file 
path. The acceptable modes are "scan", "tokens", "parse", "pretty", "symbol", "typecheck", and "codegen". These modes do a lexical scanning 
of the source file, do a lexical scanning while outputting the tokens to stdout, parse the source file, pretty print the file after having
been parsed successfully, symbolcheck and build the symbol table for the source file, typecheck the source file, and finally, outputs a
file called "go.out.c" which is the (unoptimized) C file representing the GoLite program, respectively. Each mode runs the modes on which
it depends automatically, and outputs errors to stderr.

# Project Contents
doc/ contains our reports for each milestone. programs/ contains testfiles **we** wrote ourselves, for each phase. src/ contains our
sourcefiles. There are the aforementioned scripts, along with test.sh, which executes the run script on each file in our programs/
directory, alongside execute.sh, which compiles and executes the file outputted after running the codegen. 
