#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil
import tempfile
import platform

def run_command(cmd, cwd=None):
    print("Running:", " ".join(cmd))
    subprocess.check_call(cmd, cwd=cwd)

def main():
    if len(sys.argv) < 5:
        print("Usage: build_clang.py <install_dir> <source_dir> <config> <target_arch>")
        sys.exit(1)
    
    install_dir = sys.argv[1]
    source_dir  = sys.argv[2]
    config      = sys.argv[3]
    target_arch = sys.argv[4]

    print(f"Install directory: {install_dir}")

    if config in ["Debug", "RelWithDebInfo"]:
        clang_build_type = "RelWithDebInfo"
    else:
        clang_build_type = "Release"

    # Define the stamp file in the installation directory.
    stamp_file = os.path.join(install_dir, "built.stamp")
    
    # If the stamp file exists, clang is already built.
    if os.path.exists(stamp_file):
        print(f"Clang already built for configuration {config}. Skipping build.")
        sys.exit(0)
    
    # Ensure the install directory exists.
    #os.makedirs(install_dir, exist_ok=True)

    build_dir = tempfile.mkdtemp(prefix="clang_build_")
    print(f"Created build directory: {build_dir}")
    
    # Configure step: run CMake to configure the LLVM/Clang build.
    configure_cmd = [
        "cmake", "-Wno-dev", "-GNinja",
        f"-DCMAKE_INSTALL_PREFIX={install_dir}",
        "-DLLVM_ENABLE_PROJECTS=clang",
        f"-DLLVM_TARGETS_TO_BUILD={target_arch}",
        "-DLLVM_PARALLEL_LINK_JOBS=6",
        "-DLLVM_ENABLE_ASSERTIONS=OFF",
        "-DLLVM_BUILD_LLVM_DYLIB=OFF",
        "-DLLVM_USE_LINKER=lld",
        f"-DCMAKE_BUILD_TYPE={clang_build_type}",
        source_dir
    ]
    print(f"Configuring LLVM/Clang for {clang_build_type} ...")
    run_command(configure_cmd, cwd=build_dir)
    
    # Build step.
    print(f"Building LLVM/Clang for {clang_build_type} ...")
    run_command(["cmake", "--build", "."], cwd=build_dir)
    
    # Install step.
    print(f"Installing LLVM/Clang to {install_dir} ...")
    run_command(["cmake", "--build", ".", "--target", "install"], cwd=build_dir)
    
    # Create the stamp file to mark a successful build.
    print(f"Creating stamp file: {stamp_file} ...")
    run_command(["cmake", "-E", "touch", stamp_file])
    
    # Clean up the temporary build directory after successful install.
    print(f"Cleaning up build directory: {build_dir} ...")
    shutil.rmtree(build_dir)
    
    print(f"Clang build complete for configuration {config}")

if __name__ == "__main__":
    main()
