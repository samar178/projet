<<<<<<< HEAD
prog: main.o enigme.o
	gcc main.o enigme.o -o prog -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

main.o: main.c enigme.h
	gcc -c main.c

enigme.o: enigme.c enigme.h
	gcc -c enigme.c

clean:
	rm -f *.o prog
=======
puzzle_game: main.o game.o
	gcc main.o game.o -o puzzle_game -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm -g

main.o: main.c game.h
	gcc -c main.c -g -Wall

game.o: game.c game.h
	gcc -c game.c -g -Wall

clean:
	rm -f *.o puzzle_game
>>>>>>> 382a5b2 (first commit)
