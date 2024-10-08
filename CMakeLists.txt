set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# It is best practice to set the toolchain via the command line.
# Example: cmake -DCMAKE_TOOLCHAIN_FILE=x86_64_toolchain.cmake .
if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "No C++ compiler specified. \
             A toolchain can be specified with: cmake -DCMAKE_TOOLCHAIN_FILE=x86_64_toolchain.cmake .")
endif()

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)

project(BOSS
        VERSION 0.8
        LANGUAGES C CXX ASM_NASM)

if (NOT DEFINED ARCH)
    set(ARCH "x86_64")
endif ()

set(CMAKE_CXX_LINK_FLAGS "-ffreestanding -z max-page-size=0x1000 -T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld -ffreestanding -nostdlib")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <CMAKE_CXX_LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES> <LINK_FLAGS>")
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> -I${CMAKE_CURRENT_SOURCE_DIR}/src/include/arch/${ARCH}/asm/ -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
add_compile_options(-g -m64 -ffreestanding -nostdlib -mno-red-zone -fno-rtti -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -nodefaultlibs -Wall -Wextra -Wtype-limits -Wmaybe-uninitialized -fno-exceptions)
# -g compile with debug symbols

include_directories(include/)

file(GLOB CXX_SOURCES "*/*.cpp")
file(GLOB CXX_SUB_SOURCES "*/*/*.cpp")
file(GLOB ASM_ARCH_SRCS "arch/${ARCH}/asm/*.asm")


add_executable(boss ${CXX_SOURCES} ${CXX_SUB_SOURCES} ${CXX_ARCH_SRCS} ${ASM_ARCH_SRCS})

set(ISO_FILE ${CMAKE_CURRENT_BINARY_DIR}/boss.iso)

set(HDD_IMAGE ${CMAKE_CURRENT_BINARY_DIR}/mydisk.img)

set(HDD_IMAGE_SIZE_MB 64)

add_custom_command(OUTPUT ${ISO_FILE}
        DEPENDS boss
        COMMAND touch ${HDD_IMAGE}
        COMMAND dd if=/dev/zero of=${HDD_IMAGE} bs=1M count=${HDD_IMAGE_SIZE_MB}
        COMMAND cp -R ${CMAKE_CURRENT_SOURCE_DIR}/image ${CMAKE_CURRENT_BINARY_DIR}/image/
        COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/boss ${CMAKE_CURRENT_BINARY_DIR}/image/iso/boot/boss.bin
        COMMAND grub-mkrescue -o ${ISO_FILE} ${CMAKE_CURRENT_BINARY_DIR}/image/iso
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(RunQEMU
        COMMAND qemu-system-${ARCH} -cdrom ${ISO_FILE}
        COMMENT Run OS image in QEMU
)

add_custom_target(RunQEMUMonitor
        COMMAND qemu-system-${ARCH} -serial file:${CMAKE_CURRENT_SOURCE_DIR}/serial.log -cdrom ${ISO_FILE} -monitor telnet::4444,server,nowait
        # telnet localhost 4444
        # sendkey ctrl-alt-delete
        COMMENT Run OS image in QEMU with monitor
)

add_custom_target(Run-STDIO-QEMU
        COMMAND xterm -e qemu-system-${ARCH} -cdrom ${ISO_FILE} -monitor stdio
        COMMENT Run OS image in QEMU, get input from stdio
)

add_custom_target(LogQEMU
        # -d int -no-reboot
        COMMAND qemu-system-${ARCH} -serial file:${CMAKE_CURRENT_SOURCE_DIR}/serial.log -cdrom ${ISO_FILE} -drive file=${HDD_IMAGE},format=raw,if=ide,index=0,media=disk
        COMMENT Log output from QEMU
)

add_custom_target(DebugQEMU2
        COMMAND qemu-system-${ARCH} -s -S -serial file:${CMAKE_CURRENT_SOURCE_DIR}/serial.log -cdrom ${ISO_FILE}
        COMMENT Debug OS image in QEMU
)

add_custom_target(DebugQEMU
        COMMAND xterm -e qemu-system-${ARCH} -s -S -serial stdio -cdrom ${ISO_FILE} & gdb boss -batch -ex 'target remote localhost:1234' -ex 'b start' -ex 'c' -ex 'disconnect'#> qemu-debug.printChar 2>&1 &
        COMMAND gdb boss -ex 'set arch i386:x86-64' -ex 'target remote localhost:1234'
        COMMENT Debug OS image in QEMU
)

add_custom_target(iso DEPENDS ${ISO_FILE})

add_dependencies(RunQEMU iso)
add_dependencies(RunQEMUMonitor iso)
add_dependencies(LogQEMU iso)
add_dependencies(Run-STDIO-QEMU iso)
add_dependencies(DebugQEMU iso)
add_dependencies(DebugQEMU2 iso)