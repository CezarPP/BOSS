# BO<sup>3</sup>SS

## Basic Object-Oriented Operating SSystem

BO<sup>3</sup>SS is an operating system written in _C++20_ planning to leverage Modern C++ concepts and Design Patterns.

It's main focus is not raw performance, but rather understandability of the code for learning purposes.

## Compiling & running the kernel

You can use one of the CMake targets which in term use _qemu_ to run the OS.
I choose CMake because I wanted a smooth developer experience with Clion.

## Features

* [x] Logging to ports
* [x] Printing to screen
* [x] Interrupts & Exceptions
* [x] Keyboard driver
* [x] Paging & Physical & Virtual allocators
* [x] malloc() and custom ::operator new
* [ ] C++ STD library replica (in progress)
* [x] ATA Hard disk driver
* [x] SimpleFS file system
* [ ] Virtual File System (VFS) (in progress)