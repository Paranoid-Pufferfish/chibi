#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

FILE *config_file;
SDL_Window *window;
SDL_Renderer *renderer;
IMG_Animation *animation = nullptr;
SDL_Texture **Frames = nullptr;
int width, height, pos_x, pos_y;
int transition = 1;
float aspect_ratio;
char buf[1024];
char chibi_path[1024] = "../Sitting.png";
static const SDL_DialogFileFilter filters[] = {
    {"PNG images", "png"},
    {"JPEG images", "jpg;jpeg"},
    {"GIF images", "gif"},
    {"Webp images", "webp"},
    {"All images", "png;jpg;jpeg;gif;webp"},
    {"All files", "*"}
};

static void SDLCALL callback(void *userdata, const char *const*filelist, int filter) {
    if (!filelist) {
        SDL_Log("An error occured: %s", SDL_GetError());
        return;
    } else if (!*filelist) {
        SDL_Log("The user did not select any file.");
        SDL_Log("Most likely, the dialog was canceled.");
        return;
    }

    if (*filelist) {
        SDL_Log("Loading '%s'", *filelist);
        IMG_FreeAnimation(animation);
        animation = IMG_LoadAnimation(*filelist);
        if (animation == nullptr) {
            SDL_Log("Chibi not found, Quitting.'%s', Bailing Out, Goodbye!",SDL_GetError());
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
        SDL_SetWindowSize(window, SDL_min(animation->w,200 * aspect_ratio), SDL_min(animation->h,200));
        strcpy(chibi_path, *filelist);
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowPosition(window, &pos_x, &pos_y);
#ifdef SDL_PLATFORM_UNIX
        char *home = getenv("HOME");
        strcpy(buf, home);
        strcat(buf, "/.config/el-creatura");
        SDL_CreateDirectory(buf);
        strcat(buf, "/config.txt");
#else
        char *home = getenv("LocalAppData");
        strcpy(buf, home);
        strcat(buf, "/el-creatura");
        SDL_CreateDirectory(buf);
        strcat(buf, "/config.txt");
#endif
        config_file = fopen(buf, "w");
        sprintf(buf, "%d,%d,%d,%d,%d,%s", width, height, pos_x, pos_y, transition, chibi_path);
        fputs(buf, config_file);
        fclose(config_file);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (argv[1][0] == '~') {
            char *home = getenv("HOME");
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
        char *home = getenv("HOME");
        strcpy(buf, home);
        strcat(buf, "/.config/el-creatura/config.txt");
#else
        char *home = getenv("LocalAppData");
        strcpy(buf, home);
        strcat(buf, ".el-creatura/config.txt");
#endif

        config_file = fopen(buf, "r");
        if (config_file != nullptr) {
            SDL_Log("Loading From '%s'", buf);
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
            SDL_Log("Loading Defaults");
            width = height = 200;
            pos_x = 1920 - 200;
            pos_y = 1080 - 200;
        }
    }

    if (!SDL_CreateWindowAndRenderer("EL creatura", width, height,
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_TRANSPARENT |
                                     SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY |
                                     SDL_WINDOW_NOT_FOCUSABLE, &window,
                                     &renderer)) {
        SDL_Log("Couldnt Create Window and Renderer. '%s', Bailing Out, Goodbye!",SDL_GetError());
        exit(1);
    }
    if (!SDL_SetWindowPosition(window, pos_x, pos_y)) {
        SDL_Log("Couldnt Move Window. '%s', Bailing Out, Goodbye!",SDL_GetError());
        exit(1);
    }
    SDL_Log("Created Window %dx%d at %dx%d", width, height, pos_x, pos_y);


    animation = IMG_LoadAnimation(chibi_path);
    if (animation == nullptr) {
        SDL_Log("Chibi not found. '%s', Bailing Out, Goodbye!",SDL_GetError());
        exit(1);
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
    int current_frame_idx = 0;
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
                        if (event.button.clicks == 2 && event.button.button == SDL_BUTTON_LEFT)
                            SDL_ShowOpenFileDialog(callback, nullptr, window, filters, 6, nullptr, 0);
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
                    default: ;
                }
            }
            SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0,SDL_ALPHA_TRANSPARENT_FLOAT);
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
    char *home = getenv("HOME");
    strcpy(buf, home);
    strcat(buf, "/.config/el-creatura");
    SDL_CreateDirectory(buf);
    strcat(buf, "/config.txt");
#else
    char *home = getenv("LocalAppData");
    strcpy(buf, home);
    strcat(buf, "/el-creatura");
    SDL_CreateDirectory(buf);
    strcat(buf, "/config.txt");
#endif
    config_file = fopen(buf, "w");
    sprintf(buf, "%d,%d,%d,%d,%d,%s", width, height, pos_x, pos_y, transition, chibi_path);
    fputs(buf, config_file);
    fclose(config_file);
    IMG_FreeAnimation(animation);
    free(Frames);
    return 0;
}
