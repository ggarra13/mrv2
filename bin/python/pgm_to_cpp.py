#!/usr/bin/env python3
import sys
import os

def read_pgm_binary(path):
    with open(path, "rb") as f:
        # Read magic number
        magic = f.readline().strip()
        if magic != b'P5':
            raise ValueError("Only binary PGM (P5) supported")

        # Read width, height (skip comments)
        def read_non_comment_line():
            line = f.readline()
            while line.startswith(b'#'):
                line = f.readline()
            return line

        dims = read_non_comment_line()
        while len(dims.split()) < 2:
            dims += read_non_comment_line()

        width, height = map(int, dims.split())

        # Read max value
        maxval = int(read_non_comment_line().strip())
        if maxval > 255:
            raise ValueError("Only 8-bit PGM supported (maxval <= 255)")

        # The next byte is the start of binary pixel data
        pixel_data = f.read()

        if len(pixel_data) != width * height:
            raise ValueError("Unexpected pixel data size")

        return width, height, pixel_data


def write_cpp_array(output_path, var_name, width, height, data):
    with open(output_path, "w") as out:
        out.write("#include <cstdint>\n\n")
        out.write(f"const int {var_name}_width = {width};\n")
        out.write(f"const int {var_name}_height = {height};\n")
        out.write(f"const uint8_t {var_name}[] = {{\n")

        for i, byte in enumerate(data):
            if i % 12 == 0:
                out.write("    ")
            out.write(f"0x{byte:02x}, ")
            if i % 12 == 11:
                out.write("\n")

        out.write("\n};\n")


def main():
    if len(sys.argv) != 4:
        print("Usage: pgm_to_cpp.py input.pgm output.cpp variable_name")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    var_name = sys.argv[3]

    width, height, data = read_pgm_binary(input_path)
    write_cpp_array(output_path, var_name, width, height, data)

    print(f"Converted {input_path} -> {output_path}")
    print(f"Resolution: {width}x{height}")


if __name__ == "__main__":
    main()
