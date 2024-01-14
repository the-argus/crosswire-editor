#include "Room.h"
#include <cmath>

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
  if (i.SetPlayerSpawn) {
    player_spawn = {.x = (float)i.mouseX, .y = (float)i.mouseY};
  }
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

void Room::createImageAt(const char *filename, SDL_Texture *tex, float x,
                         float y) {
  runtimeImageData.push_back(tex);
  serializableImageData.push_back(cw::Image{
      .filename = std::span(filename, strlen(filename)),
      .data =
          cw::ImageData{
              .position =
                  {
                      .x = x,
                      .y = y,
                  },
              .rotation = 0.0f,
          },
  });
}

void Room::updateRoomBuildSiteTool(Inputs i) {
  if (i.New) {
    buildSites.push_back(cw::BuildSite{
        .position_a =
            {
                .x = (float)i.mouseX,
                .y = (float)i.mouseY,
            },
        .position_b =
            {
                .x = (float)i.mouseX,
                .y = (float)i.mouseY + 10,
            },
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

  if (i.DragPoint && buildSites.size() != 0) {
    if (!buildSiteSelection) {
      size_t nearest_index = 0;
      float best_distance = INFINITY;
      bool is_a;

      size_t index = 0;
      for (auto &site : buildSites) {
        auto check = [i, &best_distance, &is_a, index,
                      &nearest_index](Vec2 point, bool this_is_a) {
          float w = abs(point.x - i.mouseX);
          float h = abs(point.y - i.mouseY);
          float dist = sqrt((w * w) + (h * h));

          if (dist < best_distance) {
            best_distance = dist;
            is_a = this_is_a;
            nearest_index = index;
          }
        };

        check(site.position_a, true);
        check(site.position_b, false);

        index++;
      }

      if (best_distance > 30)
        return;

      buildSiteSelection = {.index = nearest_index, .is_a = is_a};
    }

    Vec2 &point = buildSiteSelection->is_a
                      ? buildSites[buildSiteSelection->index].position_a
                      : buildSites[buildSiteSelection->index].position_b;

    point.x = i.mouseX;
    point.y = i.mouseY;
  } else {
    buildSiteSelection = {};
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
      if (currentTurret.value() >= turrets.size())
        currentTurret = {};
      assert(turrets.size() != 0);
      turrets.erase(turrets.begin() + currentTurret.value());
    }
  }
}

void Room::updateRoomImageTool(Inputs i) {
  if (i.New && selectedImageFilename && selectedImage) {
    createImageAt(selectedImageFilename.value(), selectedImage.value(),
                  i.mouseX, i.mouseY);
  } else if (i.Delete) {
    if (currentImage) {
      if (currentImage.value() >= runtimeImageData.size()) {
        currentImage = {};
        return;
      }
      assert(runtimeImageData.size() != 0);
      runtimeImageData.erase(runtimeImageData.begin() + currentImage.value());
      serializableImageData.erase(serializableImageData.begin() +
                                  currentImage.value());
    }
  }
}

cw::SerializeResultCode Room::trySerialize(const char *levelname,
                                           bool overwrite) const {
  std::vector<cw::TerrainEntry> terrains;
  size_t index = 0;
  for (const auto &type : terrain_types) {
    terrains.push_back(cw::TerrainEntry{
        .verts = Areas[index].getPoints(),
        .type = type,
    });
    index++;
  }
  cw::Level level{
      .player_spawn = {player_spawn},
      .terrains = terrains,
      .images = serializableImageData,
      .build_sites = buildSites,
      .turrets = turrets,
  };

  return cw::serialize("levels", levelname, overwrite, level);
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
    size_t index = 0;
    for (const auto &site : buildSites) {
      if (index == currentBuildSite) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      }
      SDL_Rect rect{
          .x = (int)site.position_a.x - 2,
          .y = (int)site.position_a.y - 2,
          .w = 5,
          .h = 5,
      };

      SDL_RenderFillRect(renderer, &rect);
      rect.x = site.position_b.x;
      rect.y = site.position_b.y;
      SDL_RenderFillRect(renderer, &rect);

      if (index == currentBuildSite) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      }
      SDL_RenderDrawLine(renderer, site.position_a.x, site.position_a.y,
                         site.position_b.x, site.position_b.y);
      ++index;
    }
  }

  // draw turrets
  {
    size_t index = 0;
    for (const auto &turret : turrets) {
      if (index == currentTurret) {
        SDL_SetRenderDrawColor(renderer, 70, 255, 40, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      }
      int size = index == currentTurret ? 20 : 10;
      const float dirlength = 40.0f;
      SDL_Rect rect{
          .x = (int)turret.position.x - (size / 2),
          .y = (int)turret.position.y - (size / 2),
          .w = size,
          .h = size,
      };

      SDL_RenderFillRect(renderer, &rect);

      // show direction
      Vec2 visualdirection{
          .x = turret.direction.x * dirlength,
          .y = turret.direction.y * dirlength,
      };
      SDL_RenderDrawLine(renderer, turret.position.x, turret.position.y,
                         turret.position.x + visualdirection.x,
                         turret.position.y + visualdirection.y);
      ++index;
    }
  }

  // draw images
  {
    size_t index = 0;
    for (auto &tex : runtimeImageData) {
      SDL_Rect dest{
          .x = (int)serializableImageData[index].data.position.x,
          .y = (int)serializableImageData[index].data.position.y,
          // TODO: store actual width and height of image
          .w = 100,
          .h = 100,
      };
      SDL_RenderCopy(renderer, tex, nullptr, &dest);

      if (currentImage == index) {
        dest.x -= 10;
        dest.y -= 10;
        dest.w += 20;
        dest.h += 20;
        SDL_RenderDrawRect(renderer, &dest);
      }
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