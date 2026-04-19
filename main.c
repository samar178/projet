#include "enigme.h"
#include <stdio.h>

// déclaration de la fonction animation (définie dans enigme.c)
void animation_fin(SDL_Renderer *r, TTF_Font *font, int score);

int main()
{
    // Initialisation SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL init error: %s\n", SDL_GetError());
        return 1;
    }

    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("Audio error: %s\n", Mix_GetError());
        return 1;
    }

    // Fenêtre + renderer
    SDL_Window *w = SDL_CreateWindow("Seven Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    SDL_Renderer *r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

    // Police
    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if(!font)
    {
        printf("Font error\n");
        return 1;
    }

    // Lancer le jeu
    int score = jouer(r, font);

    // ✅ Animation finale (remplace tout ancien affichage)
    animation_fin(r, font, score);

    // Nettoyage
    TTF_CloseFont(font);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
