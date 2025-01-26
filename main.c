#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <unistd.h>

FILE *config_file;
SDL_Window *window;
SDL_Renderer *renderer;
IMG_Animation *animation;
SDL_Texture **Frames;
int width = 200, height = 200, pos_x = 0, pos_y = 0;
int transition = 1;
float aspect_ratio;
char chibi_path[1024];
char buf[1024] = {0};
char buf2[1024] = {0};
char temp_cfg_path[1024] = {0};
char cfg_path[1024] = {0};
bool volatile wait = true;
int current_frame_idx;
bool defaults = false;
static const SDL_DialogFileFilter filters[] = {
    {"All images", "png;jpg;jpeg;gif;webp"},
    {"PNG images", "png"},
    {"JPEG images", "jpg;jpeg"},
    {"GIF images", "gif"},
    {"Webp images", "webp"},
    {"All files", "*"}
};

static void SDLCALL callback(void *userdata, const char *const*filelist, int filter) {
    if (!filelist) {
        SDL_Log("An error occured: %s", SDL_GetError());
        wait = false;
        return;
    } else if (!*filelist) {
        SDL_Log("The user did not select any file.");
        SDL_Log("Most likely, the dialog was canceled.");
        wait = false;
        return;
    }

    if (*filelist) {
        current_frame_idx = 0;
        SDL_Log("Loading '%s'", *filelist);
        IMG_FreeAnimation(animation);
        animation = IMG_LoadAnimation(*filelist);
        if (animation == nullptr) {
            SDL_Log("Chibi not found, Quitting.'%s', Bailing Out, Goodbye!", SDL_GetError());
            exit(1);
        }
        SDL_Log("Loaded '%s'", *filelist);
        Frames = calloc(animation->count, sizeof(SDL_Texture));
        for (int i = 0; i < animation->count; ++i) {
            SDL_DestroyTexture(Frames[i]);
            Frames[i] = SDL_CreateTextureFromSurface(renderer, animation->frames[i]);
            SDL_SetTextureBlendMode(Frames[i],SDL_BLENDMODE_BLEND);
        }
        aspect_ratio = (float) animation->w / (float) animation->h;
        SDL_SetWindowAspectRatio(window, aspect_ratio, aspect_ratio);
        SDL_SetWindowSize(window, SDL_min(animation->w, 200 * aspect_ratio), SDL_min(animation->h, 200));
        SDL_SetWindowShape(window, animation->frames[0]);
        strcpy(chibi_path, *filelist);
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowPosition(window, &pos_x, &pos_y);
#ifdef SDL_PLATFORM_UNIX
        char *home = getenv("HOME");
        strcpy(buf, home);
        strcat(buf, "/.config/chibi-sdl");
        SDL_CreateDirectory(buf);
        strcat(buf, "/config.txt");
#else
        char *home = getenv("LocalAppData");
        strcpy(buf, home);
        strcat(buf, "/chibi-sdl");
        SDL_CreateDirectory(buf);
        strcat(buf, "/config.txt");
#endif
        if ((config_file = fopen(buf, "w")) == nullptr) {
            perror("fopen");
            exit(1);
        }
        SDL_Log("Writing to Config File");
        sprintf(buf, "%d,%d,%d,%d,%d,%s", width, height, pos_x, pos_y, transition, chibi_path);
        fputs(buf, config_file);
        fclose(config_file);
        wait = false;
    }
}

// ReSharper disable once CppDFAConstantFunctionResult
int main(int argc, char *argv[]) {
    char *home;
#ifdef SDL_PLATFORM_UNIX
    home = getenv("HOME");
    strcpy(temp_cfg_path, home);
    strcat(temp_cfg_path, "/.config/chibi-sdl");
    SDL_CreateDirectory(temp_cfg_path);
    strcat(temp_cfg_path, "/.config.tmp");
#else
    home = getenv("LocalAppData");
    strcpy(temp_cfg_path, home);
    strcat(temp_cfg_path, "/chibi-sdl");
    SDL_CreateDirectory(temp_cfg_path);
    strcat(temp_cfg_path, "/~config.tmp");
#endif
    if (!(argc > 1 && strcmp(argv[1], "--append") == 0)) {
        SDL_Log("Parent Process, Launch Subsequent chibis with --append");
        SDL_RemovePath(temp_cfg_path);
        if (argc > 1 && strcmp(argv[1], "--append") != 0) {
            if (argv[1][0] == '~') {
                strcpy(buf, home);
                strcat(buf, argv[1] + 1);
            }
            config_file = fopen(argv[1], "r");
            if (config_file == nullptr) {
                SDL_Log("'%s' does not exist", argv[1]);
                exit(1);
            }
            SDL_Log("Loading From '%s'", argv[1]);
            if (fgets(buf, 1024, config_file) == nullptr) {
                SDL_Log("Error in file structure");
                exit(1);
            }
            char *token = strtok(buf, ",");
            if (token == nullptr) {
                SDL_Log("Error in file strcture");
                exit(1);
            }
            width = (int) strtol(token, nullptr, 10);
            token = strtok(nullptr, ",");
            if (token == nullptr) {
                SDL_Log("Error in file strcture");
                exit(1);
            }
            height = (int) strtol(token, nullptr, 10);
            token = strtok(nullptr, ",");
            if (token == nullptr) {
                SDL_Log("Error in file strcture");
                exit(1);
            }
            pos_x = (int) strtol(token, nullptr, 10);
            token = strtok(nullptr, ",");
            if (token == nullptr) {
                SDL_Log("Error in file strcture");
                exit(1);
            }
            pos_y = (int) strtol(token, nullptr, 10);
            token = strtok(nullptr, ",");
            if (token == nullptr) {
                SDL_Log("Error in file strcture");
                exit(1);
            }
            transition = (int) strtol(token, nullptr, 10);
            token = strtok(nullptr, ",");
            if (token == nullptr) {
                SDL_Log("Error in file strcture");
                exit(1);
            }
            strcpy(chibi_path, token);
            chibi_path[strcspn(chibi_path, "\n")] = 0;
            fclose(config_file);
        } else {
#ifdef SDL_PLATFORM_UNIX
            home = getenv("HOME");
            strcpy(buf, home);
            strcat(buf, "/.config/chibi-sdl/config.txt");
#else
            home = getenv("LocalAppData");
            strcpy(buf, home);
            strcat(buf, ".chibi-sdl/config.txt");
#endif
            config_file = fopen(buf, "r");
            if (config_file != nullptr) {
                SDL_Log("Loading From '%s'", buf);
                if (fgets(buf, 1024, config_file) == nullptr) {
                    SDL_Log("Error in file structure");
                    exit(1);
                }
                bool parent = true;
                strcpy(buf2, buf);
                while (fgets(buf, 1024, config_file) != nullptr) {
                    if (fork() == 0) {
                        parent = false;
                        break;
                    }
                }
                if (parent == true)
                    strcpy(buf, buf2);
                char *token = strtok(buf, ",");
                if (token == nullptr) {
                    SDL_Log("Error in file strcture");
                    exit(1);
                }
                width = (int) strtol(token, nullptr, 10);

                token = strtok(nullptr, ",");
                if (token == nullptr) {
                    SDL_Log("Error in file strcture");
                    exit(1);
                }
                height = (int) strtol(token, nullptr, 10);

                token = strtok(nullptr, ",");
                if (token == nullptr) {
                    SDL_Log("Error in file strcture");
                    exit(1);
                }
                pos_x = (int) strtol(token, nullptr, 10);

                token = strtok(nullptr, ",");
                if (token == nullptr) {
                    SDL_Log("Error in file strcture");
                    exit(1);
                }
                pos_y = (int) strtol(token, nullptr, 10);
                token = strtok(nullptr, ",");
                if (token == nullptr) {
                    SDL_Log("Error in file strcture");
                    exit(1);
                }
                transition = (int) strtol(token, nullptr, 10);
                token = strtok(nullptr, ",");
                if (token == nullptr) {
                    SDL_Log("Error in file strcture");
                    exit(1);
                }
                strcpy(chibi_path, token);
                chibi_path[strcspn(chibi_path, "\n")] = 0;
                fclose(config_file);
            } else {
                SDL_Log("Loading Defaults");
                width = height = 200;
                pos_x = 0;
                pos_y = 0;
                defaults = true;
            }
        }
        if (!SDL_CreateWindowAndRenderer("Chibi", width, height,
                                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_TRANSPARENT |
                                         SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY |
                                         SDL_WINDOW_NOT_FOCUSABLE, &window,
                                         &renderer)) {
            SDL_Log("Couldnt Create Window and Renderer. '%s', Bailing Out, Goodbye!", SDL_GetError());
            exit(1);
        }
    } else {
        if (!SDL_CreateWindowAndRenderer("Chibi", 200, 200,
                                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_TRANSPARENT |
                                         SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY |
                                         SDL_WINDOW_NOT_FOCUSABLE, &window,
                                         &renderer)) {
            SDL_Log("Couldnt Create Window and Renderer. '%s', Bailing Out, Goodbye!", SDL_GetError());
            exit(1);
        }
        wait = true;
        SDL_ShowOpenFileDialog(callback, nullptr, window, filters, 6, nullptr, 0);
        while (wait) {
            SDL_PumpEvents();
        }
    }
    if (defaults) {
        wait = true;
        SDL_ShowOpenFileDialog(callback, nullptr, window, filters, 6, nullptr, 0);
        while (wait) {
            SDL_PumpEvents();
        }
    }
    if (!SDL_SetWindowPosition(window, pos_x, pos_y)) {
        SDL_Log("Couldnt Move Window. '%s', Bailing Out, Goodbye!", SDL_GetError());
        exit(1);
    }
    SDL_Log("Created Window %dx%d at %dx%d", width, height, pos_x, pos_y);
    animation = IMG_LoadAnimation(chibi_path);
    if (animation == nullptr) {
        SDL_Log("Chibi not found. '%s', Asking user for file", SDL_GetError());
        wait = true;
        SDL_ShowOpenFileDialog(callback, nullptr, window, filters, 6, nullptr, 0);
        while (wait) {
            SDL_PumpEvents();
        }
    }
    SDL_Log("Loaded '%s'", chibi_path);
    Frames = calloc(animation->count, sizeof(SDL_Texture));
    for (int i = 0; i < animation->count; ++i) {
        Frames[i] = SDL_CreateTextureFromSurface(renderer, animation->frames[i]);
        SDL_SetTextureBlendMode(Frames[i],SDL_BLENDMODE_BLEND);
    }
    SDL_Event event;
    bool quit = false;
    bool bordered = false;
    current_frame_idx = 0;
    int current_transparency_step = 0;
    unsigned int lastTime = 0, currentTime;
    aspect_ratio = (float) width / (float) height;
    while (!quit) {
        SDL_RenderClear(renderer);
        if (transition == 1) {
            if (current_transparency_step > 0) {
                currentTime = SDL_GetTicks();
                if (currentTime > lastTime + 100) {
                    current_transparency_step--;
                    for (int j = 0; j < animation->count; ++j) {
                        SDL_SetTextureAlphaMod(Frames[j], 51 * current_transparency_step);
                    }
                    lastTime = currentTime;
                }
            } else if (current_transparency_step < 0) {
                currentTime = SDL_GetTicks();
                if (currentTime > lastTime + 100) {
                    current_transparency_step++;
                    for (int j = 0; j < animation->count; ++j) {
                        SDL_SetTextureAlphaMod(Frames[j], 51 * (5 + current_transparency_step));
                    }
                    lastTime = currentTime;
                }
            }
        }
        if (current_frame_idx < animation->count) {
            SDL_RenderTexture(renderer, Frames[current_frame_idx], nullptr, nullptr);
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_EVENT_QUIT: quit = true;
                        break;
                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        if (event.button.clicks == 2 && event.button.button == SDL_BUTTON_LEFT) {
                            wait = true;
                            SDL_ShowOpenFileDialog(callback, nullptr, window, filters, 6, nullptr, 0);
                            while (wait) {
                                SDL_PumpEvents();
                            }
                        }
                        if (event.button.button == SDL_BUTTON_MIDDLE)
                            SDL_SetWindowAspectRatio(window, aspect_ratio, aspect_ratio);
                        if (event.button.button == SDL_BUTTON_RIGHT) {
                            SDL_SetWindowBordered(window, !bordered);
                            bordered = !bordered;
                        }
                        break;
                    case SDL_EVENT_WINDOW_MOUSE_ENTER:
                        current_transparency_step = 5;
                        break;
                    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                        current_transparency_step = -5;
                        break;
                    default: ;
                }
            }

            SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0,SDL_ALPHA_TRANSPARENT_FLOAT);
            SDL_SetWindowShape(window, animation->frames[current_frame_idx]);
            SDL_RenderPresent(renderer);
            SDL_Delay(animation->delays[current_frame_idx]);
            current_frame_idx++;
        } else {
            current_frame_idx = 0;
        }
    }
    SDL_GetWindowSize(window, &width, &height);
    SDL_GetWindowPosition(window, &pos_x, &pos_y);
#ifdef SDL_PLATFORM_UNIX
    home = getenv("HOME");
    strcpy(cfg_path, home);
    strcat(cfg_path, "/.config/chibi-sdl");
    SDL_CreateDirectory(cfg_path);
    strcat(cfg_path, "/config.txt");
#else
    home = getenv("LocalAppData");
    strcpy(cfg_path, home);
    strcat(cfg_path, "/chibi-sdl");
    SDL_CreateDirectory(cfg_path);
    strcat(cfg_path, "/config.txt");
#endif
    if ((config_file = fopen(temp_cfg_path, "a")) == nullptr) {
        perror("fopen");
        exit(1);
    }
    SDL_Log("Writing to Temporary Config File");
    sprintf(buf, "%d,%d,%d,%d,%d,%s\n", width, height, pos_x, pos_y, transition, chibi_path);
    fputs(buf, config_file);
    fclose(config_file);
    SDL_Log("Copying temporary config file to Permanent location");
    if (!SDL_CopyFile(temp_cfg_path, cfg_path)) {
        SDL_Log("%s", SDL_GetError());
    }
    IMG_FreeAnimation(animation);
    free(Frames);
    return 0;
}