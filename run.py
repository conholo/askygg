import argparse
import subprocess
import os
import json

parser = argparse.ArgumentParser()
parser.add_argument('--mode', choices=['headless', 'editor'], required=True,
                    help="Run the application in headless or editor mode.")
parser.add_argument('--build_type', choices=['debug', 'release'], default='release',
                    help="Choose to run the debug or release build. Default is release.")
args = parser.parse_args()

with open('settings.json') as f:
    settings = json.load(f)

input_dir = settings['input_dir']
output_dir = settings['output_dir']
config_file = settings['config_file']

# This scripts directory.
script_dir = os.path.dirname(os.path.realpath(__file__))

# Construct the build directory path based on the build type argument
build_dir = os.path.join(script_dir, f"build_{args.build_type}")

# Determine the application path based on the mode argument
app_path = os.path.join(build_dir, f"askygg_editor", f"askygg_editor")

# Create the command to run the application (askygg_editor)
cmd = [app_path, f"--{args.mode}", "--input_dir", input_dir, "--output_dir", output_dir, "--config_file", config_file]

subprocess.run(cmd)

