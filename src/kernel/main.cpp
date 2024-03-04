#include "print.cpp"

extern "C" void kernel_main() {
    Printer globalPrinter;
    globalPrinter.print_clear();
    globalPrinter.print_set_color(PRINTER_COLORS::PRINT_COLOR_YELLOW, PRINTER_COLORS::PRINT_COLOR_BLACK);
    globalPrinter.print_str("Welcome to our 64-bit kernel!");
}