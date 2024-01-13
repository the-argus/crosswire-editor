#pragma once
#include "Inputs.h"
#include "Polygons.h"
#include "serialize.h"
#include <optional>
#include <vector>
#include <functional>

enum class EditingTool {
  Polygons,
  Images,
  Turrets,
  BuildSites,
};

/**
 * @brief Represents a room/level with polygons representing areas
 */
struct Room {
private:
  std::vector<Polygon> Areas;
  std::optional<size_t> currentPolygon;

  // these two should always be the same length
  std::vector<cw::Image> serializableImageData;
  std::vector<SDL_Texture *> runtimeImageData;

  std::vector<cw::Turret> turrets;

  void updateRoomPolygonTool(Inputs i);
  void updateRoomBuildSiteTool(Inputs i);
  void updateRoomTurretTool(Inputs i);
  void updateRoomImageTool(Inputs i);

  std::optional<std::function<void(Inputs)>> updateFunc;
  EditingTool currentTool = EditingTool::Polygons;

public:
  void setCurrentTool(EditingTool tool);

  // Constructor
  Room();
  // Update the current room based on user input and selected polygon
  void updateRoom(Inputs i);

  // Render all polygons of the current room onto the screen
  void drawRoom(SDL_Renderer *renderer);

  // Get the number of polygons in the room
  inline constexpr size_t getNumberOfPolygons() { return Areas.size(); }

  // Select a polygon of the ones currently in the room. If the polygon is out
  // of range, does nothing.
  inline constexpr void selectPolygon(size_t i) {
    if (i < getNumberOfPolygons()) {
      currentPolygon = i;
    }
  }
};
