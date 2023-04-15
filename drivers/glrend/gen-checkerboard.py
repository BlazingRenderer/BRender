#!/usr/bin/env python3

SIZE=64

BLACK = "0, 0, 0, 1, "
MAGENTA = "255, 0, 255, 1, "

print("#include <cstdint>\n")
print("const uint8_t checkerboard_rgba[] = {")

for j in range(SIZE):
    print('\t', end='')
    for i in range(SIZE):
        if j < SIZE/2:
            if i < SIZE/2:
                print(BLACK, end='')
            else:
                print(MAGENTA, end='')
        else:
            if i < SIZE/2:
                print(MAGENTA, end='')
            else:
                print(BLACK, end='')
    print()
print("};")
