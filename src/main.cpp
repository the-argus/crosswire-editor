#include <SDL.h>
#include <vector>
#include <fstream>

#include <imgui.h>
#include "../inc/imgui_impl_sdl2.h"
#include "../inc/imgui_impl_sdlrenderer2.h"
#include "Inputs.h"
#include "Polygons.h"
#include "util.h"
#include "Room.h"


Inputs getInputs() {
    static bool mouseHeld = false;
    Inputs i = {0, 0, 0};
    SDL_GetMouseState(&i.mouseX, &i.mouseY);
    i.DragPoint = mouseHeld;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_n:     i.NewPolygon = 1;    break;
                case SDLK_d:     i.DeletePoint = 1;   break;
                case SDLK_DELETE:i.DeletePolygon = 1; break;
                case SDLK_SPACE: i.AddPoint = 1;      break;
                case SDLK_s:     i.SaveToFile = 1;    break;
                case SDLK_RIGHT: i.IncrementSelection = 1; break;
                case SDLK_LEFT:  i.DecrementSelection = 1; break;
            }
        } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            mouseHeld = true;
            i.SelectPoint = 1;
        } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            mouseHeld = false;
        }
    }
    return i;
}

Room level;

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// Main code
int main(int argc, char* argv[]){
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    //Set Style Dark
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done){

        ////////////////////////
        ///// Update Logic /////
        Inputs i = getInputs();
        level.updateRoom(i);
        

        //Start Dear IMGUI Frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();


        //Draw Side Menu
        ImGui::Begin("Polygon Selector", nullptr, ImGuiWindowFlags_NoMove);
        ImGui::SetWindowPos(ImVec2(0, 0)); // Set position to the side of the screen
        ImGui::SetWindowSize(ImVec2(300, 400)); // Set the size of the menu

        //Create a scrollable Region
        ImGui::BeginChild("Polygon Selector", ImGui::GetContentRegionAvail(), true);

        //Create list
        for (int i = 0; i < level.getNumberOfPolygons(); i++) {
            if (ImGui::Selectable(("Polygon " + std::to_string(i)).c_str())) {
                //Update Selected Polygon
                level.selectPolygon(i);
            }
        }

        //End the scrollable region
        ImGui::EndChild();
        ImGui::End();

        //Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

        level.drawRoom(renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}