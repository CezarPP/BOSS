# #!/bin/bash
# Installing the cross compiler

cd ~

mkdir -p opt/cross
mkdir -p src

export PREFIX="$HOME/opt/cross"
# export TARGET=i686-elf
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

cd $HOME/src
mkdir build-binutils
cd build-binutils

# CLONE gcc and binutils in src
tar -xzvf gcc-13.2.0.tar.gz
tar -xvf binutils-2.42.tar.xz


# BINUTILS
../binutils-2.42/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror

make
make install # if you need root here start again, do not use sudo

# CLONE gdb in src
# Installing GDB does not yet work
../gdb-14.1/configure --target=$TARGET --prefix="$PREFIX" --disable-werror
make all-gdb # ERROR HERE
make install-gdb

# GCC
cd $HOME/src
which -- $TARGET-as || echo $TARGET-as is not in the PATH
mkdir build-gcc
cd build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc