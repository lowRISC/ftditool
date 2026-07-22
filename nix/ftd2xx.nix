# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
{
  stdenv,
  lib,
  fetchzip,
}:
stdenv.mkDerivation rec {
  pname = "ftd2xx";
  version = "1.4.34";
  src = fetchzip {
    url = "https://storage.googleapis.com/lowrisc-ci-longterm-cache/libftd2xx-linux-x86_64-${version}.tgz";
    hash = "sha256-O47VO8DpNJt2VTjvyEwA4sxLegUrwdazeIh3JHW2URI=";
  };

  dontBuild = true;
  dontConfigure = true;

  installPhase = ''
    mkdir -p $out/lib $out/include
    cp lib*.a $out/lib/
    cp lib*.so* $out/lib/
    cp *.h $out/include/
  '';

  meta = {
    description = "FTDI D2XX libraries";
    platforms = ["x86_64-linux"];
  };
}
