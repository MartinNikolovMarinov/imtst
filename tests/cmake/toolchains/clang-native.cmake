# Native Clang toolchain for local builds.
# Usage:
#   cmake --preset clang-debug
#   cmake --preset clang-release

set(CMAKE_C_COMPILER clang CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER clang CACHE STRING "ASM compiler" FORCE)

# Prefer LLVM binutils when available...

# LLVM's AR wrapper allows LLVM to create, modify and extract archives.
find_program(LLVM_AR NAMES llvm-ar)
if(LLVM_AR)
    set(CMAKE_AR "${LLVM_AR}" CACHE FILEPATH "Archiver" FORCE)
else()
    message(WARNING "clang-native toolchain: llvm-ar not found; using CMake default archiver.")
endif()

# LLVM's ranlib wrapper allows LLVM to generates an index to the contents of an archive and store it in the archive.
find_program(LLVM_RANLIB NAMES llvm-ranlib)
if(LLVM_RANLIB)
    set(CMAKE_RANLIB "${LLVM_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)
else()
    message(WARNING "clang-native toolchain: llvm-ranlib not found; using CMake default ranlib.")
endif()

# LLVM's nm wrapper allows LLVM to list symbols from object files.
find_program(LLVM_NM NAMES llvm-nm)
if(LLVM_NM)
    set(CMAKE_NM "${LLVM_NM}" CACHE FILEPATH "nm" FORCE)
else()
    message(WARNING "clang-native toolchain: llvm-nm not found; using CMake default nm.")
endif()
