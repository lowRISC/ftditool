# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

{
  stdenv,
  lib,
  fetchFromGitHub,
}:
stdenv.mkDerivation rec {
  pname = "elfio";
  version = "Release_3.12";

  src = fetchFromGitHub {
    owner = "serge1";
    repo = "ELFIO";
    rev = "${version}";
    sha256 = "sha256-tDRBscs2L/3gYgLQvb1+8nNxqkr8v1xBkeDXuOqShX4=";
  };

  dontBuild = true;

  installPhase = ''
    mkdir -p $out/include
    cp -r elfio $out/include
  '';

  meta = with lib; {
    description = "Header-only ELF implementation for C++";
    homepage = "https://github.com/serge1/ELFIO";
    license = licenses.mit;
    platforms = platforms.all;
  };
}
