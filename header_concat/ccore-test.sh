#!/bin/sh
#./header_concat ../../libCCORE/include/CCORE.h out/CCORE.h v
zig build-exe main.zig

./main ../../libCCORE/include/CCORE.h out/CCORE.h