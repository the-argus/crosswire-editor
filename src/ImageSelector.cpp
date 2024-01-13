#include "ImageSelector.h"
#include <SDL2/SDL_image.h>
#include <filesystem>
#include <iostream>

ImageSelector::~ImageSelector() noexcept {
  for (auto &tex : textures) {
    SDL_DestroyTexture(tex.tex);
  }
}

ImageSelector::ImageSelector(SDL_Renderer *renderer, const char *imagefolder,
                             size_t max_images) noexcept {
  std::filesystem::path folder{imagefolder};

  if (!std::filesystem::exists(folder)) {
    std::cout << "No such file or directory " << folder << std::endl;
    std::abort();
  }
  if (!std::filesystem::is_directory(folder)) {
    std::cout << "Not a directory: " << folder << std::endl;
    std::abort();
  }

  size_t count = 0;
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(folder)) {
    ++count;
    if (count > max_images)
      break;
    if (!entry.is_regular_file())
      continue;

    if (entry.path().extension() == ".png") {
      std::cout << "Loading image " << entry.path() << " into level editor."
                << std::endl;

      SDL_Surface *image_surface = IMG_Load(entry.path().c_str());
      if (image_surface == nullptr) {
        std::cout << "Error loading " << entry.path() << ": " << IMG_GetError()
                  << std::endl;
      }

      SDL_Texture *texture =
          SDL_CreateTextureFromSurface(renderer, image_surface);
      if (texture == nullptr) {
        std::cout
            << "Error creating texture from surface for image. Skipping.\n";
        break;
      }

      SDL_FreeSurface(image_surface);

      textures.push_back(Texture{
          .filename = entry.path(),
          .basename = entry.path().stem(),
          .tex = texture,
      });
    }
  }
}

/// Returns a list of the basenames of the image files
std::vector<std::string> ImageSelector::get_image_names() const noexcept {
  std::vector<std::string> strings;
  strings.reserve(size());

  for (const auto &tex : textures) {
    strings.push_back(tex.basename);
  }

  return strings;
}

/// Get a texture for an image file at a given index
SDL_Texture *ImageSelector::get(size_t index) const noexcept {
  return textures[index].tex;
}
