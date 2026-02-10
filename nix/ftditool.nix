# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
{
  ft4222,
  libmpsse,
  ftd2xx,
  stdenv,
  clang_21,
  cmake,
  gnumake,
  argparse,
  magic-enum,
  picosha2,
  makeWrapper,
}:
stdenv.mkDerivation {
  pname = "ftditool";
  version = "0.1.0";
  src = ../.;

  nativeBuildInputs = [
    clang_21
    cmake
    gnumake
    makeWrapper
  ];

  buildInputs = [
    ft4222
    libmpsse
    argparse
    magic-enum
    picosha2
    ftd2xx
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  postInstall = ''
    wrapProgram $out/bin/ftditool --prefix LD_LIBRARY_PATH : "${ftd2xx}/lib"
  '';
}
