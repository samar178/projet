#include "game.h"

int init_game(Game* game) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    if (IMG_Init(IMG_INIT_PNG) < 0) return -1;
    if (TTF_Init() < 0) return -1;
    
    game->window = SDL_CreateWindow("Enigme Puzzle",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1917, 947, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!game->window) return -1;
    
    game->renderer = SDL_CreateRenderer(game->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!game->renderer) return -1;
    
    game->state = SCREEN_SELECTION;
    game->selected_image = -1;
    game->is_fullscreen = 0;
    game->pieces_placed = 0;
    game->timer_active = 0;
    game->dragging_piece = -1;
    
    game->victory_effect = 0;
    game->victory_angle = 0;
    game->victory_scale = 1.0;
    game->victory_start_time = 0;
    
    game->font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf", 28);
    if (!game->font) game->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);
    
    load_assets(game);
    return 0;
}

void load_assets(Game* game) {
    char path[512];
    
    sprintf(path, "assets/background.png");
    SDL_Surface* surf = IMG_Load(path);
    if (surf) {
        game->background = SDL_CreateTextureFromSurface(game->renderer, surf);
        SDL_FreeSurface(surf);
        printf("✓ Loaded: %s\n", path);
    } else {
        printf("✗ Failed to load: %s\n", path);
    }
    
    const char* img_files[] = {"img1.png", "img2.png", "img3.png", "img4.png", "img5.png"};
    
    int total_width = 5 * 200 + 4 * 30;
    int start_x = (1917 - total_width) / 2;
    int start_y = (947 - 200) / 2;
    
    for (int i = 0; i < 5; i++) {
        sprintf(path, "assets/%s", img_files[i]);
        surf = IMG_Load(path);
        if (surf) {
            game->images[i] = SDL_CreateTextureFromSurface(game->renderer, surf);
            SDL_FreeSurface(surf);
            printf("✓ Loaded: %s\n", path);
        } else {
            printf("✗ Failed to load: %s\n", path);
        }
        
        game->image_rects[i].x = start_x + i * 230;
        game->image_rects[i].y = start_y;
        game->image_rects[i].w = 200;
        game->image_rects[i].h = 200;
    }
    
    int button_width = 120;
    int button_height = 120;
    int spacing = 20;
    int total_buttons_width = (button_width * 2) + spacing;
    int buttons_start_x = (1917 - total_buttons_width) / 2;
    
    sprintf(path, "assets/window.png");
    surf = IMG_Load(path);
    if (surf) {
        game->window_btn.texture = SDL_CreateTextureFromSurface(game->renderer, surf);
        SDL_FreeSurface(surf);
        printf("✓ Loaded: %s\n", path);
    } else {
        printf("✗ Failed to load: %s\n", path);
    }
    game->window_btn.rect.x = buttons_start_x;
    game->window_btn.rect.y = 10;
    game->window_btn.rect.w = button_width;
    game->window_btn.rect.h = button_height;
    game->window_btn.is_hovered = 0;
    
    sprintf(path, "assets/fullscreen.png");
    surf = IMG_Load(path);
    if (surf) {
        game->fullscreen_btn.texture = SDL_CreateTextureFromSurface(game->renderer, surf);
        SDL_FreeSurface(surf);
        printf("✓ Loaded: %s\n", path);
    } else {
        printf("✗ Failed to load: %s\n", path);
    }
    game->fullscreen_btn.rect.x = buttons_start_x + button_width + spacing;
    game->fullscreen_btn.rect.y = 10;
    game->fullscreen_btn.rect.w = button_width;
    game->fullscreen_btn.rect.h = button_height;
    game->fullscreen_btn.is_hovered = 0;
} 
void init_puzzle(Game* game) {
    char path[512];
    sprintf(path, "assets/img%d.png", game->selected_image + 1);
    SDL_Surface* surf = IMG_Load(path);
    if (surf) {
        game->chosen_image = SDL_CreateTextureFromSurface(game->renderer, surf);
        SDL_FreeSurface(surf);
        printf("✓ Loaded: %s\n", path);
    } else {
        printf("✗ Failed to load: %s\n", path);
    }
    
    int piece_w = 120;
    int piece_h = 120;
    int spacing = 10;
    int step = piece_w + spacing;
    
    int pieces_start_x = 30;
    int pieces_start_y = 370;
    
    int collection_start_x = 900;
    int collection_start_y = 370;
    
    game->collection_zone.x = 890;
    game->collection_zone.y = 360;
    game->collection_zone.w = 400;
    game->collection_zone.h = 400;
    
    int positions[9][2];
    for (int i = 0; i < 9; i++) {
        int col = i % 3;
        int row = i / 3;
        positions[i][0] = pieces_start_x + col * step;
        positions[i][1] = pieces_start_y + row * step;
    }
    
    for (int i = 0; i < 9; i++) {
        int r = rand() % 9;
        int temp_x = positions[i][0];
        int temp_y = positions[i][1];
        positions[i][0] = positions[r][0];
        positions[i][1] = positions[r][1];
        positions[r][0] = temp_x;
        positions[r][1] = temp_y;
    }
    
    int img_width, img_height;
    SDL_QueryTexture(game->chosen_image, NULL, NULL, &img_width, &img_height);
    
    for (int i = 0; i < 9; i++) {
        int grid_x = i % 3;
        int grid_y = i / 3;
        
        SDL_Rect src_rect = {
            grid_x * (img_width / 3),
            grid_y * (img_height / 3),
            img_width / 3,
            img_height / 3
        };
        
        SDL_Texture* piece_tex = SDL_CreateTexture(game->renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
            piece_w, piece_h);
        
        SDL_SetRenderTarget(game->renderer, piece_tex);
        SDL_Rect dest_rect = {0, 0, piece_w, piece_h};
        SDL_RenderCopy(game->renderer, game->chosen_image, &src_rect, &dest_rect);
        SDL_SetRenderTarget(game->renderer, NULL);
        
        game->pieces[i].texture = piece_tex;
        game->pieces[i].rect.x = positions[i][0];
        game->pieces[i].rect.y = positions[i][1];
        game->pieces[i].rect.w = piece_w;
        game->pieces[i].rect.h = piece_h;
        game->pieces[i].correct_x = collection_start_x + (grid_x * step);
        game->pieces[i].correct_y = collection_start_y + (grid_y * step);
        game->pieces[i].grid_x = grid_x;
        game->pieces[i].grid_y = grid_y;
        game->pieces[i].is_placed = 0;
        
        game->collection_grid[i].x = collection_start_x + (grid_x * step);
        game->collection_grid[i].y = collection_start_y + (grid_y * step);
        game->collection_grid[i].w = piece_w;
        game->collection_grid[i].h = piece_h;
    }
    
    game->pieces_placed = 0;
    game->timer_active = 0;
    game->victory_effect = 0;
    game->victory_angle = 0;
    game->victory_scale = 1.0;
}
void set_fullscreen(Game* game, int fullscreen) {
    if (fullscreen) {
        SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN);
        game->is_fullscreen = 1;
        printf("✓ FULLSCREEN mode\n");
    } else {
        SDL_SetWindowFullscreen(game->window, 0);
        game->is_fullscreen = 0;
        printf("✓ WINDOW mode\n");
    }
    SDL_Delay(100);
}

void handle_events(Game* game, SDL_Event* event) {
    if (event->type == SDL_QUIT) {
        exit(0);
    }
    
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    if (game->state == SCREEN_SELECTION) {
        if (game->window_btn.texture) {
            if (mouse_x >= game->window_btn.rect.x && mouse_x <= game->window_btn.rect.x + game->window_btn.rect.w &&
                mouse_y >= game->window_btn.rect.y && mouse_y <= game->window_btn.rect.y + game->window_btn.rect.h) {
                game->window_btn.is_hovered = 1;
            } else {
                game->window_btn.is_hovered = 0;
            }
        }
        
        if (game->fullscreen_btn.texture) {
            if (mouse_x >= game->fullscreen_btn.rect.x && mouse_x <= game->fullscreen_btn.rect.x + game->fullscreen_btn.rect.w &&
                mouse_y >= game->fullscreen_btn.rect.y && mouse_y <= game->fullscreen_btn.rect.y + game->fullscreen_btn.rect.h) {
                game->fullscreen_btn.is_hovered = 1;
            } else {
                game->fullscreen_btn.is_hovered = 0;
            }
        }
        
        if (event->type == SDL_MOUSEBUTTONDOWN) {
            int x = event->button.x;
            int y = event->button.y;
            
            for (int i = 0; i < 5; i++) {
                if (x >= game->image_rects[i].x && x <= game->image_rects[i].x + 200 &&
                    y >= game->image_rects[i].y && y <= game->image_rects[i].y + 200) {
                    game->selected_image = i;
                    init_puzzle(game);
                    game->state = SCREEN_PUZZLE;
                    printf("Selected image: %d\n", i);
                    break;
                }
            }
            
            if (game->window_btn.texture) {
                SDL_Rect btn = game->window_btn.rect;
                if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
                    set_fullscreen(game, 0);
                }
            }
            
            if (game->fullscreen_btn.texture) {
                SDL_Rect btn = game->fullscreen_btn.rect;
                if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
                    set_fullscreen(game, 1);
                }
            }
        }
    }
    else if (game->state == SCREEN_PUZZLE) {
        if (!game->timer_active && game->pieces_placed < 9) {
            game->start_time = SDL_GetTicks();
            game->timer_active = 1;
        }
        
        if (event->type == SDL_MOUSEBUTTONDOWN) {
            int x = event->button.x;
            int y = event->button.y;
            
            for (int i = 0; i < 9; i++) {
                if (x >= game->pieces[i].rect.x && x <= game->pieces[i].rect.x + game->pieces[i].rect.w &&
                    y >= game->pieces[i].rect.y && y <= game->pieces[i].rect.y + game->pieces[i].rect.h) {
                    
                    game->dragging_piece = i;
                    game->drag_offset_x = x - game->pieces[i].rect.x;
                    game->drag_offset_y = y - game->pieces[i].rect.y;
                    
                    if (game->pieces[i].is_placed) {
                        game->pieces[i].is_placed = 0;
                        game->pieces_placed--;
                        printf("Piece removed from collection zone\n");
                    }
                    break;
                }
            }
        }
        else if (event->type == SDL_MOUSEMOTION && game->dragging_piece != -1) {
            int x = event->motion.x;
            int y = event->motion.y;
            game->pieces[game->dragging_piece].rect.x = x - game->drag_offset_x;
            game->pieces[game->dragging_piece].rect.y = y - game->drag_offset_y;
        }
        else if (event->type == SDL_MOUSEBUTTONUP && game->dragging_piece != -1) {
            int piece_x = game->pieces[game->dragging_piece].rect.x;
            int piece_y = game->pieces[game->dragging_piece].rect.y;
            int piece_w = game->pieces[game->dragging_piece].rect.w;
            int piece_h = game->pieces[game->dragging_piece].rect.h;
            
            int piece_center_x = piece_x + piece_w/2;
            int piece_center_y = piece_y + piece_h/2;
            
            if (piece_center_x >= game->collection_zone.x && piece_center_x <= game->collection_zone.x + game->collection_zone.w &&
                piece_center_y >= game->collection_zone.y && piece_center_y <= game->collection_zone.y + game->collection_zone.h) {
                
                int min_dist = 10000;
                int closest_index = -1;
                
                for (int i = 0; i < 9; i++) {
                    int grid_center_x = game->collection_grid[i].x + game->collection_grid[i].w/2;
                    int grid_center_y = game->collection_grid[i].y + game->collection_grid[i].h/2;
                    int dist = abs(piece_center_x - grid_center_x) + abs(piece_center_y - grid_center_y);
                    
                    if (dist < min_dist) {
                        min_dist = dist;
                        closest_index = i;
                    }
                }
                
                if (closest_index != -1) {
                    game->pieces[game->dragging_piece].rect.x = game->collection_grid[closest_index].x;
                    game->pieces[game->dragging_piece].rect.y = game->collection_grid[closest_index].y;
                    
                    if (!game->pieces[game->dragging_piece].is_placed) {
                        game->pieces[game->dragging_piece].is_placed = 1;
                        game->pieces_placed++;
                        printf("Piece placed at slot: %d\n", closest_index);
                    }
                }
            }
            
            game->dragging_piece = -1;
            
            if (game->pieces_placed == 9 && !game->victory_effect) {
                game->victory_effect = 1;
                game->victory_start_time = SDL_GetTicks();
                game->victory_angle = 0;
                game->victory_scale = 0.5;
                printf("\n🎉 SUCCÈS - PUZZLE COMPLET! 🎉\n");
            }
        }
    }
}
void render(Game* game) {
    SDL_RenderClear(game->renderer);
    
    if (game->background) {
        SDL_RenderCopy(game->renderer, game->background, NULL, NULL);
    }
    
    if (game->state == SCREEN_SELECTION) {
        render_screen_selection(game);
    } else {
        render_screen_puzzle(game);
    }
    
    if (game->victory_effect == 1) {
        Uint32 elapsed = SDL_GetTicks() - game->victory_start_time;
        
        if (elapsed < 2000) {
            game->victory_angle = (elapsed * 360) / 1000;
            game->victory_scale = 1.0 + (sin(elapsed * 0.01) * 0.3);
            
            char victory_text[] = "🎉 SUCCES! 🎉";
            SDL_Color color = {255, 215, 0, 255};
            
            TTF_SetFontSize(game->font, 48);
            SDL_Surface* surf = TTF_RenderText_Blended(game->font, victory_text, color);
            if (surf) {
                SDL_Texture* text_tex = SDL_CreateTextureFromSurface(game->renderer, surf);
                SDL_Rect dest = {1917/2 - (surf->w * game->victory_scale)/2, 
                                 947/2 - (surf->h * game->victory_scale)/2, 
                                 surf->w * game->victory_scale, 
                                 surf->h * game->victory_scale};
                SDL_RenderCopyEx(game->renderer, text_tex, NULL, &dest, 
                                game->victory_angle, NULL, SDL_FLIP_NONE);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(text_tex);
            }
        } else {
            game->victory_effect = 2;
        }
    }
    
    if (game->victory_effect == 2) {
        char continue_text[] = "Cliquez pour continuer...";
        TTF_SetFontSize(game->font, 24);
        SDL_Surface* surf = TTF_RenderText_Blended(game->font, continue_text, (SDL_Color){200,200,200,255});
        if (surf) {
            SDL_Texture* text_tex = SDL_CreateTextureFromSurface(game->renderer, surf);
            SDL_Rect dest = {1917/2 - surf->w/2, 947/2 + 100, surf->w, surf->h};
            SDL_RenderCopy(game->renderer, text_tex, NULL, &dest);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(text_tex);
        }
        
        int mouse_x, mouse_y;
        Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
        if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            game->victory_effect = 0;
            game->state = SCREEN_SELECTION;
        }
    }
    
    SDL_RenderPresent(game->renderer);
}

void render_screen_selection(Game* game) {
    for (int i = 0; i < 5; i++) {
        if (game->images[i]) {
            SDL_RenderCopy(game->renderer, game->images[i], NULL, &game->image_rects[i]);
        }
    }
    
    if (game->window_btn.texture) {
        if (game->window_btn.is_hovered) {
            SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 150);
            SDL_RenderFillRect(game->renderer, &game->window_btn.rect);
        }
        SDL_RenderCopy(game->renderer, game->window_btn.texture, NULL, &game->window_btn.rect);
    }
    
    if (game->fullscreen_btn.texture) {
        if (game->fullscreen_btn.is_hovered) {
            SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 150);
            SDL_RenderFillRect(game->renderer, &game->fullscreen_btn.rect);
        }
        SDL_RenderCopy(game->renderer, game->fullscreen_btn.texture, NULL, &game->fullscreen_btn.rect);
    }
}

void render_screen_puzzle(Game* game) {
    SDL_Rect img_rect = {30, 30, 250, 250};
    if (game->chosen_image) {
        SDL_RenderCopy(game->renderer, game->chosen_image, NULL, &img_rect);
    }
    
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_Rect pieces_area = {20, 360, 400, 400};
    SDL_RenderDrawRect(game->renderer, &pieces_area);
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 60);
    SDL_RenderFillRect(game->renderer, &pieces_area);
    
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_Rect collection_area = {890, 360, 400, 400};
    SDL_RenderDrawRect(game->renderer, &collection_area);
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 60);
    SDL_RenderFillRect(game->renderer, &collection_area);
    
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
    for (int i = 0; i < 9; i++) {
        SDL_RenderDrawRect(game->renderer, &game->collection_grid[i]);
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 40);
        SDL_RenderFillRect(game->renderer, &game->collection_grid[i]);
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
    }
    
    for (int i = 0; i < 9; i++) {
        if (game->pieces[i].texture) {
            SDL_RenderCopy(game->renderer, game->pieces[i].texture, NULL, &game->pieces[i].rect);
        }
    }
    
    if (game->timer_active || game->pieces_placed > 0) {
        Uint32 elapsed = (SDL_GetTicks() - game->start_time) / 1000;
        int minutes = elapsed / 60;
        int seconds = elapsed % 60;
        
        char time_text[32];
        sprintf(time_text, "Time: %02d:%02d", minutes, seconds);
        
        if (game->font) {
            TTF_SetFontSize(game->font, 28);
            SDL_Surface* surf = TTF_RenderText_Blended(game->font, time_text, (SDL_Color){255,255,255,255});
            if (surf) {
                SDL_Texture* text_tex = SDL_CreateTextureFromSurface(game->renderer, surf);
                SDL_Rect text_rect = {1917 / 2 - 70, 10, 140, 35};
                SDL_RenderCopy(game->renderer, text_tex, NULL, &text_rect);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(text_tex);
            }
        }
        
        float progress = (float)game->pieces_placed / 9.0;
        SDL_Rect progress_bar = {1917 / 2 - 150, 55, 300, 20};
        SDL_Rect progress_fill = {1917 / 2 - 150, 55, (int)(300 * progress), 20};
        
        SDL_SetRenderDrawColor(game->renderer, 60, 60, 60, 255);
        SDL_RenderFillRect(game->renderer, &progress_bar);
        SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(game->renderer, &progress_fill);
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(game->renderer, &progress_bar);
    }
}

void update(Game* game) {}

void cleanup_game(Game* game) {
    if (game->background) SDL_DestroyTexture(game->background);
    for (int i = 0; i < 5; i++) {
        if (game->images[i]) SDL_DestroyTexture(game->images[i]);
    }
    if (game->window_btn.texture) SDL_DestroyTexture(game->window_btn.texture);
    if (game->fullscreen_btn.texture) SDL_DestroyTexture(game->fullscreen_btn.texture);
    if (game->chosen_image) SDL_DestroyTexture(game->chosen_image);
    for (int i = 0; i < 9; i++) {
        if (game->pieces[i].texture) SDL_DestroyTexture(game->pieces[i].texture);
    }
    if (game->font) TTF_CloseFont(game->font);
    
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
