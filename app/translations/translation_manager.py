#
# Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

import os
import subprocess
import argparse

def run_lupdate(ts_dir, app_sources, lib_sources):
    """
    Run lupdate to update the .ts files.
    """
    ts_files = [os.path.join(ts_dir, file) for file in os.listdir(ts_dir) if file.endswith('.ts')]

    if not ts_files:
        print(f"No .ts files found in {ts_dir}.")
        return

    all_sources = app_sources + lib_sources
    command = ['lupdate'] + all_sources + ['-ts'] + ts_files

    print("Running lupdate command:", ' '.join(command))

    try:
        subprocess.run(command, check=True)
        print("lupdate completed successfully, obsolete entries removed.")
    except subprocess.CalledProcessError as e:
        print(f"lupdate failed with error code: {e.returncode}")
        print(e.output)


def run_lrelease(ts_dir, qm_dir):
    """
    Run lrelease to compile the .ts files into .qm files.
    """
    ts_files = [os.path.join(ts_dir, file) for file in os.listdir(ts_dir) if file.endswith('.ts')]
    if not ts_files:
        print(f"No .ts files found in {ts_dir}.")
        return

    if not os.path.exists(qm_dir):
        os.makedirs(qm_dir)

    for ts_file in ts_files:
        qm_file = os.path.join(qm_dir, os.path.splitext(os.path.basename(ts_file))[0] + '.qm')
        command = ['lrelease', ts_file, '-qm', qm_file]
        
        print("Running lrelease command:", ' '.join(command))

        try:
            subprocess.run(command, check=True)
            print(f"lrelease completed successfully, created {qm_file}")
        except subprocess.CalledProcessError as e:
            print(f"lrelease failed with error code: {e.returncode}")
            print(e.output)


def create_ts(language, ts_dir, app_sources, lib_sources):
    """
    Create a new .ts file for the given language code.
    """
    new_ts_file = os.path.join(ts_dir, f"{language}.ts")
    
    if os.path.exists(new_ts_file):
        print(f"The .ts file for language '{language}' already exists.")
        return
    
    print(f"Creating new .ts file: {new_ts_file}")
    all_sources = app_sources + lib_sources
    command = ['lupdate', '-source-language', 'en_US', '-target-language', language] + all_sources + ['-ts', new_ts_file]

    try:
        subprocess.run(command, check=True)
        print(f"New .ts file {new_ts_file} created successfully.")
    except subprocess.CalledProcessError as e:
        print(f"lupdate failed with error code: {e.returncode}")
        print(e.output)


def collect_sources(app_dir, lib_dir):
    """
    Collect all relevant .cpp, .h, and .qml files from app and lib directories.
    """
    app_sources = []
    for root, _, files in os.walk(app_dir):
        for file in files:
            if file.endswith(('.cpp', '.h', '.qml')):
                app_sources.append(os.path.join(root, file))

    lib_sources = []
    if os.path.exists(lib_dir):
        for root, _, files in os.walk(lib_dir):
            for file in files:
                if file.endswith(('.cpp', '.h')):
                    lib_sources.append(os.path.join(root, file))

    return app_sources, lib_sources


if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Manage translations with lupdate and lrelease.")
    parser.add_argument('--new-ts', metavar='LANGUAGE', help='Create a new .ts file for the given language code (e.g., "es" for Spanish).')
    parser.add_argument('--lupdate', action='store_true', help='Run lupdate to update all existing .ts files.')
    parser.add_argument('--lrelease', action='store_true', help='Run lrelease to compile .ts files into .qm files.')

    args = parser.parse_args()

    if not any(vars(args).values()):
        # If no arguments are passed, print the help message
        parser.print_help()
        exit(0)

    # Define paths
    translations_dir = os.path.dirname(os.path.abspath(__file__))  
    app_dir = os.path.dirname(os.path.dirname(translations_dir))
    lib_dir = os.path.join(os.path.dirname(app_dir), 'lib')
    ts_dir = os.path.join(translations_dir, 'ts')
    qm_dir = os.path.join(translations_dir, 'qm')

    # Ensure the translations/ts directory exists
    if not os.path.exists(ts_dir):
        print(f"Error: {ts_dir} directory does not exist.")
        exit(1)

    # Collect source files from app and lib directories
    app_sources, lib_sources = collect_sources(app_dir, lib_dir)

    # Handle --new-ts option
    if args.new_ts:
        create_ts(args.new_ts, ts_dir, app_sources, lib_sources)

    # Handle --lupdate option
    if args.lupdate:
        run_lupdate(ts_dir, app_sources, lib_sources)

    # Handle --lrelease option
    if args.lrelease:
        run_lrelease(ts_dir, qm_dir)
