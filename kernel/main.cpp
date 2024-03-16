#include "util/print.h"
#include "arch/x86_64/interrupts.h"

extern "C" void kernel_main(void *multiboot) {
    setupInterrupts();

    Printer::instance().print_clear();
    Printer::instance().print_set_color(PRINTER_COLORS::PRINT_COLOR_YELLOW, PRINTER_COLORS::PRINT_COLOR_BLACK);
    Printer::instance().print_str("Welcome to our 64-bit kernel!");
    Printer::instance().print_hex(reinterpret_cast<uint64_t>(multiboot));
}