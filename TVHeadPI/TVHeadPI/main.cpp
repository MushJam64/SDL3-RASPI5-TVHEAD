#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_joystick.h>
#include <stdio.h> 
#include <SDL3/SDL_video.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Surface* eye_surface;
SDL_Texture* eye_texture;
SDL_FRect eye_position;
SDL_Surface* mouth_surface;
SDL_Texture* mouth_texture;
SDL_Event event;
SDL_Joystick* joystick;

int can_blink = true;
int blinktimer;
int total;
int i;

enum EyeExpression {
    NONE,
    HAPPY,
    SAD,
    ANGRY,
    SURPRISED
};

EyeExpression manual_expression = NONE;

void init_joystick() {
    SDL_Init(SDL_INIT_JOYSTICK);
    int joystick_count;
    SDL_JoystickID* joysticks = SDL_GetJoysticks(&joystick_count);
    if (joystick_count > 0) {
        joystick = SDL_OpenJoystick(joysticks[0]);
    }
}

void check_blinking() {
    total = SDL_GetNumJoystickButtons(joystick);

    if (SDL_GetJoystickButton(joystick, 0)) {
        manual_expression = HAPPY;
    }
    else if (SDL_GetJoystickButton(joystick, 1)) {
        manual_expression = SAD;
    }
    else if (SDL_GetJoystickButton(joystick, 2)) {
        manual_expression = ANGRY;
    }
    else if (SDL_GetJoystickButton(joystick, 3)) {
        manual_expression = SURPRISED;
    }
    else {
        manual_expression = NONE;
    }

    if (manual_expression == NONE) {
        can_blink = true;
        blinktimer++;
        if (blinktimer > 860) {
            blinktimer = 0;
        }
    }
    else {
        can_blink = false;
        blinktimer = 0;
    }
}


void check_rendering_eye_states() {
    check_blinking();

    // Determine which eye image to load based on blink timer
    const char* image_path = nullptr;

    if (can_blink) {
        if ((blinktimer >= 0 && blinktimer < 15) || (blinktimer >= 60)) {
            image_path = "./images/test_eyes.png";
        } else if ((blinktimer >= 15 && blinktimer < 30) || (blinktimer >= 45 && blinktimer <= 60)) {
            image_path = "./images/test_half.png";
        } else if (blinktimer > 30 && blinktimer < 45) {
            image_path = "./images/test_closed.png";
        }
    } 

    if (manual_expression != NONE) {
        const char* path = nullptr;

        switch (manual_expression) {
        case HAPPY:
            path = "./images/test_happy.png";
            break;
        case SAD:
            path = "./images/test_sad.png";
            break;
        case ANGRY:
            path = "./images/test_angry.png";
            break;
        case SURPRISED:
            path = "./images/test_surprised.png";
            break;
        default:
            break;
        }

        if (path) {
            SDL_Surface* expression_surface = IMG_Load(path);
            SDL_Texture* expression_texture = SDL_CreateTextureFromSurface(renderer, expression_surface);
            SDL_DestroySurface(expression_surface);
            if (eye_texture) SDL_DestroyTexture(eye_texture);
            eye_texture = expression_texture;
        }
    }

    if (image_path) {
        SDL_Surface* new_surface = IMG_Load(image_path);
        if (!new_surface) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image %s: %s", image_path, SDL_GetError());
            return;
        }

        SDL_Texture* new_texture = SDL_CreateTextureFromSurface(renderer, new_surface);
        SDL_DestroySurface(new_surface);
        if (!new_texture) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture from %s: %s", image_path, SDL_GetError());
            return;
        }

        // Replace the old eye texture
        if (eye_texture) {
            SDL_DestroyTexture(eye_texture);
        }
        eye_texture = new_texture;
    }
}

float get_axis(int index) {
    if (!joystick) return 0;
    SDL_PumpEvents();
    return SDL_GetJoystickAxis(joystick, index);
}

int controller_num_buttons() {
    if (!joystick) return 0;
    return SDL_GetNumJoystickButtons(joystick);
}

int main(int argc, char* argv[])
{   

    init_joystick();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    if (!SDL_CreateWindowAndRenderer("CRT STELLE 0.1.2", 1280, 640, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    eye_surface = IMG_Load("./images/test_eyes.png");
    if (!eye_surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        return 3;
    }
    
    eye_texture = SDL_CreateTextureFromSurface(renderer, eye_surface);
    if (!eye_texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
        return 3;
    }
    SDL_DestroySurface(eye_surface);

    mouth_surface = IMG_Load("./images/test_mouth.png");
    if (!mouth_surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        return 3;
    }
    mouth_texture = SDL_CreateTextureFromSurface(renderer, mouth_surface);
    if (!mouth_texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
        return 3;
    }
    SDL_DestroySurface(mouth_surface);
    SDL_SetRenderLogicalPresentation(renderer, 1280, 640, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }
        eye_position.w = 1280;
        eye_position.h = 640;
        

        check_rendering_eye_states();

     //   if (joystick) {
            eye_position.x = get_axis(0) / 32767.0f * 25;

            eye_position.y = get_axis(1) / 32767.0f * 25;
    //    }

        printf("BlinkTimer: %d\n", blinktimer);

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, eye_texture, NULL, &eye_position);
        SDL_RenderTexture(renderer, mouth_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(eye_texture);
    SDL_DestroyTexture(mouth_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}