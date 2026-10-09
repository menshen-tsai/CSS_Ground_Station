#!/usr/bin/env python3
import os
import sys

# Get absolute path to the script's directory (project root)
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = script_dir
project_name = os.path.basename(project_root)

# Path to Core/Inc inside the project
header_dir = os.path.join(project_root, "Core", "Inc")
header_path = os.path.join(header_dir, "project_info.h")

# Create Core/Inc if it doesn't exist
if not os.path.isdir(header_dir):
    try:
        os.makedirs(header_dir)
        print(f"[INFO] Created missing folder: {header_dir}")
    except Exception as e:
        print(f"[ERROR] Could not create {header_dir}: {e}", file=sys.stderr)
        sys.exit(1)

# Write header file
try:
    with open(header_path, "w", encoding="utf-8") as f:
        f.write("#ifndef PROJECT_INFO_H\n")
        f.write("#define PROJECT_INFO_H\n")
        f.write(f"#define PROJECT_NAME \"{project_name}\"\n")
        f.write("#endif\n")
    print(f"[INFO] Generated {header_path} with PROJECT_NAME=\"{project_name}\"")
except Exception as e:
    print(f"[ERROR] Failed to write {header_path}: {e}", file=sys.stderr)
    sys.exit(1)
