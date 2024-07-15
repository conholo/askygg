import subprocess
import os

script_dir = os.path.dirname(os.path.realpath(__file__))
debug_build_dir = os.path.join(script_dir, "build_debug")
release_build_dir = os.path.join(script_dir, "build_release")

# Create build directories if they do not exist
os.makedirs(debug_build_dir, exist_ok=True)
os.makedirs(release_build_dir, exist_ok=True)

# Helper function to run a command and log its output
def run_command(command, cwd):
    process = subprocess.Popen(command, cwd=cwd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    print(stdout.decode())
    if process.returncode != 0:
        print(stderr.decode())
        raise subprocess.CalledProcessError(process.returncode, command)

# Set CMake commands
cmake_command_debug = [
    "cmake",
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=Debug",
    "-DCMAKE_C_COMPILER=gcc",
    "-DCMAKE_CXX_COMPILER=g++",
    ".."
]

cmake_command_release = [
    "cmake",
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_C_COMPILER=gcc",
    "-DCMAKE_CXX_COMPILER=g++",
    ".."
]

# Run CMake and Ninja in debug build directory
print("Configuring Debug Build...")
run_command(cmake_command_debug, debug_build_dir)
print("Building Debug Targets...")
run_command(["ninja"], debug_build_dir)

# Run CMake and Ninja in release build directory
print("Configuring Release Build...")
run_command(cmake_command_release, release_build_dir)
print("Building Release Targets...")
run_command(["ninja"], release_build_dir)

print("Build completed successfully.")
