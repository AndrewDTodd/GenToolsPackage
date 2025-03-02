#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil

def run_command(cmd, cwd=None):
    print("Running:", " ".join(cmd))
    subprocess.check_call(cmd, cwd=cwd)

def main():
    if len(sys.argv) < 7:
        print("Usage: build_clang.py <stamp_file> <build_dir> <install_dir> <source_dir> <config> <target_arch>")
        sys.exit(1)
    
    stamp_file = sys.argv[1]
    build_dir   = sys.argv[2]
    install_dir = sys.argv[3]
    source_dir  = sys.argv[4]
    config      = sys.argv[5]
    target_arch = sys.argv[6]
    
    # If the stamp file exists, clang is already built.
    if os.path.exists(stamp_file):
        print(f"Clang already built for configuration {config}. Skipping build.")
        sys.exit(0)
    
    # Ensure the build directory exists.
    os.makedirs(build_dir, exist_ok=True)
    
    # Configure step: run cmake to configure the LLVM/Clang build.
    configure_cmd = [
        "cmake", "-Wno-dev", "-GNinja",
        f"-DCMAKE_INSTALL_PREFIX={install_dir}",
        "-DLLVM_ENABLE_PROJECTS=clang",
        # Set your target architecture as needed:
        f"-DLLVM_TARGETS_TO_BUILD={target_arch}",
        "-DLLVM_PARALLEL_LINK_JOBS=6",
        "-DLLVM_ENABLE_ASSERTIONS=OFF",
        "-DLLVM_BUILD_LLVM_DYLIB=OFF",
        f"-DCMAKE_BUILD_TYPE={config}",
        source_dir
    ]
    print("Configuring LLVM/Clang for configuration", config)
    run_command(configure_cmd, cwd=build_dir)
    
    # Build step.
    print("Building LLVM/Clang for configuration", config)
    run_command(["cmake", "--build", "."], cwd=build_dir)
    
    # Install step.
    print("Installing LLVM/Clang for configuration", config)
    run_command(["cmake", "--build", ".", "--target", "install"], cwd=build_dir)
    
    # Create the stamp file to mark a successful build.
    print("Creating stamp file:", stamp_file)
    run_command(["cmake", "-E", "touch", stamp_file])
    
    # Clean up the build directory after successful install.
    print("Cleaning up build directory:", build_dir)
    shutil.rmtree(build_dir)
    
    print("Clang build complete for configuration", config)

if __name__ == "__main__":
    main()
