#include "enigme.h"
#include <stdio.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048) < 0)
    {
        printf("Audio error: %s\n", Mix_GetError());
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow("Seven Game",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,0);

    SDL_Renderer *r = SDL_CreateRenderer(w,-1,0);

    TTF_Font *font = TTF_OpenFont("arial.ttf",24);
    if(!font)
    {
        printf("Font error\n");
        return 1;
    }

    int score = jouer(r,font);

    SDL_SetRenderDrawColor(r,0,0,0,255);
    SDL_RenderClear(r);

    char txt[50];
    sprintf(txt,"Score: %d / 10",score);

    draw_text(r,font,txt,300,250);

    if(score > 5)
        draw_text(r,font,"YOU WIN",320,320);
    else
        draw_text(r,font,"YOU LOSE",320,320);

    SDL_RenderPresent(r);

    SDL_Delay(4000);

    return 0;
}
