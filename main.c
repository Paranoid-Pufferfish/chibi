#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <err.h>
#include "vendor/SDL/include/SDL3/SDL.h"
#include "vendor/SDL_image/include/SDL3_image/SDL_image.h"

struct chibi_opts {
    bool trans;
    int xpos, ypos;
    int w, h;
    char *file;
};

struct chibi_opts chibi = {.trans = false};

SDL_Window *window;
SDL_Renderer *renderer;
IMG_Animation *animation;
SDL_Texture **frames;

void usage() {
    errx(1, "usage: [-f|--file path/to/image] [-p|--position <xpos>x<ypos>] [-s|--size <w>x<h>] [-t|--transparency]");
}

void parse(int argc, char **argv) {
    /* options descriptor */
    static struct option longopts[] = {
        {"file", required_argument, nullptr, 'f'},
        {"position", required_argument, nullptr, 'p'},
        {"size", required_argument, nullptr, 's'},
        {"transparency", no_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}
    };
    int ch;
    char *endptr;
    char *pos = calloc(10, sizeof(char));
    char *size = calloc(10, sizeof(char));
    char *file = calloc(1024, sizeof(char));
    while ((ch = getopt_long(argc, argv, "tp:s:f:", longopts, nullptr)) != -1) {
        switch (ch) {
            case 't':
                chibi.trans = true;
                break;
            case 'p':
                strncpy(pos, optarg, 10);
                break;
            case 's':
                strncpy(size, optarg, 10);
                break;
            case 'f':
                strncpy(file, optarg, 1024);
                chibi.file = file;
                break;
            default:
                usage();
        }
    }

    char *p = pos;
    chibi.xpos = (int) strtol(strsep(&pos, "x"), &endptr, 10);
    if (p[0] == '\0' || *endptr != '\0') {
        if (p[0] != '\0')
            fprintf(stderr, "Incorrect Argument : %s\n", p);
        usage();
    }
    p = pos;
    chibi.ypos = (int) strtol(strsep(&pos, "x"), &endptr, 10);
    if (p[0] == '\0' || *endptr != '\0') {
        if (p[0] != '\0')
            fprintf(stderr, "Incorrect Argument : %s\n", p);
        usage();
    }
    p = size;
    chibi.w = (int) strtol(strsep(&size, "x"), &endptr, 10);
    if (p[0] == '\0' || *endptr != '\0') {
        if (p[0] != '\0')
            fprintf(stderr, "Incorrect Argument : %s\n", p);
        usage();
    }
    p = size;
    chibi.h = (int) strtol(strsep(&size, "x"), &endptr, 10);
    if (p[0] == '\0' || *endptr != '\0') {
        if (p[0] != '\0')
            fprintf(stderr, "Incorrect Argument : %s\n", p);
        usage();
    }
    if (file[0] == '\0')
        usage();

    FILE *img = fopen(file, "r");
    if (img == nullptr) {
        printf("File not found");
        usage();
    }
    fclose(img);
}

int main(int argc, char **argv) {
    parse(argc, argv);
    if (!SDL_CreateWindowAndRenderer(chibi.file, chibi.w, chibi.h,
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_TRANSPARENT |
                                     SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY |
                                     SDL_WINDOW_NOT_FOCUSABLE, &window,
                                     &renderer)) {
        SDL_Log("Couldnt Create Window and Renderer. '%s', Bailing Out, Goodbye!", SDL_GetError());
        exit(1);
    }
    if (!SDL_SetWindowPosition(window, chibi.xpos, chibi.ypos)) {
        SDL_Log("Couldnt Move Window. '%s', Bailing Out, Goodbye!", SDL_GetError());
        exit(1);
    }
    animation = IMG_LoadAnimation(chibi.file);
    if (animation == nullptr) {
        SDL_Log("%s is not a supported file. '%s', Bailing Out, Goodbye!", chibi.file, SDL_GetError());
        exit(1);
    }
    if ((frames = calloc(animation->count, sizeof(SDL_Texture))) == nullptr)
        errx(1, "calloc");

    for (int i = 0; i < animation->count; ++i) {
        frames[i] = SDL_CreateTextureFromSurface(renderer, animation->frames[i]);
        SDL_SetTextureBlendMode(frames[i],SDL_BLENDMODE_BLEND);
    }
    bool quit = false, bordered = false;
    int curridx = 0;
    SDL_Event event;
    float aspect_ratio = (float) chibi.w / (float) chibi.h;
    unsigned int lastTime = 0, currentTime;
    int current_transparency_step = 0;
    while (!quit) {
        SDL_SetWindowIcon(window, animation->frames[curridx]);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, frames[curridx], nullptr, nullptr);
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: quit = true;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        SDL_SetWindowBordered(window, !bordered);
                        bordered = !bordered;
                    }
                    if (event.button.clicks == 2 && event.button.button == SDL_BUTTON_LEFT)
                        SDL_SetWindowAspectRatio(window, aspect_ratio, aspect_ratio);
                    break;
                case SDL_EVENT_WINDOW_MOUSE_ENTER:
                    current_transparency_step = 5;
                    break;
                case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                    current_transparency_step = -5;
                    break;
                default:
                    break;
            }
        }
        if (chibi.trans == 1) {
            if (current_transparency_step > 0) {
                currentTime = SDL_GetTicks();
                if (currentTime > lastTime + 100) {
                    current_transparency_step--;
                    for (int j = 0; j < animation->count; ++j) {
                        SDL_SetTextureAlphaMod(frames[j], 51 * current_transparency_step);
                    }
                    lastTime = currentTime;
                }
            } else if (current_transparency_step < 0) {
                currentTime = SDL_GetTicks();
                if (currentTime > lastTime + 100) {
                    current_transparency_step++;
                    for (int j = 0; j < animation->count; ++j) {
                        SDL_SetTextureAlphaMod(frames[j], 51 * (5 + current_transparency_step));
                    }
                    lastTime = currentTime;
                }
            }
        }
        SDL_SetWindowShape(window, animation->frames[curridx]);
        SDL_RenderPresent(renderer);
        SDL_Delay(animation->delays[curridx]);
        if (curridx + 1 < animation->count) {
            curridx++;
        } else {
            curridx = 0;
        }
    }
    return 0;
}
