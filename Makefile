jeu: main.c joueur/joueur.c background/background.c ennemi/ennemi.c minimap/minimap.c
	gcc main.c joueur/joueur.c background/background.c ennemi/ennemi.c minimap/minimap.c -o jeu -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
clean:
	rm -f jeu
