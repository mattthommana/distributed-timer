import argparse
import subprocess
import os
import shlex
import logging

logging.basicConfig(level=logging.INFO)

def execute(cmd_str, cwd=None, combine_stderr=True):
    cmd = shlex.split(cmd_str.replace('"', '').replace("'", ''))
    stderr_setting = subprocess.PIPE if not combine_stderr else subprocess.STDOUT

    with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=stderr_setting, universal_newlines=True, cwd=cwd) as popen:
        for stdout_line in iter(popen.stdout.readline, ""):
            yield stdout_line

        return_code = popen.wait()
        if return_code:
            if not combine_stderr:
                error_output = popen.stderr.read()
                logging.error(f"Command {cmd_str} failed with error:\n{error_output}")
            raise subprocess.CalledProcessError(return_code, cmd)

def ensure_directory_exists(directory_path):
    """Ensures that the given directory exists. If not, creates it."""
    if not os.path.exists(directory_path):
        os.makedirs(directory_path)

def build_library(repo_name,repo_url, library_version, source_cache_dir, build_cache_dir, install_cache_dir, num_parallel, recurse_submodules=False, cmake_args="", offline_mode=False):
    build_cache_dir = os.path.join(build_cache_dir, repo_name)
    source_cache_dir= os.path.join(source_cache_dir, repo_name)
    ensure_directory_exists(source_cache_dir)
    ensure_directory_exists(build_cache_dir)
    ensure_directory_exists(install_cache_dir)

    if offline_mode:
        # No download in offline mode
        download_command = ""
    else:
        submodule_option = "--recurse-submodules" if recurse_submodules else ""
        # Get from git repository
        download_command = f"git clone --branch {library_version} --depth 1 {submodule_option} -j{num_parallel} {repo_url} {source_cache_dir}"

    for l in execute(download_command):
        print(l)

    cmake_command = f"cmake {cmake_args} {source_cache_dir}"
    for l in execute(cmake_command, cwd=build_cache_dir):
        print(l)

    build_command = f"cmake --build . --config Release --parallel {num_parallel}"
    for l in execute(build_command, cwd=build_cache_dir):
        print(l)

def install_library(repo_name, build_cache_dir, num_parallel):
    build_cache_dir = os.path.join(build_cache_dir, repo_name)
    install_command = f"cmake --build . --target install --config Release --parallel {num_parallel}"
    for l in execute(install_command, cwd=build_cache_dir):
        print(l)

def call_protoc(protoc_path, proto_input_path, proto_bindings_dir):
    protoc_command = f"{protoc_path} --cpp_out={proto_bindings_dir} --proto_path={os.path.dirname(proto_input_path)} {proto_input_path}"
    subprocess.check_call(protoc_command, shell=True)

def main():
    parser = argparse.ArgumentParser(description="Library utilities CLI")
    subparsers = parser.add_subparsers(dest="action")

    # Subparser for the build_library function
    build_parser = subparsers.add_parser("build")
    build_parser.add_argument("repo_name", help="Name of the repository/library to build.")
    build_parser.add_argument("repo_url", help="URL of the repository to clone.")
    build_parser.add_argument("library_version", help="Version/branch of the library to build.")
    build_parser.add_argument("source_cache_dir", help="Directory where the source code should be cloned to.")
    build_parser.add_argument("build_cache_dir", help="Directory for the build artifacts.")
    build_parser.add_argument("install_cache_dir", help="Directory for the installed library.")
    build_parser.add_argument("num_parallel", type=int, help="Number of parallel threads for building.")
    build_parser.add_argument("--recurse_submodules", action="store_true", help="Recursively initialize submodules when cloning.")
    build_parser.add_argument("--cmake_args", default="", help="Additional CMake arguments.")
    build_parser.add_argument("--offline_mode", action="store_true", help="Whether to work in offline mode without cloning the repository.")

    # Subparser for the install_library function
    install_parser = subparsers.add_parser("install")
    install_parser.add_argument("repo_name", help="Name of the repository/library to install.")
    install_parser.add_argument("build_cache_dir", help="Directory of the built library.")
    install_parser.add_argument("num_parallel", type=int, help="Number of parallel threads for installation.")

    # Subparser for protoc
    protoc_parser = subparsers.add_parser("protoc")
    protoc_parser.add_argument("protoc_path", help="Path to the protoc binary.")
    protoc_parser.add_argument("proto_input_path", help="Path to the input proto file.")
    protoc_parser.add_argument("proto_bindings_dir", help="Directory where generated bindings will be placed.")

    args = parser.parse_args()

    if args.action == "protoc":
        call_protoc(args.protoc_path, args.proto_input_path, args.proto_bindings_dir)


    args = parser.parse_args()

    if args.action == "build":
        print(args.repo_name, args.repo_url, args.library_version, args.source_cache_dir, args.build_cache_dir, args.install_cache_dir, args.num_parallel, args.recurse_submodules, args.cmake_args, args.offline_mode)
        build_library(args.repo_name, args.repo_url, args.library_version, args.source_cache_dir, args.build_cache_dir, args.install_cache_dir, args.num_parallel, args.recurse_submodules, args.cmake_args, args.offline_mode)
    elif args.action == "install":
        install_library(args.repo_name, args.build_cache_dir, args.num_parallel)
    elif args.action == "protoc":
        call_protoc(args.protoc_path, args.proto_input_path, args.proto_bindings_dir)



if __name__ == "__main__":
    main()