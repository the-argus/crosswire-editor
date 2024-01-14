#pragma once
#include "ImageSelector.h"
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
  std::optional<size_t> currentImage;
  std::optional<SDL_Texture *> selectedImage;
  std::optional<const char *> selectedImageFilename;

  // these two should always be the same length
  std::vector<cw::Image> serializableImageData;
  std::vector<SDL_Texture *> runtimeImageData;

  std::vector<cw::Turret> turrets;

  std::vector<cw::BuildSite> buildSites;
  Vec2 player_spawn = {100, 100};

  struct SelectedBuildSiteInfo {
    size_t index;
    bool is_a;
  };
  std::optional<SelectedBuildSiteInfo> buildSiteSelection;

  void updateRoomPolygonTool(Inputs i);
  void updateRoomBuildSiteTool(Inputs i);
  void updateRoomTurretTool(Inputs i);
  void updateRoomImageTool(Inputs i);
  void createImageAt(const char *filename, SDL_Texture *tex, float x, float y);

  std::optional<std::function<void(Inputs)>> updateFunc;
  EditingTool currentTool = EditingTool::Polygons;

  cw::TerrainType terrain_type = cw::TerrainType::Ditch;
  cw::TurretPattern turret_pattern = cw::TurretPattern::Circle;
  float turret_fire_rate = 0.5f;

  std::vector<std::string> filenamesLoadedFromFile;

public:
  void setCurrentTool(EditingTool tool);
  std::string getDisplayNameAtIndex(size_t index) const;
  inline constexpr void setTerrainType(cw::TerrainType type) {
    terrain_type = type;
  }
  inline constexpr void setTurretPattern(cw::TurretPattern pattern) {
    turret_pattern = pattern;
  }

  // TODO: getter and setter? cringe
  inline constexpr void setTurretFireRate(float rate) {
    turret_fire_rate = rate;
  }
  inline constexpr float getTurretFireRate() { return turret_fire_rate; }

  inline constexpr void setCurrentImage(size_t index) {
    if (index >= runtimeImageData.size())
      return;
    currentImage = index;
  }

  inline constexpr void setCurrentBuildSite(size_t index) {
    if (index >= buildSites.size()) {
      return;
    }
    currentBuildSite = index;
  }

  inline constexpr size_t getNumBuildSites() const { return buildSites.size(); }

  // change the characteristics of the next image placed
  inline constexpr void changeImagePlacementOptions(const char *filename,
                                                    SDL_Texture *tex) {
    selectedImage = tex;
    selectedImageFilename = filename;
  }

  inline constexpr const std::vector<cw::Image> &getImages() const {
    return serializableImageData;
  }

  inline constexpr const std::vector<cw::Turret> &getTurrets() const {
    return turrets;
  }

  inline constexpr cw::Turret &getTurret(size_t index) {
    assert(index <= turrets.size());
    return turrets[index];
  }

  // TODO: naming convention on this is inconsistent, should be setCurrentTurret
  inline constexpr void selectTurret(size_t index) {
    if (index >= turrets.size())
      return;
    currentTurret = index;
  }

  inline constexpr void setTerrainTypeFor(size_t index, cw::TerrainType type) {
    if (index >= terrain_types.size()) {
      return;
    }
    terrain_types[index] = type;
  }
  inline constexpr cw::TerrainType getTerrainTypeFor(size_t index) {
    assert(index < terrain_types.size());
    return terrain_types[index];
  }

  cw::SerializeResultCode trySerialize(const char *levelname,
                                       bool overwrite) const;

  cw::DeserializeResultCode tryDeserialize(const char *levelname,
                                           const ImageSelector &image_selector);

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
