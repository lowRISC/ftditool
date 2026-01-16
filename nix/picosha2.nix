# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

{
  stdenv,
  lib,
  fetchFromGitHub,
  cmake,
}:
stdenv.mkDerivation rec {
  pname = "picosha2";
  version = "v1.0.1";

  src = fetchFromGitHub {
    owner = "okdshin";
    repo = "PicoSHA2";
    rev = "${version}";
    sha256 = "sha256-3psCzbrwR+vO9TyTKOx+gEaWuHDx6pSgLOQ3DqrJsnI=";
  };

  dontBuild = true;

  installPhase = ''
    mkdir -p $out/include
    cp picosha2.h $out/include
  '';

  meta = with lib; {
    description = "Header-only SHA256 implementation for C++";
    homepage = "https://github.com/okdshin/PicoSHA2";
    license = licenses.mit;
    platforms = platforms.all;
  };
}
