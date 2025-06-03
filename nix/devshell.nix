{ pkgs }:
pkgs.mkShellNoCC {
  packages = [
    pkgs.python312
    pkgs.platformio
    pkgs.esptool
    pkgs.espflash
    pkgs.go
  ];
}
