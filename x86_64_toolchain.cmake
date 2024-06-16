# Toolchain for x86_64

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_SYSTEM_VERSION 1) # This is not necessary

# specify the cross compiler
set(CMAKE_C_COMPILER ~/opt/cross/bin/x86_64-elf-gcc)
set(CMAKE_CXX_COMPILER ~/opt/cross/bin/x86_64-elf-g++)

# where is the target environment
# set(CMAKE_FIND_ROOT_PATH /usr/lib64/)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)