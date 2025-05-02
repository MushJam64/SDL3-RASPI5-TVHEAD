#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_video.h>
#include <vector>
#include <stdio.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* eye_texture;
SDL_Texture* mouth_texture;
SDL_FRect eye_position;
SDL_Event event;
SDL_Joystick* joystick;

int blinktimer = 0;

enum EyeExpression {
    NONE,
    HAPPY,
    SAD,
    ANGRY,
    SURPRISED
};

EyeExpression manual_expression = NONE;
bool konami_active = false;

std::vector<int> input_buffer;
const std::vector<int> konami_code = {12, 12, 13, 13, 14, 15, 14, 15, 1, 0};

IMG_Animation* gif_animation = nullptr;
int gif_frame = 0;
Uint64 gif_last_time = 0;
bool gif_loaded = false;

void init_joystick() {
    SDL_Init(SDL_INIT_JOYSTICK);
    int joystick_count;
    SDL_JoystickID* joysticks = SDL_GetJoysticks(&joystick_count);
    if (joystick_count > 0) {
        joystick = SDL_OpenJoystick(joysticks[0]);
    }
}

void check_blinking() {
    if (SDL_GetJoystickButton(joystick, 0)) manual_expression = HAPPY;
    else if (SDL_GetJoystickButton(joystick, 1)) manual_expression = SAD;
    else if (SDL_GetJoystickButton(joystick, 2)) manual_expression = ANGRY;
    else if (SDL_GetJoystickButton(joystick, 3)) manual_expression = SURPRISED;
    else manual_expression = NONE;

    if (manual_expression == NONE) {
        blinktimer++;
        if (blinktimer > 860) blinktimer = 0;
    } else {
        blinktimer = 0;
    }
}

void check_rendering_eye_states() {
    check_blinking();
    const char* path = nullptr;

    if (manual_expression != NONE) {
        switch (manual_expression) {
            case HAPPY: path = "./images/test_happy.png"; break;
            case SAD: path = "./images/test_sad.png"; break;
            case ANGRY: path = "./images/test_angry.png"; break;
            case SURPRISED: path = "./images/test_surprised.png"; break;
            default: break;
        }
    } else {
        if ((blinktimer >= 0 && blinktimer < 60) || (blinktimer >= 240))
            path = "./images/test_eyes.png";
        else if ((blinktimer >= 60 && blinktimer < 120) || (blinktimer >= 180 && blinktimer <= 240))
            path = "./images/test_half.png";
        else if (blinktimer > 120 && blinktimer < 180)
            path = "./images/test_closed.png";
    }

    if (path) {
        SDL_Surface* surf = IMG_Load(path);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
        if (eye_texture) SDL_DestroyTexture(eye_texture);
        eye_texture = tex;
    }
}

float get_axis(int index) {
    if (!joystick) return 0;
    SDL_PumpEvents();
    return SDL_GetJoystickAxis(joystick, index);
}

int main(int argc, char* argv[]) {
    init_joystick();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("SDL Konami", 1280, 640, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window/renderer: %s", SDL_GetError());
        return 1;
    }

    SDL_Surface* mouth_surf = IMG_Load("./images/test_mouth.png");
    mouth_texture = SDL_CreateTextureFromSurface(renderer, mouth_surf);
    SDL_DestroySurface(mouth_surf);

    gif_animation = IMG_LoadAnimation("./images/retro.gif");
    if (gif_animation) {
        gif_loaded = true;
    }

    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) return 0;

            if (event.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN) {
                input_buffer.push_back(event.jbutton.button);
                if (input_buffer.size() > konami_code.size()) {
                    input_buffer.erase(input_buffer.begin());
                }
                if (input_buffer == konami_code) {
                    konami_active = true;
                    gif_frame = 0;
                    gif_last_time = SDL_GetTicks();
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                /* the pressed key was Escape? */
                if (event.key.key == SDLK_ESCAPE) {
                    konami_active = true;
                    gif_frame = 0;
                    gif_last_time = SDL_GetTicks();
                }
            }
        }

        eye_position.w = 1280;
        eye_position.h = 640;
        eye_position.x = get_axis(0) / 32767.0f * 25;
        eye_position.y = get_axis(1) / 32767.0f * 25;

        if (konami_active && gif_loaded) {
            Uint64 now = SDL_GetTicks();
            if (now - gif_last_time > gif_animation->delays[gif_frame]) {
                gif_frame = (gif_frame + 1) % gif_animation->count;
                gif_last_time = now;
            }

            SDL_Texture* frame_texture = SDL_CreateTextureFromSurface(renderer, gif_animation->frames[gif_frame]);
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, frame_texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_DestroyTexture(frame_texture);
            continue;
        }

        check_rendering_eye_states();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, eye_texture, NULL, &eye_position);
        SDL_RenderTexture(renderer, mouth_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    if (eye_texture) SDL_DestroyTexture(eye_texture);
    if (mouth_texture) SDL_DestroyTexture(mouth_texture);
    if (gif_loaded) IMG_FreeAnimation(gif_animation);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
