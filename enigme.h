#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

void draw_text(SDL_Renderer *r, TTF_Font *font, const char *text, int x, int y);
int jouer(SDL_Renderer *r, TTF_Font *font);

#endif
