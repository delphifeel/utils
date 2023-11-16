clang -ggdb main.c -o header_concat_d
gdb --args ./header_concat_d include/CCORE.h out/CCORE.h
