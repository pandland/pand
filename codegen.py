import os

def generate_cpp_header(folder_path, output_file):
    header_guard = "JS_MODULES_H"
    with open(output_file, 'w') as f:
        f.write("/* AUTO GENERATED FILE - DO NOT EDIT IT MANUALLY */\n")
        f.write(f"#ifndef {header_guard}\n")
        f.write(f"#define {header_guard}\n\n")
        f.write("#include <unordered_map>\n")
        f.write("#include <string>\n\n")
        f.write("std::unordered_map<std::string, std::string> js_internals = {\n")
        
        first = True
        for file_name in os.listdir(folder_path):
            if file_name.endswith(".js"):
                module_name = os.path.splitext(file_name)[0]
                file_path = os.path.join(folder_path, file_name)
                
                with open(file_path, 'r', encoding='utf-8') as js_file:
                    source_code = js_file.read().replace('\n', '\\n').replace('"', '\\"')
                
                if not first:
                    f.write(",\n")
                first = False
                
                f.write(f'    {{"std:{module_name}", "{source_code}"}}')
        
        f.write("\n};\n\n")
        f.write(f"#endif // {header_guard}\n")

folder_path = "./js"
output_file = "core/js_internals.hh" 

generate_cpp_header(folder_path, output_file)
