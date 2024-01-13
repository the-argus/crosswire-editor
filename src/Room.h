#pragma once
#include "Inputs.h"
#include "Polygons.h"
#include "serialize.h"
#include <functional>
#include <optional>
#include <vector>

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
  std::vector<cw::TerrainType> terrain_types;
  std::optional<size_t> currentPolygon;
  std::optional<size_t> currentBuildSite;
  std::optional<size_t> currentTurret;

  // these two should always be the same length
  std::vector<cw::Image> serializableImageData;
  std::vector<SDL_Texture *> runtimeImageData;

  std::vector<cw::Turret> turrets;

  std::vector<cw::BuildSite> buildSites;

  void updateRoomPolygonTool(Inputs i);
  void updateRoomBuildSiteTool(Inputs i);
  void updateRoomTurretTool(Inputs i);
  void updateRoomImageTool(Inputs i);

  std::optional<std::function<void(Inputs)>> updateFunc;
  EditingTool currentTool = EditingTool::Polygons;

  cw::TerrainType terrain_type = cw::TerrainType::Ditch;
  cw::TurretPattern turret_pattern = cw::TurretPattern::Circle;
  float turret_fire_rate = 0.5f;

public:
  void setCurrentTool(EditingTool tool);
  std::string getDisplayNameAtIndex(size_t index) const;
  inline constexpr void setTerrainType(cw::TerrainType type) {
    terrain_type = type;
  }
  inline constexpr void setTurretPattern(cw::TurretPattern pattern) {
    turret_pattern = pattern;
  }

  inline constexpr void setTurretFireRate(float rate) {
    turret_fire_rate = rate;
  }

  inline constexpr const std::vector<cw::Turret> &getTurrets() const {
    return turrets;
  }

  inline constexpr cw::Turret &getTurret(size_t index) {
    assert(index <= turrets.size());
    return turrets[index];
  }

  inline constexpr void selectTurret(size_t index) {
    if (index >= turrets.size())
      return;
    currentTurret = index;
  }

  cw::SerializeResultCode trySerialize(const char *levelname,
                                       bool overwrite) const;

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
