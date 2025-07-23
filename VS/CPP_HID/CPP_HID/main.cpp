#include <iostream>
#include <stdexcept>
#include <SDL3/SDL.h>



//#pragma comment(lib, "../x64/Debug/lib/x64/SDL3.lib")

class SDLExeption final : public std::runtime_error
{
public:
    explicit SDLExeption(const std::string& message) : std::runtime_error(message + ": " + SDL_GetError()) {}
 
};


int main()
{
    std::cout << "HID"<< std::endl;
	SDL_Window* window = NULL;

    if (SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
        std::cout << "SDL could not be initialized: " << SDL_GetError(); 
    }
    else 
    {
        std::cout << "SDL video system and joystick is ready to go\r\n ";
    }

	window = SDL_CreateWindow("SDL Gamepad Example", 640, 480, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (!window)
        throw SDLExeption("Couldn't create window");

    SDL_GPUDevice* gpuDevice{

        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_DXIL, true, nullptr)
    };

    if (!gpuDevice) 
		throw SDLExeption("Couldn't create GPU device");


    if (!SDL_ClaimWindowForGPUDevice(gpuDevice, window)) 
		throw SDLExeption("Couldn't claim window for GPU device");

    SDL_ShowWindow(window);

    bool isRunning{ true };
	SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
			    //break;

            default:
                break;
            }
        }
    }

	SDL_Renderer* renderer = SDL_CreateRenderer(window, "NAME");

    SDL_Rect rect;
	rect.x = 50;
	rect.y = 50;
	rect.w = 200;
	rect.h = 200;


	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer); // Clear the renderer with the current draw color\

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	 
	
    
	SDL_RenderPresent(renderer);

  

    
    SDL_Quit();

    return 0;

  
}