#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "background/background.h"
#include "joueur/joueur.h"
#include "ennemi/ennemi.h"
#include "minimap/minimap.h"

// --- FONCTIONS UTILITAIRES ---

void afficherTexte(SDL_Renderer* renderer, TTF_Font* font, const char* texte, int x, int y, SDL_Color couleur) {
    SDL_Surface* s = TTF_RenderText_Blended(font, texte, couleur);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_Rect r = {x - s->w/2, y - s->h/2, s->w, s->h};
    SDL_RenderCopy(renderer, t, NULL, &r);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

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

// NOUVEAU : Fonction pour dessiner le rectangle de score
void dessinerRectangleScore(SDL_Renderer* renderer, TTF_Font* font, int score) {
    // Fond noir semi-transparent (70% opacité)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 178); 
    SDL_Rect rectScore = {20, 150, 300, 80}; // Placé sous les coeurs
    SDL_RenderFillRect(renderer, &rectScore);

    // Bordure jaune (2 pixels d'épaisseur via deux rectangles)
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); 
    SDL_RenderDrawRect(renderer, &rectScore);
    SDL_Rect rectScoreInterne = {21, 151, 298, 78};
    SDL_RenderDrawRect(renderer, &rectScoreInterne);

    // Texte du score
    char texteScore[50];
    sprintf(texteScore, "SCORE: %d", score);
    SDL_Color jaune = {255, 255, 0, 255};
    // Position x=170, y=190 (Centre du rectangle)
    afficherTexte(renderer, font, texteScore, 170, 190, jaune); 
}

// --- MAIN ---

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() == -1) return 1;
    
    SDL_Window* window = SDL_CreateWindow("7SINS - Projet Yassine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Background bg; Joueur j; Ennemi e20; Minimap mn;
    const int SEUIL_VERROUILLAGE = 7680;

    initBackground(&bg, renderer, 1);
    initJoueur(&j, renderer);
    initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960);
    initMinimap(&mn, renderer);

    j.vie = 3; j.sante = 100.0f; j.endurance = 100.0f; j.invulnerable = 0; e20.sante = 100.0f;

    SDL_Texture* texCoeur = IMG_LoadTexture(renderer, "background/ressources/coeur.png");
    SDL_Color rouge = {255, 0, 0, 255}, vert = {0, 255, 0, 255}, blanc = {255, 255, 255, 255}, jaune = {255, 255, 0, 255};
    
    int run = 1, gameOver = 0, enPause = 0, optionPause = 0; 
    SDL_Event event;
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    while (run) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) run = 0;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p && !gameOver) {
                enPause = !enPause;
                if (!enPause) {
                    bg.startTime = SDL_GetTicks() - bg.tempsEcoule; 
                }
            }

            if (enPause) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
                        optionPause = !optionPause; 
                    }
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        if (optionPause == 0) { 
                            enPause = 0;
                            bg.startTime = SDL_GetTicks() - bg.tempsEcoule; 
                        } else { 
                            run = 0;
                        }
                    }
                }
            } else {
                if (!gameOver && j.etat != MORT) gestionEntrees(&j, keys, &event);
                else if (gameOver && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                    initJoueur(&j, renderer); // Réinitialise aussi le score à 0
                    initBackground(&bg, renderer, 1);
                    initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960);
                    j.vie = 3; j.sante = 100.0f; j.endurance = 100.0f; gameOver = 0;
                }
            }
        }

        if (!gameOver && !enPause) {
            bg.tempsEcoule = SDL_GetTicks() - bg.startTime; 

            if (j.invulnerable > 0) j.invulnerable--;
            if (j.endurance < 100.0f) j.endurance += 0.4f;

            if ((j.vie <= 0 || j.sante <= 0) && j.etat != MORT) { j.etat = MORT; j.frame = 0; }
            if (j.etat == MORT && j.mort_terminee) gameOver = 1;

            if (j.etat != MORT) {
                int v = (j.etat == SPRINT || j.etat == SPRINT_G) ? 14 : 8;
                int old_jx = j.pos.x, old_cx = bg.camera_pos.x, old_ex = e20.pos.x;

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

                if (bg.camera_pos.x >= SEUIL_VERROUILLAGE - 1920) {
                    e20.actif = 1;
                    updateEnnemi(&e20, j_hitbox);
                }

                if (e20.actif && !e20.mort_terminee) {
                    if (collisionTrigonometrique(j_hitbox, e20.pos)) {
                        int ancienne_distance = abs((old_jx + old_cx) - old_ex);
                        int nouvelle_distance = abs(absX_joueur - e20.pos.x);
                        if (nouvelle_distance < ancienne_distance) {
                            j.pos.x = old_jx; bg.camera_pos.x = old_cx; e20.pos.x = old_ex;
                        }
                        if (j.etat == FRAPPE || j.etat == FRAPPE_G) {
                            if (j.endurance > 5.0f) { 
                                e20.sante -= 0.8f; j.endurance -= 1.5f; 
                                if (e20.sante <= 0) { 
                                    e20.sante = 0; e20.etat = E_MORT; 
                                    j.score += 500; // NOUVEAU : +500 points pour avoir tué l'ennemi
                                } 
                            }
                        } else {
                            e20.etat = E_ATTAQUE;
                            if (j.invulnerable == 0 && j.etat != ACCROUPI) j.sante -= (bg.niveau_actuel == 1) ? 0.2f : 0.4f;
                        }
                    } else if (e20.etat == E_ATTAQUE) e20.etat = E_PATROUILLE;
                }

                for (int i = 0; i < bg.nb_obstacles; i++) {
                    if (bg.tab_obstacles[i].actif && collisionTrigonometrique(j_hitbox, bg.tab_obstacles[i].pos)) {
                        if (j.invulnerable == 0) { j.vie--; j.invulnerable = 60; j.pos.x = old_jx - 100; }
                        break;
                    }
                }

                // NOUVEAU : GESTION DE LA TRANSITION DE NIVEAU ET DU BONUS DE TEMPS
                if (bg.niveau_actuel == 1 && e20.mort_terminee && j.pos.x >= 1800) {
                    
                    int temps_sec = bg.tempsEcoule / 1000;
                    int temps_ref = 120; // Le temps de référence est fixé à 2 minutes
                    int bonus_temps = 0;
                    
                    if (temps_sec < temps_ref) {
                        bonus_temps = (temps_ref - temps_sec) * 50; // 50 pts par seconde sauvée
                    }
                    
                    int score_base_niveau = 1000; // Score accordé simplement pour avoir fini le niveau
                    
                    // --- ECRAN DE BENCHMARKING (Dure 4 secondes) ---
                    Uint32 startTransition = SDL_GetTicks();
                    while(SDL_GetTicks() - startTransition < 4000) {
                        SDL_Event t_event;
                        while(SDL_PollEvent(&t_event)) { 
                            if(t_event.type == SDL_QUIT) { run = 0; break; } 
                        }
                        if(!run) break;

                        SDL_RenderClear(renderer);
                        afficherBackground(bg, renderer);
                        
                        // Fond sombre pour faire ressortir le texte
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
                        SDL_Rect overlay = {0, 0, 1920, 1080};
                        SDL_RenderFillRect(renderer, &overlay);
                        
                        char t1[100], t2[100], t3[100];
                        sprintf(t1, "NIVEAU 1 TERMINE ! (+%d PTS)", score_base_niveau);
                        sprintf(t2, "TEMPS : %02d:%02d - BONUS EFFICACITE : +%d PTS", temps_sec/60, temps_sec%60, bonus_temps);
                        sprintf(t3, "NOUVEAU SCORE TOTAL : %d PTS", j.score + score_base_niveau + bonus_temps);
                        
                        afficherTexte(renderer, bg.font, t1, 1920/2, 400, blanc);
                        afficherTexte(renderer, bg.font, t2, 1920/2, 520, jaune);
                        afficherTexte(renderer, bg.font, t3, 1920/2, 640, vert);
                        
                        SDL_RenderPresent(renderer);
                    }
                    // ------------------------------------------------
                    
                    // Application du score
                    j.score += score_base_niveau + bonus_temps;

                    // Transition vers le niveau 2
                    libererBackground(&bg); initBackground(&bg, renderer, 2);
                    j.pos.x = 100; bg.camera_pos.x = 0; initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960);
                }
                updateMinimap(&mn, absX_joueur, e20.pos.x);
            } else { updatePhysique(&j); }
        }

        SDL_RenderClear(renderer);
        afficherBackground(bg, renderer); 
        updateEtAfficherDecors(&bg, renderer); 
        afficherObstacles(bg, renderer);
        
        if (!gameOver) {
            if (e20.actif) {
                afficherEnnemi(e20, renderer, bg.camera_pos);
                if (!e20.mort_terminee) {
                    int ex_relatif = e20.pos.x - bg.camera_pos.x;
                    dessinerBarreEntite(renderer, ex_relatif, e20.pos.y - 30, e20.pos.w, 8, e20.sante, 100.0f, rouge);
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
                SDL_Rect rC = {20 + (i * 60), 90, 50, 50}; SDL_RenderCopy(renderer, texCoeur, NULL, &rC);
            }
            
            // NOUVEAU : Affichage constant du Rectangle de Score
            dessinerRectangleScore(renderer, bg.font, j.score);
            
            afficherBarreNiveau(&bg, renderer); 
            afficherTemps(&bg, renderer); 
            afficherMinimap(mn, renderer, bg.niveau_actuel, !e20.mort_terminee);

            if (enPause) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
                SDL_Rect overlay = {0, 0, 1920, 1080};
                SDL_RenderFillRect(renderer, &overlay);

                afficherTexte(renderer, bg.font, "PAUSE", 1920/2, 400, blanc);
                afficherTexte(renderer, bg.font, "REPRENDRE", 1920/2, 550, (optionPause == 0) ? jaune : blanc);
                afficherTexte(renderer, bg.font, "QUITTER", 1920/2, 650, (optionPause == 1) ? jaune : blanc);
            }
        } else {
            afficherTexte(renderer, bg.font, "GAME OVER - R: REJOUER", 1920/2, 1080/2, rouge);
        }
        SDL_RenderPresent(renderer);
    }

    libererJoueur(&j); libererBackground(&bg); libererEnnemi(&e20); Liberer(&mn);
    SDL_DestroyTexture(texCoeur); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
    TTF_Quit(); SDL_Quit();
    return 0;
}
