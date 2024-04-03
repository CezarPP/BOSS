#include "arch/x86_64/interrupts.h"
#include "drivers/keyboard.h"
#include "arch/x86_64/logging.h"
#include "util/console_printer.h"
#include "arch/x86_64/paging.h"

void f() {
    // Logger::instance().println("[MAIN] IN F, x=%X", reinterpret_cast<uint64_t>(&x));
    // Printer::instance().println("[MAIN] in f");
}

extern "C" void kernel_main([[maybe_unused]] void *multiboot) {
    Printer::instance().print_clear();
    Printer::instance().printSetColor(PRINTER_COLORS::PRINT_COLOR_YELLOW, PRINTER_COLORS::PRINT_COLOR_BLACK);


    if (Logger::instance().init()) {
        Printer::instance().println("Logger could not be initialized\nExiting...");
        return;
    }

    Logger::instance().println("START OF OS");

    setupInterrupts();


    KeyboardDriver::activate();
    setInterruptHandler(0x21, KeyboardDriver::handleInterrupt);
    setInterruptHandler(0x20, f);

    Printer::instance().println("[KERNEL MAIN] Welcome to the 64-bit kernel!\n");
    Logger::instance().println("Enabling interrupts");
    enableInterrupts();
    Logger::instance().println("Enabled interrupts");

    paging::init();
    while (true) {
        haltCpu();
    }

    // Printer::instance().print_hex(reinterpret_cast<uint64_t>(multiboot));
}