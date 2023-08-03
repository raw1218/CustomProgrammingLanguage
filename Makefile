test: AST.c AST.h Parser.h Parser2.c CFG.h
	gcc -O3 -g  ASTWalker.c Lexer2.c Assembly.c CFG.c AST.c Parser2.c DataStructures/Map.c DataStructures/Stack.c DataStructures/Queue.c DataStructures/Set.c -o testprogram

test_multithreaded:
	gcc -g -fopenmp test.c -o test_multithreaded_program
