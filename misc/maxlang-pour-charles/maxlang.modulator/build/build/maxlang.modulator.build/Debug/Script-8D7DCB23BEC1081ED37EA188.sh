#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  cp /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/max-sdk-base/script/PkgInfo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo/Contents/PkgInfo
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  /Applications/CMake.app/Contents/bin/cmake -E copy_directory /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/maxlang.modulator.mxo
fi
if test "$CONFIGURATION" = "Release"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  cp /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/max-sdk-base/script/PkgInfo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo/Contents/PkgInfo
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  /Applications/CMake.app/Contents/bin/cmake -E copy_directory /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/maxlang.modulator.mxo
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  cp /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/max-sdk-base/script/PkgInfo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo/Contents/PkgInfo
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  /Applications/CMake.app/Contents/bin/cmake -E copy_directory /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/maxlang.modulator.mxo
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  cp /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/max-sdk-base/script/PkgInfo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo/Contents/PkgInfo
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  /Applications/CMake.app/Contents/bin/cmake -E copy_directory /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/../../../externals/maxlang.modulator.mxo /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/maxlang.modulator.mxo
fi

