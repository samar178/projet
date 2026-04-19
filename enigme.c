#include "enigme.h"
#include <stdio.h>

typedef struct {
    const char *q;
    const char *r1;
    const char *r2;
    const char *r3;
    int correct;
} Question;

void draw_text(SDL_Renderer *r, TTF_Font *font, const char *text, int x, int y)
{
    SDL_Color c = {255,255,255};
    SDL_Surface *s = TTF_RenderText_Blended(font, text, c);
    if(!s) return;

    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect pos = {x,y,s->w,s->h};

    SDL_RenderCopy(r, t, NULL, &pos);

    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

int menu(SDL_Renderer *r, TTF_Font *font)
{
    SDL_Event e;

    while(1)
    {
        SDL_SetRenderDrawColor(r,0,0,0,255);
        SDL_RenderClear(r);

        draw_text(r,font,"SEVEN GAME",300,200);
        draw_text(r,font,"Press ENTER to START",230,300);

        SDL_RenderPresent(r);

        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) return 0;

            if(e.type == SDL_KEYDOWN &&
               e.key.keysym.sym == SDLK_RETURN)
                return 1;
        }

        SDL_Delay(16);
    }
}

void play_sound(const char *path)
{
    Mix_Music *music = Mix_LoadMUS(path);
    if(!music) return;

    Mix_PlayMusic(music,1);
}

void show_feedback(SDL_Renderer *r, const char *imgPath)
{
    SDL_Surface *s = IMG_Load(imgPath);
    if(!s) return;

    SDL_Texture *t = SDL_CreateTextureFromSurface(r,s);

    SDL_Rect imgRect = {520,180,220,220};

    SDL_RenderCopy(r,t,NULL,&imgRect);
    SDL_RenderPresent(r);

    SDL_Delay(800);

    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}

int ask(SDL_Renderer *r, TTF_Font *font, Question q)
{
    SDL_Event e;
    int rep = -1;
    Uint32 start = SDL_GetTicks();

    while(rep == -1)
    {
        int timeLeft = 10 - (SDL_GetTicks()-start)/1000;
        if(timeLeft <= 0) return 0;

        SDL_SetRenderDrawColor(r,0,0,0,255);
        SDL_RenderClear(r);

        draw_text(r,font,q.q,50,80);
        draw_text(r,font,q.r1,50,180);
        draw_text(r,font,q.r2,50,230);
        draw_text(r,font,q.r3,50,280);
        draw_text(r,font,"Press 1 / 2 / 3",50,350);

        char timer[50];
        sprintf(timer,"Time: %d",timeLeft);
        draw_text(r,font,timer,650,20);

        SDL_RenderPresent(r);

        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) return 0;

            if(e.type == SDL_KEYDOWN)
            {
                if(e.key.keysym.sym == SDLK_1 || e.key.keysym.sym == SDLK_KP_1)
                    rep = 1;

                if(e.key.keysym.sym == SDLK_2 || e.key.keysym.sym == SDLK_KP_2)
                    rep = 2;

                if(e.key.keysym.sym == SDLK_3 || e.key.keysym.sym == SDLK_KP_3)
                    rep = 3;
            }
        }

        SDL_Delay(16);
    }

    // feedback in same page
    if(rep == q.correct)
    {
        play_sound("sound/correct.mp3");
        show_feedback(r,"images/correct.png");
        return 1;
    }
    else
    {
        play_sound("sound/wrong.mp3");
        show_feedback(r,"images/wrong.png");
        return 0;
    }
}

int jouer(SDL_Renderer *r, TTF_Font *font)
{
    if(!menu(r,font)) return 0;

    Question q[10] = {
        {"First sin?","1-Pride","2-Gluttony","3-Envy",2},
        {"Genre?","1-Romance","2-Thriller","3-Comedy",2},
        {"Number of sins?","1-5","2-7","3-10",2},
        {"Old detective?","1-Somerset","2-Mills","3-John",1},
        {"Young detective?","1-Jack","2-Mills","3-David",2},
        {"Killer?","1-John Doe","2-Mike","3-Batman",1},
        {"Last sin?","1-Wrath","2-Greed","3-Lust",1},
        {"City?","1-NY","2-Unknown","3-LA",2},
        {"Box?","1-Book","2-Head","3-Money",2},
        {"Theme?","1-Crime","2-Fantasy","3-Sci-fi",1}
    };

    int score = 0;

    for(int i=0;i<10;i++)
    {
        score += ask(r,font,q[i]);
    }

    return score;
}
