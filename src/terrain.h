#pragma once

#include <cstdint>

namespace cw {

// NOTE: this gets casted to and from int in places
enum class TerrainType : uint8_t {
  Ditch = 0,
  Obstacle,
};

} // namespace cw
