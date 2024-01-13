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
  if (i.NewPolygon) {
    Areas.push_back(Polygon(i.mouseX, i.mouseY));
    currentPolygon = Areas.size() - 1;
  }
  // Delete current Polygon
  if (i.DeletePolygon) {
    if (currentPolygon && Areas.size() != 0) {
      Areas.erase(Areas.begin() + currentPolygon.value());
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
      Areas[currentPolygon.value()].updatePolygon(i);
    }
  }
}
void Room::updateRoomBuildSiteTool(Inputs i) {}
void Room::updateRoomTurretTool(Inputs i) {}
void Room::updateRoomImageTool(Inputs i) {}

void Room::drawRoom(SDL_Renderer *renderer) {
  const uint8_t BASE_RED = 255, BASE_GREEN = 255, BASE_BLUE = 255,
                SELECT_RED = 32, SELECT_GREEN = 255, SELECT_BLUE = 64;
  for (int i = 0; i < Areas.size(); i++) {
    if (i == currentPolygon) {
      Areas[i].drawPolygon(renderer, SELECT_RED, SELECT_GREEN, SELECT_BLUE);
    } else {
      Areas[i].drawPolygon(renderer, BASE_RED, BASE_GREEN, BASE_BLUE);
    }
  }
}
