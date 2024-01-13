{
  description = "Level editor for the game Crosswire";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }: let
    supportedSystems = let
      inherit (flake-utils.lib) system;
    in [
      system.aarch64-linux
      system.x86_64-linux
    ];
  in
    flake-utils.lib.eachSystem supportedSystems (system: let
      pkgs = import nixpkgs {inherit system;};

      # combine sdl image with regular sdl
      sdl2 = pkgs.stdenv.mkDerivation {
        name = "sdl2";
        version = "custom";
        src = pkgs.emptyDirectory;
        installPhase = ''
          mkdir -p $out/include/SDL2
          for file in ${pkgs.SDL2.dev}/include/SDL2/*; do
            echo $file
            ln -sf $file $out/include/SDL2/
          done
          for file in ${pkgs.SDL2_image}/include/SDL2/*; do
            ln -sf $file $out/include/SDL2/
          done
        '';
      };
    in {
      devShell =
        pkgs.mkShell
        {
          packages = with pkgs; [
            gdb
            valgrind
            pkg-config
            sdl2
            zig_0_11
            SDL2
            SDL2_image
          ];
        };

      packages = {
        default = sdl2;
      };

      formatter = pkgs.alejandra;
    });
}
