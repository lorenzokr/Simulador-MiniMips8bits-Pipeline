compila:
		gcc pipeline.c -o teste2 -g -lncurses
		./teste2
clear:
		rm teste2
		rm arquivo_dados.txt
		rm assembly.asm

