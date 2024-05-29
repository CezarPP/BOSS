#include "arch/x86_64/interrupts.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "util/console_printer.h"
#include "multiboot/multiboot.h"
#include "allocators/kalloc.h"


void f() {
    Logger::instance().println("[MAIN] in f");
}

extern "C" void kernel_main(uint64_t multibootAndMagic) {
    // [PRINTER] Initialize printer
    Console::instance().print_clear();
    Console::instance().println("[KERNEL MAIN] Welcome to BOSS!\n");


    // [LOGGER]
    if (Logger::instance().init()) {
        Logger::instance().println("Logger could not be initialized\nExiting...");
        return;
    }

    Logger::instance().println("START OF OS");
    Logger::instance().println("[KERNEL] Multiboot and magic is %X", multibootAndMagic);

    // [INTERRUPTS] Setup
    setupInterrupts();

    // [DRIVERS]
    KeyboardDriver::activate();
    setInterruptHandler(0x21, KeyboardDriver::handleInterrupt);
    setInterruptHandler(0x20, TimerDriver::handleInterrupt);


    // [INTERRUPTS] enable
    Logger::instance().println("[KERNEL MAIN] Enabling interrupts");
    enableInterrupts();
    Logger::instance().println("[KERNEL MAIN] Enabled interrupts");

    // [MULTIBOOT]
    std::pair<uint64_t, uint64_t> memory = parseMultiboot(multibootAndMagic);
    memory = paging::physicalExcludingEarly(memory);

    // [PAGING]
    paging::init();

    // [PHYSICAL ALLOCATOR] [VIRTUAL ALLOCATOR]
    // The virtual allocator also initializes the physical allocator
    Logger::instance().println("[MAIN] Starting to initialize the Physical and Virtual allocators");
    virtual_allocator::VirtualAllocator::init(memory.first, memory.second);
    Logger::instance().println("[MAIN] Physical and Virtual allocators have been initialized");

    kalloc::init();
    // From now on we can call kAlloc(), kFree(), but also just new and delete


    while (true) {
        haltCpu();
    }
}