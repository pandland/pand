import os
import sys
import subprocess
from typing import Optional

file_path = os.path.abspath(__file__)
PROJECT_ROOT = os.path.dirname(os.path.dirname(file_path))
DEPS_ROOT = os.path.join(PROJECT_ROOT, "deps")

env = os.environ.copy()

def cmd(command: str, cwd: Optional[str] = None):
  subprocess.run(command, env=env, cwd=cwd, shell=True)

# prepare gclient config:
gconfig_location = os.path.join(DEPS_ROOT, ".gclient")
gconfig_content = """solutions = [
  {
    "name": "v8",
    "url": "https://github.com/pandland/v8.git",
    "deps_file": "DEPS",
    "managed": False,
    "custom_deps": {},
    "safesync_url": "",
  },
]
"""

print(f"# Writing config to file: '{gconfig_location}'")
print(gconfig_content)

with open(gconfig_location, "+wt") as file:
  file.write(gconfig_content)

# download depot_tools and prepare $PATH
print("# Downloading depot_tools")
DEPOT_TOOLS_REPO="https://chromium.googlesource.com/chromium/tools/depot_tools.git"
DEPOT_TOOLS_BRANCH="bc85464a"
DEPOT_TOOLS_PATH = os.path.join(DEPS_ROOT, "depot_tools")

cmd(f"git clone {DEPOT_TOOLS_REPO}", DEPS_ROOT)
cmd(f"git checkout {DEPOT_TOOLS_BRANCH}", DEPOT_TOOLS_PATH)

PATH = env["PATH"]
env["PATH"] = f"{PATH}:{DEPOT_TOOLS_PATH}"

print("# Running: 'gclient sync'")
cmd("gclient sync", DEPS_ROOT)

print("# Preparing v8 for build")

V8_DIR = os.path.join(DEPS_ROOT, "v8")
V8_TARGET="x64.release" # TODO: make it configurable
V8_BUILD_DIR=f"{V8_DIR}/out.gn/{V8_TARGET}"
V8_VERSION="12.9.203"

cmd(f"git checkout {V8_VERSION}", V8_DIR)
cmd(f"gclient sync", V8_DIR)

gn_location = os.path.join(V8_BUILD_DIR, "args.gn")
gn_content = """dcheck_always_on = false
is_component_build = false
is_debug = false
target_cpu = \"x64\"
use_custom_libcxx = false
v8_monolithic = true
v8_use_external_startup_data = false
v8_enable_i18n_support = false
treat_warnings_as_errors = false
"""

def use_py(args: str):
  if sys.platform == "win32":
    return "python " + args
  return args

cmd(use_py(f"tools/dev/v8gen.py -vv {V8_TARGET}"), V8_DIR)

with open(gn_location, "+wt") as file:
  file.write(gn_content)

print("# Compiling v8...")
cmd(f"ninja -C {V8_BUILD_DIR} v8_monolith", V8_DIR)
