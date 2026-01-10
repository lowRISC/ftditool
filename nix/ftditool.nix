# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

{
  ft4222,
  stdenv,
  clang_21,
  cmake,
  gnumake,
  argparse,
  magic-enum,
}:
stdenv.mkDerivation {
  pname = "ftditool";
  version = "0.1.0";
  src = ../.;

  nativeBuildInputs = [
    clang_21
    cmake
    gnumake
  ];

  buildInputs = [
    ft4222
    argparse
    magic-enum
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];
}
