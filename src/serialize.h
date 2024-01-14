#pragma once
#include "Vec2.h"
#include "terrain.h"
#include <array>
#include <cassert>
#include <cerrno>
// this header will be included in a file compiled with no exceptions, so no
// fstream
#include <cstdio>
#include <cstring>
#include <span>
#include <type_traits>
#include <unistd.h>
#include <vector>

namespace cw {

#define CROSSWIRE_LEVEL_FILE_EXTENSION "cwl"

struct TerrainEntry {
  std::span<const Vec2> verts;
  TerrainType type;
};

// trivially copyable portion of Image
struct ImageData {
  Vec2 position;
  float rotation;
};

struct Image {
  std::span<const char> filename; // string view
  ImageData data;
};

enum class TurretPattern : uint8_t {
  Circle,
  Tracking,
  StraightLine,
};

struct Turret {
  Vec2 position;
  Vec2 direction;
  float fireRateSeconds;
  TurretPattern pattern;
};

// NOTE: this struct is written directly to the level file. do NOT include any
// pointers or std::spans or strings or any reference types at all.
struct PlayerSpawnPoint {
  Vec2 position;
};

struct BuildSite {
  Vec2 position_a;
  Vec2 position_b;
};

struct Level {
  PlayerSpawnPoint player_spawn;
  std::span<const TerrainEntry> terrains;
  std::span<const Image> images;
  std::span<const BuildSite> build_sites;
  std::span<const Turret> turrets;
  /// this is true for levels returned by deserialize. otherwise leave it as
  /// false
  bool needs_freed = false;

  inline ~Level();
};

struct SpanHeader {
  size_t num_items;
};

struct LevelHeader {
  const char header_text[16] = "Crosswire Level";
  size_t magic = 12834734829147;
};

enum class SerializeResultCode : uint8_t {
  Okay = 0,
  NoFolderProvided,
  NoLevelNameProvided,
  NoSuchDirectory,
  FileExists,
  PathTooLong,
  PathEncodingErr,
  AccessDenied,
  TryAgain,
  AlreadyOpen, // device or resource busy
  UnknownFileOpenError,
  FileWriteErr, // failed while in the middle of writing a file
};

inline SerializeResultCode serialize(const char *folder, const char *levelname,
                                     bool overwrite, const Level &level) {
  if (!folder)
    return SerializeResultCode::NoFolderProvided;
  if (!levelname)
    return SerializeResultCode::NoLevelNameProvided;

  std::array<char, 1024> buf;
  int bytes =
      std::snprintf(buf.data(), buf.size(),
                    "%s/%s." CROSSWIRE_LEVEL_FILE_EXTENSION, folder, levelname);
  if (bytes < 0)
    return SerializeResultCode::PathEncodingErr;
  if (bytes > buf.size())
    return SerializeResultCode::PathTooLong;

  if (!overwrite && !access(buf.data(), F_OK)) {
    return SerializeResultCode::FileExists;
  }

  std::FILE *levelfile = std::fopen(buf.data(), "wb");
  if (!levelfile) {
    std::perror("Failed to open crosswire level file");
    switch (errno) {
    case EACCES:
      return SerializeResultCode::AccessDenied;
      break;
    case EAGAIN:
      return SerializeResultCode::TryAgain;
      break;
    case EBUSY:
      return SerializeResultCode::AlreadyOpen;
      break;
    default:
      return SerializeResultCode::UnknownFileOpenError;
    }
  }

  // first write level header
  static constexpr LevelHeader header;
  {
    if (std::fwrite(header.header_text, 1, sizeof(header.header_text),
                    levelfile) != sizeof(header.header_text)) {
      std::fclose(levelfile);
      return SerializeResultCode::FileWriteErr;
    }
  }
  // magic size_t
  if (std::fwrite(&header.magic, sizeof(header.magic), 1, levelfile) != 1) {
    std::fclose(levelfile);
    return SerializeResultCode::FileWriteErr;
  }

  // player spawn
  static_assert(std::is_trivially_copyable_v<decltype(level.player_spawn)>,
                "Player spawn needs to be written to file but it's not "
                "trivially copyable.");
  if (std::fwrite(&level.player_spawn, sizeof(level.player_spawn), 1,
                  levelfile) != 1) {
    std::fclose(levelfile);
    return SerializeResultCode::FileWriteErr;
  }

  // write a span of trivially copyable stuff to a file
  // NOTE: not capable of writing a span of spans
  auto write_span = [levelfile](auto span) -> bool {
    using T = typename decltype(span)::value_type;
    SpanHeader header{
        .num_items = span.size(),
    };
    static_assert(std::is_trivially_copyable_v<T>,
                  "Type passed into write_span is not trivially copyable but "
                  "it needs to be directly written to binary file.");
    static_assert(std::is_trivially_copyable_v<SpanHeader>,
                  "SpanHeader is not trivially copyable, but it needs to be "
                  "directly written to binary file.");
    if (std::fwrite(&header, sizeof(header), 1, levelfile) != 1) {
      return false;
    }
    return std::fwrite(span.data(), sizeof(T), span.size(), levelfile) ==
           span.size();
  };

  // write how many terrain entries there are
  {
    auto size = level.terrains.size();
    if (std::fwrite(&size, sizeof(size), 1, levelfile) != 1) {
      std::fclose(levelfile);
      return SerializeResultCode::FileWriteErr;
    }
  }

  for (auto &terrain : level.terrains) {
    // write the terrain type
    if (std::fwrite(&terrain.type, sizeof(terrain.type), 1, levelfile) != 1) {
      std::fclose(levelfile);
      return SerializeResultCode::FileWriteErr;
    }

    // write the vertices
    if (!write_span(terrain.verts)) {
      std::fclose(levelfile);
      return SerializeResultCode::FileWriteErr;
    }
  }

  if (!write_span(level.turrets)) {
    std::fclose(levelfile);
    return SerializeResultCode::FileWriteErr;
  }

  // write how many image entries there are
  {
    auto size = level.images.size();
    if (std::fwrite(&size, sizeof(size), 1, levelfile) != 1) {
      std::fclose(levelfile);
      return SerializeResultCode::FileWriteErr;
    }
  }

  // write all images
  for (auto &image : level.images) {
    // write filename
    if (!write_span(image.filename)) {
      std::fclose(levelfile);
      return SerializeResultCode::FileWriteErr;
    }

    // write image data, like its position in the level
    if (std::fwrite(&image.data, sizeof(image.data), 1, levelfile) != 1) {
      std::fclose(levelfile);
      return SerializeResultCode::FileWriteErr;
    }
  }

  // write all build sites
  static_assert(std::is_trivially_copyable_v<BuildSite>,
                "Attempt to directly write a BuildSite to a file but its not "
                "trivially copyable.");
  if (!write_span(level.build_sites)) {
    std::fclose(levelfile);
    return SerializeResultCode::FileWriteErr;
  }

  std::fclose(levelfile);
  return SerializeResultCode::Okay;
}

enum class DeserializeResultCode : uint8_t {
  Okay = 0,
  NoFilenameProvided,
  NoLevelOutProvided,
  NoSuchFile,
  WriteFailure,
  AccessDenied,
  TryAgain,
  AlreadyOpen,
  UnknownFileOpenError,
  InvalidHeader,
  EarlyEOF,
  UnknownReadError,
  ShouldNeverHappenUnlessPosixIsBroken,
  NoSuchImageFile,
};

/// Reads some level data from a file, allocate data using malloc. Returned item
/// has a destructor which will automatically free its contents.
inline DeserializeResultCode deserialize(const char *filename, Level *out) {
  if (!filename)
    return DeserializeResultCode::NoFilenameProvided;
  if (!out)
    return DeserializeResultCode::NoLevelOutProvided;

  std::FILE *levelfile = std::fopen(filename, "rb");
  if (!levelfile) {
    std::perror("Failed to open crosswire level file");
    switch (errno) {
    case EACCES:
      return DeserializeResultCode::AccessDenied;
      break;
    case EAGAIN:
      return DeserializeResultCode::TryAgain;
      break;
    case EBUSY:
      return DeserializeResultCode::AlreadyOpen;
      break;
    case ENOENT:
      return DeserializeResultCode::NoSuchFile;
    default:
      return DeserializeResultCode::UnknownFileOpenError;
    }
  }

#define DESERIALIZE_ERRHANDLE(CLEANUP)                                         \
  if (std::feof(levelfile)) {                                                  \
    CLEANUP                                                                    \
    std::fclose(levelfile);                                                    \
    return DeserializeResultCode::EarlyEOF;                                    \
  } else if (std::ferror(levelfile)) {                                         \
    CLEANUP                                                                    \
    std::fclose(levelfile);                                                    \
    return DeserializeResultCode::UnknownReadError;                            \
  } else {                                                                     \
    CLEANUP                                                                    \
    std::fclose(levelfile);                                                    \
    return DeserializeResultCode::ShouldNeverHappenUnlessPosixIsBroken;        \
  }

  // read the header text
  static constexpr LevelHeader header;
  {
    std::array<uint8_t, sizeof(header.header_text)> buf;
    if (std::fread(buf.data(), sizeof(uint8_t), sizeof(header.header_text),
                   levelfile) != buf.size()) {
      DESERIALIZE_ERRHANDLE()
    }
    if (std::memcmp(buf.data(), header.header_text, buf.size()) != 0) {
      std::fclose(levelfile);
      return DeserializeResultCode::InvalidHeader;
    }
  }

  // read the header magic number
  size_t magic;
  if (std::fread(&magic, sizeof(magic), 1, levelfile) != 1) {
    DESERIALIZE_ERRHANDLE()
  }
  if (magic != header.magic) {
    std::fclose(levelfile);
    return DeserializeResultCode::InvalidHeader;
  }

  PlayerSpawnPoint player_spawn;
  if (std::fread(&player_spawn, sizeof(player_spawn), 1, levelfile) != 1) {
    DESERIALIZE_ERRHANDLE()
  }

  size_t num_terrains;
  if (std::fread(&num_terrains, sizeof(num_terrains), 1, levelfile) != 1) {
    DESERIALIZE_ERRHANDLE()
  }

  std::vector<std::vector<Vec2>> polygons;
  std::vector<TerrainType> types;

  types.reserve(num_terrains);
  polygons.reserve(num_terrains);

  for (size_t i = 0; i < num_terrains; ++i) {
    TerrainType type;
    if (std::fread(&type, sizeof(type), 1, levelfile) != 1) {
      DESERIALIZE_ERRHANDLE()
    }
    types.push_back(type);

    size_t num_polygons;
    if (std::fread(&num_polygons, sizeof(num_polygons), 1, levelfile) != 1) {
      DESERIALIZE_ERRHANDLE()
    }
    polygons.emplace_back();
    auto &gons = polygons.back();
    gons.resize(num_polygons);

    if (std::fread(gons.data(), sizeof(Vec2), num_polygons, levelfile) !=
        num_polygons) {
      DESERIALIZE_ERRHANDLE()
    }
  }

  assert(types.size() == polygons.size());
  assert(types.size() == num_terrains);

  size_t num_turrets;
  if (std::fread(&num_turrets, sizeof(num_turrets), 1, levelfile) != 1) {
    DESERIALIZE_ERRHANDLE()
  }

  Turret *turrets = new Turret[num_turrets];
  out->needs_freed = true;

  if (std::fread(turrets, sizeof(Turret), num_turrets, levelfile) !=
      num_turrets) {
    DESERIALIZE_ERRHANDLE(delete[] turrets;)
  }

  size_t num_images;
  if (std::fread(&num_images, sizeof(num_images), 1, levelfile) != 1) {
    DESERIALIZE_ERRHANDLE(delete[] turrets;)
  }

  std::vector<std::vector<char>> filenames;
  std::vector<ImageData> image_datas;
  filenames.reserve(num_images);
  image_datas.reserve(num_images);

  for (size_t i = 0; i < num_images; ++i) {
    size_t num_chars;
    if (std::fread(&num_chars, sizeof(num_chars), 1, levelfile) != 1) {
      DESERIALIZE_ERRHANDLE(delete[] turrets;)
    }

    filenames.emplace_back();
    auto &filename = filenames.back();
    filename.resize(num_chars);

    if (std::fread(filename.data(), sizeof(char), num_chars, levelfile) !=
        num_chars) {
      DESERIALIZE_ERRHANDLE(delete[] turrets;)
    }

    // make sure its null terminated
    filename.push_back(0);

    // read image data
    image_datas.emplace_back();
    ImageData &target = image_datas.back();
    if (std::fread(&target, sizeof(ImageData), 1, levelfile) != 1) {
      DESERIALIZE_ERRHANDLE(delete[] turrets;)
    }
  }

  assert(filenames.size() == num_images);
  assert(filenames.size() == image_datas.size());

  // read build sites
  size_t num_build_sites;
  if (std::fread(&num_build_sites, sizeof(num_build_sites), 1, levelfile) !=
      1) {
    DESERIALIZE_ERRHANDLE(delete[] turrets;)
  }

  BuildSite *sites = new BuildSite[num_build_sites];
  out->needs_freed = true;

  if (std::fread(sites, sizeof(BuildSite), num_build_sites, levelfile) !=
      num_build_sites) {
    DESERIALIZE_ERRHANDLE(delete[] sites; delete[] turrets;)
  }

  // finally shove the data we read into some arrays
  out->player_spawn = player_spawn;
  out->terrains = std::span(new TerrainEntry[types.size()], types.size());
  out->images = std::span(new Image[image_datas.size()], image_datas.size());
  out->build_sites = std::span(sites, num_build_sites);
  out->turrets = std::span(turrets, num_turrets);

  for (size_t i = 0; i < num_terrains; ++i) {
    auto &modifiable = const_cast<TerrainEntry &>(out->terrains[i]);
    auto modifiable_verts =
        std::span(new Vec2[polygons[i].size()], polygons[i].size());
    modifiable.type = types[i];
    size_t index = 0;
    for (const auto &poly : polygons[i]) {
      modifiable_verts[index] = poly;
      ++index;
    }
    modifiable.verts = modifiable_verts;
  }

  for (size_t i = 0; i < num_images; ++i) {
    auto &modifiable = const_cast<Image &>(out->images[i]);
    modifiable.data = image_datas[i];
    auto modifiable_filename =
        std::span(new char[filenames[i].size()], filenames[i].size());
    size_t index = 0;
    for (const auto &c : filenames[i]) {
      modifiable_filename[index] = c;
      ++index;
    }
    modifiable.filename = modifiable_filename;
  }

  std::fclose(levelfile);
  return DeserializeResultCode::Okay;
#undef DESERIALIZE_ERRHANDLE
}

inline Level::~Level() {
  if (needs_freed) {
    delete[] build_sites.data();
    for (auto &image : images) {
      delete[] image.filename.data();
    }
    delete[] images.data();

    for (auto &terrain : terrains) {
      delete[] terrain.verts.data();
    }
    delete[] terrains.data();
  }
}

} // namespace cw
