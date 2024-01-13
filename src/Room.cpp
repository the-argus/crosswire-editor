#include "Room.h"

Room::Room() {
  setCurrentTool(EditingTool::Polygons);
  currentPolygon = -1;
}

void Room::setCurrentTool(EditingTool tool) {
  currentTool = tool;
  switch (tool) {
  case EditingTool::Polygons:
    updateFunc = [this](Inputs i) { updateRoomPolygonTool(i); };
    return;
  case EditingTool::Images:
    updateFunc = [this](Inputs i) { updateRoomImageTool(i); };
    return;
  case EditingTool::Turrets:
    updateFunc = [this](Inputs i) { updateRoomTurretTool(i); };
    return;
  case EditingTool::BuildSites:
    updateFunc = [this](Inputs i) { updateRoomBuildSiteTool(i); };
    return;
  }
  std::abort();
}

void Room::updateRoom(Inputs i) {
  if (updateFunc)
    updateFunc.value()(i);
}

void Room::updateRoomPolygonTool(Inputs i) {
  // Add New Polygon
  if (i.New) {
    Areas.push_back(Polygon(i.mouseX, i.mouseY));
    terrain_types.push_back(terrain_type);
    currentPolygon = Areas.size() - 1;
  }
  // Delete current Polygon
  if (i.Delete) {
    if (currentPolygon && Areas.size() != 0) {
      if (currentPolygon.value() >= Areas.size()) {
        currentPolygon = {};
        return;
      }
      Areas.erase(Areas.begin() + currentPolygon.value());
      terrain_types.erase(terrain_types.begin() + currentPolygon.value());
    }
  }

  // change current polygon with keyboard
  if (!Areas.empty()) {
    if (i.IncrementSelection) {
      if (!currentPolygon)
        currentPolygon = 0;
      currentPolygon = (currentPolygon.value() + 1) % Areas.size();
    }

    if (i.DecrementSelection) {
      if (!currentPolygon)
        currentPolygon = 0;
      currentPolygon =
          (currentPolygon.value() - 1 + Areas.size()) % Areas.size();
    }
    // Update Polygon
    if (currentPolygon) {
      if (currentPolygon.value() >= Areas.size()) {
        currentPolygon = {};
        return;
      }
      Areas[currentPolygon.value()].updatePolygon(i);
    }
  }
}
void Room::updateRoomBuildSiteTool(Inputs i) {
  if (i.New) {
    buildSites.push_back(cw::BuildSite{
        .position_a =
            {
                .x = (float)i.mouseX,
                .y = (float)i.mouseY,
            },
        .position_b = {},
    });
  } else if (i.Delete) {
    if (currentBuildSite) {
      if (currentBuildSite.value() >= buildSites.size()) {
        currentBuildSite = {};
        return;
      }
      assert(buildSites.size() != 0);
      buildSites.erase(buildSites.begin() + currentBuildSite.value());
    }
  }
}
void Room::updateRoomTurretTool(Inputs i) {
  if (i.New) {
    turrets.push_back(cw::Turret{
        .position =
            {
                .x = (float)i.mouseX,
                .y = (float)i.mouseY,
            },
        .direction = {0, 1},
        .fireRateSeconds = turret_fire_rate,
        .pattern = turret_pattern,
    });
  } else if (i.Delete) {
    if (currentTurret) {
      if (currentTurret) {
        if (currentTurret.value() >= turrets.size())
          currentTurret = {};
        assert(buildSites.size() != 0);
        buildSites.erase(buildSites.begin() + currentTurret.value());
      }
    }
  }
}
void Room::updateRoomImageTool(Inputs i) {}

cw::SerializeResultCode Room::trySerialize(const char *levelname,
                                           bool overwrite) const {
  return cw::SerializeResultCode::FileExists;
}

std::string Room::getDisplayNameAtIndex(size_t index) const {
  if (index >= Areas.size())
    return "";

  switch (terrain_types[index]) {
  case cw::TerrainType::Ditch:
    return "Ditch Polygon " + std::to_string(index);
    break;
  case cw::TerrainType::Obstacle:
    return "Obstacle Polygon " + std::to_string(index);
    break;
  }
  return "Unknown Polygon " + std::to_string(index);
}

void Room::drawRoom(SDL_Renderer *renderer) {
  const uint8_t BASE_RED = 255, BASE_GREEN = 255, BASE_BLUE = 255,
                SELECT_RED = 32, SELECT_GREEN = 255, SELECT_BLUE = 64,
                BASE_DITCH_RED = 255, BASE_DITCH_GREEN = 128,
                BASE_DITCH_BLUE = 128;

  {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (const auto &site : buildSites) {
      SDL_Rect rect{
          .x = (int)site.position_a.x,
          .y = (int)site.position_a.y,
          .w = 5,
          .h = 5,
      };

      SDL_RenderFillRect(renderer, &rect);
      rect.x = site.position_b.x;
      rect.y = site.position_b.y;
      SDL_RenderFillRect(renderer, &rect);
    }
  }

  // draw terrain
  assert(Areas.size() == terrain_types.size());
  for (int i = 0; i < Areas.size(); i++) {
    if (i == currentPolygon) {
      Areas[i].drawPolygon(renderer, SELECT_RED, SELECT_GREEN, SELECT_BLUE);
    } else {
      switch (terrain_types[i]) {
      case cw::TerrainType::Ditch:
        Areas[i].drawPolygon(renderer, BASE_DITCH_RED, BASE_DITCH_BLUE,
                             BASE_DITCH_GREEN);
        break;
      case cw::TerrainType::Obstacle:
        Areas[i].drawPolygon(renderer, BASE_RED, BASE_GREEN, BASE_BLUE);
        break;
      }
    }
  }
}
