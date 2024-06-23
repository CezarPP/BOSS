#include "fs/simple_fs.h"

#include "arch/x86_64/interrupts.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "console/console_printer.h"
#include "multiboot/multiboot.h"
#include "allocators/virtual_allocator.h"
#include "allocators/kalloc_tests.h"
#include "allocators/kalloc.h"
#include "fs/vfs_tests.h"
#include "arch/x86_64/system_calls.h"

extern "C" void kernel_main(uint64_t multibootAndMagic) {
    // [CONSOLE] Initialize console
    Console::instance().print_clear();
    Console::instance().println("Welcome to BOSS!\n");


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
    KeyboardDriver::instance().activate();
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
    // The virtual allocator also initializes and tests the physical allocator
    Logger::instance().println("[MAIN] Starting to initialize the Physical and Virtual allocators");
    virtual_allocator::VirtualAllocator::init(memory.first, memory.second);
    Logger::instance().println("[MAIN] Physical and Virtual allocators have been initialized");

    // [KALLOC]
    kalloc::init();
    kalloc::tests::runAllTests();
    // From now on we can call kAlloc(), kFree(), but also just new and delete

    // [ATA] initializing disk
    // Constructor also sets up interrupt handler
    ata::Ata ata0m{ata::ATA_PRIMARY, true};
    ata0m.identity();
    ata0m.test();
    /* Ata ata0s{ata::ATA_PRIMARY, false};
     * Ata ata1m{ata::ATA_SECONDARY, true};
     * Ata ata1s{ata::ATA_SECONDARY, false}; */

    // [SimpleFS]
    Console::instance().println("Enabling the file system...");
    simple_fs::SimpleFS simpleFs{&ata0m};
    simpleFs.format();
    simpleFs.debug();

    // [VFS]
    vfs::init(&ata0m);
    vfs::test();

    // [SYSCALL]
    Logger::instance().println("[MAIN] Issuing test system call...");
    char cwd[100];
    auto result = sys_calls::issueSyscall(0x4A, reinterpret_cast<uint64_t>(&cwd), 0xCC, 0xDD, 0xEE);
    Logger::instance().println("[MAIN] Result is %X", result);
    Logger::instance().println("[MAIN] CWD is %s", cwd);

    Console::instance().println("The filesystem has been successfully enabled");
    Console::instance().setPrompt();
    // ata0m.enableDetailedLogging();

    while (true) {
        haltCpu();
    }
}