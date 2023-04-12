#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  make -f /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  make -f /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  make -f /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd "/Users/charles/Documents/Max 8/Packages/max-sdk/source/cb/maxlang.modulator/build"
  make -f /Users/charles/Documents/Max\ 8/Packages/max-sdk/source/cb/maxlang.modulator/build/CMakeScripts/ReRunCMake.make
fi

