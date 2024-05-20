# BO<sup>3</sup>SS

## Basic Object-Oriented Operating SSystem

## Compiling the kernel

```bash
make all
```

## Running the kernel

```bash
qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso
```

## Features

* [x] Logging to ports
* [x] Printing to screen
* [x] Interrupts & Exceptions
* [x] Keyboard driver
* [x] Paging & Physical & Virtual allocators
* [x] malloc() and custom ::operator new
* [ ] C++ STD library replica (in progress)
* [ ] Virtual File System (VFS) & FAT (in progress)