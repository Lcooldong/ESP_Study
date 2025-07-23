#include <iostream>
#include <SDL3/SDL.h>

//#pragma comment(lib, "../x64/Debug/lib/x64/SDL3.lib")

int main()
{
    std::cout << "HID"<< std::endl;

    if (SDL_Init(SDL_INIT_GAMEPAD) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    int num_joysticks = SDL_HasGamepad();
    std::cout << "Joysticks/Gamepads detected: " << num_joysticks << std::endl;

    SDL_Gamepad* gamepad = nullptr;

    for (int i = 0; i < num_joysticks; ++i) {
        if (SDL_IsGamepad(i)) {
            gamepad = SDL_OpenGamepad(i);
            if (gamepad) {
                std::cout << "Opened Gamepad: " << SDL_GetGamepads(&num_joysticks) << std::endl;
                break;
            }
        }
    }

    if (!gamepad) {
        std::cerr << "No gamepad could be opened.\n";
        SDL_Quit();
        return 1;
    }

    SDL_Event event;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                std::cout << "[BUTTON DOWN] Button: " << SDL_GetGamepadStringForButton(
                    static_cast<SDL_GamepadButton>(event.gbutton.button)) << std::endl;
                break;
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                std::cout << "[AXIS] Axis: " << SDL_GetGamepadStringForAxis(
                    static_cast<SDL_GamepadAxis>(event.gaxis.axis)) << " ¡æ " << event.gaxis.value << std::endl;
                break;
            }
        }

        SDL_Delay(10);
    }

    SDL_CloseGamepad(gamepad);
    SDL_Quit();

    return 0;

  
}