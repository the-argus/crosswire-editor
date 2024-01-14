#include <SDL2/SDL_image.h>
#include <iostream>

#ifdef ZIGBUILD
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#else
#include <SDL.h>
#include <imgui.h>
#include "../inc/imgui_impl_sdl2.h"
#include "../inc/imgui_impl_sdlrenderer2.h"
#endif
#include <vector>
#include <fstream>

#include "Inputs.h"
#include "Room.h"
#include "ImageSelector.h"
#include <optional>


Inputs getInputs(bool& done) {
    static bool mouseHeld = false;
    Inputs i = {0, 0, 0};
    SDL_GetMouseState(&i.mouseX, &i.mouseY);
    i.DragPoint = mouseHeld;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                SDL_Quit();
                done = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_n:     i.New = 1;    break;
                    case SDLK_d:     i.DeletePoint = 1;   break;
                    case SDLK_DELETE:i.Delete = 1; break;
                    case SDLK_SPACE: i.AddPoint = 1;      break;
                    case SDLK_s:     i.SaveToFile = 1;    break;
                    case SDLK_RIGHT: i.IncrementSelection = 1; break;
                    case SDLK_LEFT:  i.DecrementSelection = 1; break;
                    case SDLK_ESCAPE:i.Cancel = 1; break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseHeld = true;
                    i.Select = 1;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseHeld = false;
                }
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        SDL_Quit();
                        done = true;
                        break;
                }
                break;
            default:
                break;
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

    if (!IMG_Init(IMG_INIT_PNG)) {
        printf("Unable to initialize SDL image.\n");
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
    // TODO: image selector should be destroyed before SDL_Quit so that the textures get freed at the right time
    ImageSelector selector(renderer, "assets");
    std::vector<std::string> image_names = selector.get_image_names();
    const char* turret_tracking_types[] {"Circle", "Tracking", "Straight Line"};
    int selected_turret_tracking_type = 0;
    bool select_induvidual_vertices = true;
    bool window_active = false;

    const char* terrain_types[] {"Ditch", "Obstacle"};
    int selected_terrain_type = 0;

    std::optional<cw::SerializeResultCode> lasterr = {};
    std::optional<cw::DeserializeResultCode> lastdeserializeerr = {};

    bool overwrite_files = false;

    size_t selected_turret = 0;

    // Main loop
    bool done = false;
    while (!done){

        ////////////////////////
        ///// Update Logic /////
        Inputs i = getInputs(done);
        if (!window_active) {
            level.updateRoom(i);
        }
        

        //Start Dear IMGUI Frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();


        //Draw Side Menu
        {
            ImGui::Begin("Tools");
            window_active = ImGui::IsWindowFocused();
            // ImGui::SetWindowPos(ImVec2(0, 0)); // Set position to the side of the screen
            // ImGui::SetWindowSize(ImVec2(300, 400)); // Set the size of the menu
            // ImGui::BeginChild("Polygon Selection", ImGui::GetContentRegionAvail(), true);


            if (ImGui::BeginTabBar("Level Editing Tools")) {

                if (ImGui::BeginTabItem("Polygon Selection")) {
                    level.setCurrentTool(EditingTool::Polygons);

                    ImGui::SeparatorText("Options");
                    ImGui::Checkbox("Move Induvidual Vertices (does nothing rn)", &select_induvidual_vertices);

                    auto terrain_type_combo_box = [&](const char* label, int original_index) -> std::optional<int> {
                        int terrain_type = original_index;
                        bool changed = ImGui::Combo(label,
                            &terrain_type,
                            terrain_types,
                            IM_ARRAYSIZE(terrain_types)
                        );
                        if (changed) {
                            return terrain_type;
                        }
                        return {};
                    };

                    if (auto res = terrain_type_combo_box("Next Terrain Type", selected_terrain_type)) {
                        level.setTerrainType(cw::TerrainType(res.value()));
                        selected_terrain_type = res.value();
                    }

                    static std::optional<size_t> selected_polygon = {};
                    ImGui::SeparatorText("Existing Polygons");
                    for (int i = 0; i < level.getNumberOfPolygons(); i++) {
                        if (ImGui::Selectable(level.getDisplayNameAtIndex(i).c_str())) {
                            //Update Selected Polygon
                            level.selectPolygon(i);
                            selected_polygon = i;
                        }
                        if (selected_polygon == i) {
                            if (auto res = terrain_type_combo_box("Terrain Type", (int)level.getTerrainTypeFor(i))) {
                                level.setTerrainTypeFor(i, cw::TerrainType(res.value()));
                            }
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Image Palette")) {
                    level.setCurrentTool(EditingTool::Images);
                    size_t index = 0;
                    ImGui::SeparatorText("Available Images");
                    for (auto& name : image_names) {
                        if (ImGui::Selectable(name.c_str())) {
                            level.changeImagePlacementOptions(selector.get_filename(index), selector.get(index));
                        }
                        ++index;
                    }
                    
                    ImGui::SeparatorText("Existing Images");

                    index = 0;
                    for (const auto& image : level.getImages()) {
                        if (ImGui::Selectable(image.filename.data())) {
                            level.setCurrentImage(index);
                        }
                        ++index;
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Turret Tool")) {
                    level.setCurrentTool(EditingTool::Turrets);
                    ImGui::BeginChild("Turret Characteristics", ImGui::GetContentRegionAvail(), true);
                    ImGui::SeparatorText("Turret Characteristics");

                    // turret response
                    using tres = std::pair<cw::TurretPattern, int>;
                    auto tracking_type_combo_box = [&](const char* name, int initial)-> std::optional<std::pair<cw::TurretPattern, int>>{
                        int index = initial;
                        bool changed = ImGui::Combo(name,
                            &index,
                            turret_tracking_types,
                            IM_ARRAYSIZE(turret_tracking_types)
                        );
                        if (changed) {
                            switch (index) {
                                case 0:
                                    return tres{cw::TurretPattern::Circle, index};
                                    break;
                                case 1:
                                    return tres{cw::TurretPattern::Tracking, index};
                                    break;
                                case 2:
                                    return tres{cw::TurretPattern::StraightLine, index};
                                    break;
                            }
                        }
                        return {};
                    };

                    if (auto tracking = tracking_type_combo_box("Next Turret Tracking Type", selected_turret_tracking_type)) {
                        level.setTurretPattern(tracking.value().first);
                        selected_turret_tracking_type = tracking.value().second;
                    }

                    {
                        float rate = level.getTurretFireRate();
                        if (ImGui::SliderFloat("Fire Rate in Seconds", &rate, 0.01f, 10.0f)) {
                            level.setTurretFireRate(rate);
                        }
                    }

                    ImGui::SeparatorText("Existing Turrets");

                    size_t index = 0;
                    for (const auto& turret : level.getTurrets()) {
                        auto name = "Turret " + std::to_string(index);
                        if (ImGui::Selectable(name.c_str())) {
                            level.selectTurret(index);
                            selected_turret = index;
                        }
                        if (index == selected_turret) {
                            auto& turret = level.getTurret(index);
                            float direction = atan2(turret.direction.y, turret.direction.x);
                            ImGui::SliderFloat("Fire Rate", &turret.fireRateSeconds, 0.01f, 10.0f);
                            if (ImGui::SliderFloat("Direction", &direction, 0.01f, 10.0f)) {
                                turret.direction.x = cos(direction);
                                turret.direction.y = sin(direction);
                            }
                            int tracking_index =
                                (turret.pattern == cw::TurretPattern::Circle) ? (0)
                                : (turret.pattern == cw::TurretPattern::Tracking ? (1)
                                : (2));
                            if (auto tracking = tracking_type_combo_box("Tracking Type", tracking_index)) {
                                turret.pattern = tracking.value().first;
                            }
                        }
                        ++index;
                    }

                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Build Site Tool")) {
                    level.setCurrentTool(EditingTool::BuildSites);
                    ImGui::SeparatorText("Existing Build Sites");
                    for (size_t i = 0; i < level.getNumBuildSites(); ++i) {
                        if (ImGui::Selectable(("Build Site " + std::to_string(i)).c_str())) {
                            level.setCurrentBuildSite(i);
                        }
                    }
                    ImGui::EndTabItem();
                }

                //End the scrollable region
                // ImGui::EndChild();
                ImGui::EndTabBar();
            }

            ImGui::SeparatorText("Level Save Dialog");

            {
                static std::array<char, 1024> buf = {0};
                ImGui::InputText("Level Name", buf.data(), buf.size());
                buf[1023] = 0; // always null terminated, idk if imgui does this

                ImGui::Checkbox("Overwrite files when saving?", &overwrite_files);

                if (ImGui::Button("Save")) {
                    if (std::strlen(buf.data()) != 0) {
                        lasterr = level.trySerialize(buf.data(), overwrite_files);
                    }
                }
            }

            if (lasterr) {
                switch (lasterr.value()) {
                    case cw::SerializeResultCode::Okay:
                        ImGui::Text("Saved file successfully.");
                        break;
                    case cw::SerializeResultCode::FileExists:
                        ImGui::Text("File already exists.");
                        break;
                    case cw::SerializeResultCode::TryAgain:
                        ImGui::Text("Temporary failure, try again.");
                        break;
                    case cw::SerializeResultCode::AlreadyOpen:
                        ImGui::Text("Device or resource busy (file already open?)");
                        break;
                    case cw::SerializeResultCode::PathTooLong:
                        ImGui::Text("Given path is too long.");
                        break;
                    default:
                        ImGui::Text("Unknown file save error.");
                        break;
                }
            }

            ImGui::SeparatorText("Load level");

            {
                static std::array<char, 1024> load_buf = {0};
                ImGui::InputText("Level", load_buf.data(), load_buf.size());
                load_buf[1023] = 0; // always null terminated, idk if imgui does this

                ImGui::Text("Warning: loading overwrites all current editor data.");
                if (ImGui::Button("Load") && strlen(load_buf.data()) != 0) {
                    lastdeserializeerr = level.tryDeserialize(load_buf.data(), selector);
                }
            }

            if (lastdeserializeerr) {
                switch (lastdeserializeerr.value()) {
                    case cw::DeserializeResultCode::Okay:
                        ImGui::Text("Loaded file successfully.");
                        break;
                    case cw::DeserializeResultCode::EarlyEOF:
                    case cw::DeserializeResultCode::InvalidHeader:
                        ImGui::Text("File parsing error, corruption or old version?");
                        break;
                    case cw::DeserializeResultCode::TryAgain:
                        ImGui::Text("Temporary failure, try again.");
                        break;
                    case cw::DeserializeResultCode::AlreadyOpen:
                        ImGui::Text("Device or resource busy (file already open?)");
                        break;
                    case cw::DeserializeResultCode::NoSuchImageFile:
                        ImGui::Text("The file contains references to image files which cannot be found in the assets folder.");
                        break;
                    default:
                        ImGui::Text("Unknown file save error.");
                        break;
                }
            }

            ImGui::End();
        }

        //Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        level.drawRoom(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
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
