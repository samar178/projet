#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include "background/background.h"
#include "joueur/joueur.h"
#include "ennemi/ennemi.h"
#include "minimap/minimap.h"

// Fonction pour dessiner les barres (Santé et Endurance)
void dessinerBarreEntite(SDL_Renderer* renderer, int x, int y, int w, int h, float valeur, float max_valeur, SDL_Color couleur) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect fond = {x, y, w, h};
    SDL_RenderFillRect(renderer, &fond);
    if (valeur < 0) valeur = 0;
    int largeur = (int)((valeur / max_valeur) * w);
    SDL_SetRenderDrawColor(renderer, couleur.r, couleur.g, couleur.b, 255);
    SDL_Rect remplissage = {x, y, largeur, h};
    SDL_RenderFillRect(renderer, &remplissage);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() == -1) return 1;
    SDL_Window* window = SDL_CreateWindow("7SINS - Système de Combat", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Background bg; Joueur j; Ennemi e20; Minimap mn;
    const int SEUIL_VERROUILLAGE = 7680;

    initBackground(&bg, renderer, 1); 
    initJoueur(&j, renderer); 
    initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960); 
    initMinimap(&mn, renderer);

    // Initialisation des stats
    j.vie = 3;           // Coeurs pour les obstacles
    j.sante = 100.0f;    // Barre rouge pour le combat
    j.endurance = 100.0f; 
    j.invulnerable = 0;
    e20.sante = 100.0f;

    SDL_Texture* texCoeur = IMG_LoadTexture(renderer, "background/ressources/coeur.png");
    SDL_Color rouge = {255, 0, 0}, vert = {0, 255, 0};
    int run = 1, gameOver = 0;
    SDL_Event event;
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    while (run) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) run = 0;
            if (!gameOver) {
                if (j.etat != MORT) gestionEntrees(&j, keys, &event);
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                initJoueur(&j, renderer); initBackground(&bg, renderer, 1); 
                initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960);
                j.vie = 3; j.sante = 100.0f; j.endurance = 100.0f; gameOver = 0;
            }
        }

        if (!gameOver) {
            if (j.invulnerable > 0) j.invulnerable--;
            if (j.endurance < 100.0f) j.endurance += 0.4f;

            if ((j.vie <= 0 || j.sante <= 0) && j.etat != MORT) {
                j.etat = MORT; j.frame = 0;
            }
            if (j.etat == MORT && j.mort_terminee) gameOver = 1;

            if (j.etat != MORT) {
                int v = (j.etat == SPRINT || j.etat == SPRINT_G) ? 14 : 8;
                int old_jx = j.pos.x;

                // Mouvements et Scrolling
                if (keys[SDL_SCANCODE_RIGHT]) {
                    bg.direction = 0;
                    if (bg.camera_pos.x >= SEUIL_VERROUILLAGE && !e20.mort_terminee) {
                        if (j.pos.x < 1800) j.pos.x += v;
                    } else if (j.pos.x >= 960 && bg.camera_pos.x < SEUIL_VERROUILLAGE) scrolling(&bg, v);
                    else if (j.pos.x < 1850) j.pos.x += v;
                }
                if (keys[SDL_SCANCODE_LEFT]) {
                    bg.direction = 1;
                    if (j.pos.x <= 960 && bg.camera_pos.x > 0) scrolling(&bg, v);
                    else if (j.pos.x > 50) j.pos.x -= v;
                }

                updatePhysique(&j);
                int absX_joueur = j.pos.x + bg.camera_pos.x;
                SDL_Rect j_hitbox = {absX_joueur, j.pos.y, j.pos.w, j.pos.h};
                
                // 1. Obstacles (Dégâts aux cœurs)
                for (int i = 0; i < bg.nb_obstacles; i++) {
                    if (bg.tab_obstacles[i].actif && collisionTrigonometrique(j_hitbox, bg.tab_obstacles[i].pos)) {
                        if (j.invulnerable == 0) {
                            j.vie--; j.invulnerable = 60; j.pos.x = old_jx - 150;
                        }
                        break;
                    }
                }

                // 2. Ennemi (Combat et Dégâts réalistes)
                if (bg.camera_pos.x >= SEUIL_VERROUILLAGE - 1920) e20.actif = 1;
                if (e20.actif && !e20.mort_terminee) {
                    updateEnnemi(&e20);
                    if (collisionTrigonometrique(j_hitbox, e20.pos)) {
                        
                        if (j.etat == FRAPPE || j.etat == FRAPPE_G) {
                            // Dégâts basés sur l'endurance restante
                            if (j.endurance > 5.0f) {
                                float degats = (j.endurance / 100.0f) * 0.8f; 
                                e20.sante -= degats;
                                j.endurance -= 1.2f; // Consomme de l'endurance par coup
                            } else {
                                e20.sante -= 0.02f; // Épuisé = quasiment aucun dégât
                            }
                            
                            if (e20.sante <= 0) { e20.sante = 0; e20.etat = E_MORT; }
                            
                        } else if (j.invulnerable == 0 && e20.etat == E_ATTAQUE) { // <-- L'ennemi doit être en train d'attaquer
                            j.sante -= 0.5f; 
                        }
                    }
                }

                // Transition Niveau 2
                if (bg.niveau_actuel == 1 && e20.mort_terminee && j.pos.x >= 1800) {
                    libererBackground(&bg); initBackground(&bg, renderer, 2); 
                    j.pos.x = 100; bg.camera_pos.x = 0;
                    initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960); 
                }
                updateMinimap(&mn, absX_joueur, e20.pos.x);
            } else {
                updatePhysique(&j); 
            }
        }

        SDL_RenderClear(renderer);
        afficherBackground(bg, renderer);
        updateEtAfficherDecors(&bg, renderer);
        afficherObstacles(bg, renderer);
        
        if (!gameOver) {
            if (e20.actif) {
                afficherEnnemi(e20, renderer, bg.camera_pos);
                if (!e20.mort_terminee) {
                    int ex = e20.pos.x - bg.camera_pos.x;
                    dessinerBarreEntite(renderer, ex, e20.pos.y - 30, e20.pos.w, 8, e20.sante, 100.0f, rouge);
                }
            }
            if (j.invulnerable % 10 < 5) {
                afficherJoueur(j, renderer);
                if (j.etat != MORT) {
                    dessinerBarreEntite(renderer, j.pos.x, j.pos.y - 30, j.pos.w, 8, j.sante, 100.0f, rouge);
                    dessinerBarreEntite(renderer, j.pos.x, j.pos.y - 22, j.pos.w, 6, j.endurance, 100.0f, vert);
                }
            }
            for(int i = 0; i < j.vie; i++) {
                SDL_Rect rC = {20 + (i * 60), 20, 50, 50};
                SDL_RenderCopy(renderer, texCoeur, NULL, &rC);
            }
            afficherBarreNiveau(&bg, renderer);
            afficherMinimap(mn, renderer, bg.niveau_actuel, !e20.mort_terminee);
        } else {
            SDL_Surface* sMsg = TTF_RenderText_Blended(bg.font, "GAME OVER - R: REJOUER", rouge);
            SDL_Texture* tMsg = SDL_CreateTextureFromSurface(renderer, sMsg);
            SDL_Rect rMsg = {1920/2 - sMsg->w/2, 1080/2 - sMsg->h/2, sMsg->w, sMsg->h};
            SDL_RenderCopy(renderer, tMsg, NULL, &rMsg);
            SDL_FreeSurface(sMsg); SDL_DestroyTexture(tMsg);
        }
        SDL_RenderPresent(renderer);
    }
    libererJoueur(&j); libererBackground(&bg); libererEnnemi(&e20);
    return 0;
}
