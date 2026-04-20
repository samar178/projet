#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
    SDL_Texture* backgroundTextures[2]; // Index 0: niveau 1, Index 1: niveau 2
    SDL_Rect minimapPosition;           // Zone d'affichage 480x54

    SDL_Texture* playerTexture;         // Icône joueur
    SDL_Rect playerPosition;            // Position calculée

    SDL_Texture* ennemiTexture;         // Icône ennemi
    SDL_Rect ennemiPosition;            // Position calculée
} Minimap;

void initMinimap(Minimap* m, SDL_Renderer* renderer);
void updateMinimap(Minimap* m, int absX_joueur, int absX_ennemi);
void afficherMinimap(Minimap m, SDL_Renderer* renderer, int niveau_actuel, int ennemiVivant);
void Liberer(Minimap* m);

#endif
