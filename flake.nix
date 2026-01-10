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
      ftditool = pkgs.callPackage ./nix/ftditool.nix {inherit ft4222;};
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
          ];
          buildInputs = [ft4222];
        };
      };
    };
  in
    flake-utils.lib.eachDefaultSystem system_outputs;
}
