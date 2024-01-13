#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

class ImageSelector {

public:
  ImageSelector() = delete;
  ~ImageSelector() noexcept;
  ImageSelector(SDL_Renderer *renderer, const char *imagefolder,
                size_t max_images = 512) noexcept;

  /// Returns a list of the basenames of the image files
  std::vector<std::string> get_image_names() const noexcept;

  /// Get a texture for an image file at a given index
  SDL_Texture *get(size_t index) const noexcept;

  /// The number of textures in the container
  constexpr inline size_t size() const noexcept { return textures.size(); }

private:
  struct Texture {
    std::string filename;
    std::string basename;
    SDL_Texture *tex;
  };

  std::vector<Texture> textures;
};
