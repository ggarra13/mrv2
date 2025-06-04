import struct
import sys
import os

def generate_header(spv_filepath, output_filepath, var_name, namespace=""):
    with open(spv_filepath, 'rb') as f:
        spv_data = f.read()

    with open(output_filepath, 'w') as f:
        header_guard = os.path.basename(output_filepath).replace('.', '_').upper()
        f.write(f"#ifndef {header_guard}\n")
        f.write(f"#define {header_guard}\n\n")
        f.write("#include <cstdint>\n")
        f.write("#include <cstddef>\n\n")

        f.write("namespace tl {\n\n")
        if namespace:
            f.write(f"    namespace {namespace} {{\n\n")

        f.write(f"   const uint32_t {var_name}[] = {{\n    ")

        # Write bytes as hex literals
        words = struct.iter_unpack("<I", spv_data)
        for i, (word,) in enumerate(words):
            f.write(f"0x{word:08x}, ")
            if (i + 1) % 4 == 0:
                f.write("\n    ")

        # Handle the last byte and closing brace
        if len(spv_data) > 0:
             # Remove trailing comma if needed
            f.seek(f.tell() - 2, 0) # Go back 2 characters (', ')
            f.truncate() # Remove trailing comma and space
        f.write("\n};\n\n")


        f.write(f"const size_t {var_name}_len = sizeof({var_name});\n\n")

        if namespace:
            f.write(f"    }} // namespace {namespace}\n\n")

        f.write("} // namespace tl\n")
            
        f.write(f"#endif // {header_guard}\n")

    print(f"Generated {output_filepath}")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print(f"Usage: python {sys.argv[0]} <input.spv> <output.h> <variable_name> [namespace]")
        sys.exit(1)

    spv_file = sys.argv[1]
    output_file = sys.argv[2]
    variable_name = sys.argv[3]
    namespace_name = sys.argv[4] if len(sys.argv) > 4 else ""

    generate_header(spv_file, output_file, variable_name, namespace_name)
    
