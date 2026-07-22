# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
{
  stdenv,
  lib,
  fetchzip,
}:
stdenv.mkDerivation rec {
  pname = "ft4222";
  version = "1.4.4.232";
  src = fetchzip {
    url = "https://storage.googleapis.com/lowrisc-ci-longterm-cache/libft4222-linux-${version}.zip";
    hash = "sha256-WGy6llULMvjPcc8L1e7qPh9xYB1a2sFKyoLI7WZWNp0=";
  };

  dontBuild = true;
  dontConfigure = true;

  unpackPhase = ''
    tar -xf $src/libft4222-linux-${version}.tgz
  '';

  installPhase = ''
    mkdir -p $out/lib $out/include
    cp build-x86_64/lib*.a $out/lib/
    cp build-x86_64/lib*.so* $out/lib/
    cp *.h $out/include/
  '';

  meta = {
    description = "FTDI FT4222 libraries";
    platforms = ["x86_64-linux"];
  };
}
