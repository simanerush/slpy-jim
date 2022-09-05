slpy: slpy-lex.o slpy-parse.o slpy.o slpy-ast.o
	g++ -g -o slpy slpy-lex.o slpy-ast.o slpy-parse.o slpy.o

slpy-ast.o: slpy-ast.cc
	g++ -std=c++17 -g -c slpy-ast.cc

slpy-lex.o: slpy-lex.cc
	g++ -std=c++17 -g -c slpy-lex.cc

slpy-parse.o: slpy-parse.cc
	g++ -std=c++17 -g -c slpy-parse.cc

slpy.o: slpy.cc
	g++ -std=c++17 -g -c slpy.cc

slpy-lex.cc: slpy-lex.hh
slpy-parse.cc: slpy-parse.hh slpy-ast.hh
slpy.cc: slpy-lex.hh slpy-ast.hh slpy-parse.hh
slpy-ast.cc: slpy-ast.hh

clean:
	rm *~ *.o slpy
