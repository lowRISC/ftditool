# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
{
  stdenv,
  lib,
  fetchzip,
}:
stdenv.mkDerivation rec {
  pname = "libmpsse";
  version = "1.0.8";
  src = fetchzip {
    url = "https://storage.googleapis.com/lowrisc-ci-cache/external/libmpsse-x86_64-${version}.tgz";
    hash = "sha256-9JwRDfQj6KMV+k1WxgKq6IB3hgSXNI9sqX3NNQz2n3c=";
  };

  dontBuild = true;
  dontConfigure = true;

  installPhase = ''
    mkdir -p $out/lib $out/include
    cp build/lib*.a $out/lib/
    cp build/lib*.so* $out/lib/
    cp include/*.h $out/include/
  '';

  meta = {
    description = "FTDI MPSSE libraries";
    platforms = ["x86_64-linux"];
  };
}
