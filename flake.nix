# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
{
  description = "Ftdi tool";
  inputs = {
    lowrisc-nix.url = "github:lowRISC/lowrisc-nix";
    nixpkgs.follows = "lowrisc-nix/nixpkgs";
    flake-utils.follows = "lowrisc-nix/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  } @ inputs: let
    system_outputs = system: let
      pkgs = import nixpkgs {inherit system;};
      ft4222 = pkgs.callPackage ./nix/ft4222.nix {};
      ftd2xx = pkgs.callPackage ./nix/ftd2xx.nix {};
      libmpsse = pkgs.callPackage ./nix/libmpsse.nix {};
      picosha2 = pkgs.callPackage ./nix/picosha2.nix {};
      ftditool = pkgs.callPackage ./nix/ftditool.nix {
        inherit ft4222;
        inherit ftd2xx;
        inherit libmpsse;
        inherit picosha2;
      };
    in {
      formatter = pkgs.alejandra;
      packages.default = ftditool;
      devShells = {
        default = pkgs.mkShell {
          name = "ftditool env";
          nativeBuildInputs = with pkgs; [
            clang_21
            llvmPackages_21.llvm
            cmake
            gnumake
            reuse
            ft4222
            libmpsse
          ];
          buildInputs = with pkgs; [ft4222 libmpsse ftd2xx libusb1 systemd];
          shellHook = ''
            # Setting LD_LIBRARY_PATH for libftd2xx
            export LD_LIBRARY_PATH="${ftd2xx}/lib:$LD_LIBRARY_PATH"
          '';
        };
      };
    };
  in
    flake-utils.lib.eachDefaultSystem system_outputs;
}
