{
  description = "chip8 emulator dev environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: 
  let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in
  {
    devShells.x86_64-linux.default = pkgs.mkShell {
      packages = with pkgs; [
        tmux
        zsh
        gcc
        gdb
        gnumake
      ];
      nativeBuildInputs = with pkgs; [
        criterion
        SDL2
      ];
      shellHook = ''
      tmux a -t chip8
      '';
    };
  };
}
