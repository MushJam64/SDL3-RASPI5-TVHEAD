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
    if (SDL_GetJoystickButton(joystick, i)) {
        can_blink = false;
        blinktimer = 0;
    }
    else {
        can_blink = true;
        blinktimer++;
    }
}

void check_rendering_eye_states() {
    check_blinking();
        if (can_blink) {
            if (blinktimer >= 0 && blinktimer < 60 || blinktimer >= 240){
            SDL_Surface* openeyes = IMG_Load("./images/test_eyes.png");
            SDL_Texture* openeyes_texture = SDL_CreateTextureFromSurface(renderer, openeyes);
            SDL_DestroySurface(openeyes);
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, openeyes_texture, NULL, &eye_position);
            SDL_RenderTexture(renderer, mouth_texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_DestroyTexture(openeyes_texture);
            }
            if (blinktimer >= 60 && blinktimer < 120|| blinktimer >= 180 && blinktimer <= 240) {  
                SDL_Surface* halfeyes = IMG_Load("./images/test_half.png");
                SDL_Texture* halfeyes_texture = SDL_CreateTextureFromSurface(renderer, halfeyes);
                SDL_DestroySurface(halfeyes);
                SDL_RenderClear(renderer);
                SDL_RenderTexture(renderer, halfeyes_texture, NULL, &eye_position);
                SDL_RenderTexture(renderer, mouth_texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_DestroyTexture(halfeyes_texture);    
            } 
            if (blinktimer > 120 && blinktimer < 180) {
                SDL_Surface* closedeyes = IMG_Load("./images/test_closed.png");
                SDL_Texture* closedeyes_texture = SDL_CreateTextureFromSurface(renderer, closedeyes);
                SDL_DestroySurface(closedeyes);
                SDL_RenderClear(renderer);
                SDL_RenderTexture(renderer, closedeyes_texture, NULL, &eye_position);
                SDL_RenderTexture(renderer, mouth_texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_DestroyTexture(closedeyes_texture); 
            } 
            else if (blinktimer == 860) {
                blinktimer = 0;
            }
        }
            else {
            SDL_Surface* happyeyes = IMG_Load("./images/test_happy.png");
            SDL_Texture* happyeyes_texture = SDL_CreateTextureFromSurface(renderer, happyeyes);
            SDL_DestroySurface(happyeyes);
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, happyeyes_texture, NULL, &eye_position);
            SDL_RenderTexture(renderer, mouth_texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_DestroyTexture(happyeyes_texture);
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

    if (!SDL_CreateWindowAndRenderer("Hello SDL", 1280, 640, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
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