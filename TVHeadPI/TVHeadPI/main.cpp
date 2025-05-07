
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
SDL_FRect mouth_position;
SDL_Event event;
SDL_Joystick* joystick;

int blinktimer = 33; //we start at 33 so no insta-blink shit

enum EyeExpression {
    NONE,
    HAPPY,
    SAD,
    ANGRY,
    SURPRISED
};

enum MouthExpression {
    MOUTH_NONE,
    MOUTH_HAPPY,
    MOUTH_SAD,
    MOUTH_ANGRY,
    MOUTH_SURPRISED
};

EyeExpression manual_expression = NONE;
MouthExpression mouth_expression = MOUTH_NONE;

bool konami_active = false;
bool konami_played_once = false;

bool expressions_locked = false;
EyeExpression locked_eye_expression = NONE;
MouthExpression locked_mouth_expression = MOUTH_NONE;

std::vector<int> input_buffer;
const std::vector<int> konami_code = { 12, 12, 13, 13, 14, 15, 14, 15, 1, 0 };

IMG_Animation* gif_animation = nullptr;
int gif_frame = 0;
Uint64 gif_last_time = 0;
bool gif_loaded = false;

void debug_print_joystick_buttons() {
    if (!joystick) return;
    SDL_PumpEvents();
    int num_buttons = SDL_GetNumJoystickButtons(joystick);
    for (int i = 0; i < num_buttons; ++i) {
        if (SDL_GetJoystickButton(joystick, i)) {
            printf("Button %d is pressed\n", i);
        }
    }
}

void init_joystick() {
    SDL_Init(SDL_INIT_JOYSTICK);
    int joystick_count;
    SDL_JoystickID* joysticks = SDL_GetJoysticks(&joystick_count);
    if (joystick_count > 0) {
        joystick = SDL_OpenJoystick(joysticks[0]);
    }
}

void check_blinking() {
    SDL_PumpEvents();
    Uint8 hat = SDL_GetJoystickHat(joystick, 0);

    switch (hat) {
    case SDL_HAT_UP: manual_expression = HAPPY; break;
    case SDL_HAT_DOWN: manual_expression = SAD; break;
    case SDL_HAT_LEFT: manual_expression = ANGRY; break;
    case SDL_HAT_RIGHT: manual_expression = SURPRISED; break;
    default: manual_expression = NONE; break;
    }

    if (manual_expression == NONE) {
        blinktimer++;
        if (blinktimer > 860) blinktimer = 0;
    }
    else {
        blinktimer = 140;
    }
}

void check_mouth_expression() {
    if (expressions_locked) return;

    if (SDL_GetJoystickButton(joystick, 0)) mouth_expression = MOUTH_HAPPY;
    else if (SDL_GetJoystickButton(joystick, 1)) mouth_expression = MOUTH_SAD;
    else if (SDL_GetJoystickButton(joystick, 2)) mouth_expression = MOUTH_ANGRY;
    else if (SDL_GetJoystickButton(joystick, 3)) mouth_expression = MOUTH_SURPRISED;
    else mouth_expression = MOUTH_NONE;

    const char* path = nullptr;
    switch (mouth_expression) {
    case MOUTH_HAPPY: path = "./images/mouth_happy.png"; break;
    case MOUTH_SAD: path = "./images/mouth_sad.png"; break;
    case MOUTH_ANGRY: path = "./images/mouth_angry.png"; break;
    case MOUTH_SURPRISED: path = "./images/mouth_surprised.png"; break;
    default: path = "./images/test_mouth.png"; break;
    }

    if (path) {
        SDL_Surface* surf = IMG_Load(path);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_DestroySurface(surf);
            if (mouth_texture) SDL_DestroyTexture(mouth_texture);
            mouth_texture = tex;
        }
    }
}

void check_rendering_eye_states() {
    if (!expressions_locked) {
        check_blinking();
    }

    const char* path = nullptr;
    EyeExpression current_eye_expression = expressions_locked ? locked_eye_expression : manual_expression;

    if (current_eye_expression != NONE) {
        switch (current_eye_expression) {
        case HAPPY: path = "./images/test_happy.png"; break;
        case SAD: path = "./images/test_sad.png"; break;
        case ANGRY: path = "./images/test_angry.png"; break;
        case SURPRISED: path = "./images/test_shocked.png"; break;
        default: break;
        }
    }
    else {
        if ((blinktimer >= 0 && blinktimer < 8) || (blinktimer >= 32))
            path = "./images/test_eyes.png";
        else if ((blinktimer >= 8 && blinktimer < 16) || (blinktimer >= 24 && blinktimer <= 32))
            path = "./images/test_half.png";
        else if (blinktimer > 16 && blinktimer < 24)
            path = "./images/test_closed.png";
    }

    if (path) {
        SDL_Surface* surf = IMG_Load(path);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_DestroySurface(surf);
            if (eye_texture) SDL_DestroyTexture(eye_texture);
            eye_texture = tex;
        }
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

    if (!SDL_CreateWindowAndRenderer("CRT STELLE", 1280, 640, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
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

                if (event.jbutton.button == 4) { //depends on the controller switch this to 9 if i have the PS controller else it's 4
                    expressions_locked = !expressions_locked;
                    if (expressions_locked) {
                        locked_eye_expression = manual_expression;
                        locked_mouth_expression = mouth_expression;
                    }
                }
            }

            if (event.type == SDL_EVENT_JOYSTICK_HAT_MOTION) {
                switch (event.jhat.value) {
                case SDL_HAT_UP: input_buffer.push_back(12); break;
                case SDL_HAT_DOWN: input_buffer.push_back(13); break;
                case SDL_HAT_LEFT: input_buffer.push_back(14); break;
                case SDL_HAT_RIGHT: input_buffer.push_back(15); break;
                default: break;
                }
            }

            if (input_buffer.size() > konami_code.size()) {
                input_buffer.erase(input_buffer.begin());
            }
            if (input_buffer == konami_code && !konami_played_once) {
                konami_active = true;
                konami_played_once = true;
                gif_frame = 0;
                gif_last_time = SDL_GetTicks();
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    konami_active = true;
                    konami_played_once = true;
                    gif_frame = 0;
                    gif_last_time = SDL_GetTicks();
                }
            }
        }

        int win_width, win_height;
        SDL_GetWindowSize(window, &win_width, &win_height);

        eye_position.w = win_width;
        eye_position.h = win_height;
        float eye_offset_x = get_axis(0) / 32767.0f * 25;
        float eye_offset_y = get_axis(1) / 32767.0f * 25;
        eye_position.x = (win_width - eye_position.w) / 2 + eye_offset_x;
        eye_position.y = (win_height - eye_position.h) / 2 + eye_offset_y;

        mouth_position.w = win_width;
        mouth_position.h = win_height;
        float mouth_offset_x = get_axis(2) / 32767.0f * 25;
        float mouth_offset_y = get_axis(3) / 32767.0f * 25;
        mouth_position.x = (win_width - mouth_position.w) / 2 + mouth_offset_x;
        mouth_position.y = (win_height - mouth_position.h) / 2 + mouth_offset_y;

        if (konami_active && gif_loaded) {
            Uint64 now = SDL_GetTicks();
            if (now - gif_last_time > gif_animation->delays[gif_frame]) {
                gif_frame++;
                gif_last_time = now;
            }

            if (gif_frame < gif_animation->count) {
                SDL_Texture* frame_texture = SDL_CreateTextureFromSurface(renderer, gif_animation->frames[gif_frame]);
                SDL_RenderClear(renderer);
                SDL_RenderTexture(renderer, frame_texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_DestroyTexture(frame_texture);
                continue;
            }
            else {
                konami_active = false;
                konami_played_once = false;
                input_buffer.clear();
            }
        }

        debug_print_joystick_buttons();
        check_rendering_eye_states();
        check_mouth_expression();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, eye_texture, NULL, &eye_position);
        SDL_RenderTexture(renderer, mouth_texture, NULL, &mouth_position);
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
