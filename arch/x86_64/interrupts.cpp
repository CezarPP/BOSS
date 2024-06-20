#include "arch/x86_64/interrupts.h"
#include "arch/x86_64/io_ports.h"
#include "std/cstring.h"
#include "arch/x86_64/exceptions.h"
#include "arch/x86_64/logging.h"
#include "arch/x86_64/system_calls.h"

constexpr auto NUM_IDT_ENTRIES = 256;

// 64-bit Descriptor in IDT
struct InterruptDescriptor {
    uint16_t base_low;        // offset bits 0..15
    uint16_t selector;        // a code segment selector in GDT or LDT
    uint8_t zero;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type_attributes; // gate type, dpl, and p fields
    uint16_t base_middle;        // offset bits 16..31
    uint32_t base_high;        // offset bits 32..63
    uint32_t zero2;            // reserved
}__attribute__((packed));
static_assert(AssertSize<InterruptDescriptor, 16>());

/* The location of the IDT is kept in the IDTR register.
This is loaded using the LIDT assembly instruction, whose argument is a pointer to an IDT Descriptor structure
 */

struct IDTPointer {
    uint16_t limit; // a size one less than the size of the IDT in bytes
    Address base; // The linear Address of the Interrupt Descriptor Table (not the physical Address, paging applies).
} __attribute__((packed));
static_assert(AssertSize<IDTPointer, 10>());

static InterruptHandler interruptHandlers[NUM_IDT_ENTRIES];
InterruptDescriptor idt[NUM_IDT_ENTRIES];
IDTPointer IDT_ptr;

extern "C"
{

extern void defaultIRQ();
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void idtLoad();

extern void asmInterrupt();
}


// OS Dev

constexpr auto MASTER_PIC_COMMAND = 0x20;
constexpr auto MASTER_PIC_DATA = 0x21;
constexpr auto SLAVE_PIC_COMMAND = 0xA0;
constexpr auto SLAVE_PIC_DATA = 0xA1;

constexpr auto SYSTEM_CS = 0x08;
constexpr auto IDT_FLAG = 0x8E;

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

constexpr auto ICW1_ICW4 = 0x01;        /* Indicates that ICW4 will be present */
constexpr auto ICW1_SINGLE = 0x02;        /* Single (cascade) mode */
constexpr auto ICW1_INTERVAL4 = 0x04;        /* Call Address interval 4 (8) */
constexpr auto ICW1_LEVEL = 0x08;        /* Level triggered (edge) mode */
constexpr auto ICW1_INIT = 0x10;        /* Initialization - required! */

constexpr auto ICW4_8086 = 0x01;        /* 8086/88 (MCS-80/85) mode */
constexpr auto ICW4_AUTO = 0x02;        /* Auto (normal) EOI */
constexpr auto ICW4_BUF_SLAVE = 0x08;        /* Buffered mode/slave */
constexpr auto ICW4_BUF_MASTER = 0x0C;        /* Buffered mode/master */


static void remapIRQTable() {
    /*
     * In protected mode, IRQs 0 to 7 conflict with the CPU exceptions, which are reserved by Intel up until 0x1F.
     * Move the interrupt vectors to the beginning of the available range INT 0..0xF -> INT 0x20,...0x2F
     */
    Port8Bit masterCommandPort{MASTER_PIC_COMMAND};
    Port8Bit slaveCommandPort{SLAVE_PIC_COMMAND};
    Port8Bit masterData{MASTER_PIC_DATA};
    Port8Bit slaveData{SLAVE_PIC_DATA};


    // Starts the initialization sequence
    masterCommandPort.write(ICW1_INIT | ICW1_ICW4);
    slaveCommandPort.write(ICW1_INIT | ICW1_ICW4);

    // Vector offsets
    masterData.write(0x20);
    slaveData.write(0x28);

    // Tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    masterData.write(0x04);
    // Tell Slave PIC its cascade identity
    slaveData.write(0x02);

    // Have the PICs use 8086 mode (and not 8080 mode)
    masterData.write(ICW4_8086);
    slaveData.write(ICW4_8086);

    masterData.write(0x0);
    slaveData.write(0x0);
}

void idtSetGate(Byte num, Address base, uint16_t sel, Byte flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_middle = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32);
    idt[num].selector = sel;
    idt[num].zero = 0x0;
    idt[num].type_attributes = flags;
    idt[num].zero2 = 0x0;
}

extern "C" {
void syscallHandler(SystemCallRegisters *registersState) {
    Logger::instance().println("[SYSCALL] System call handler...");
    dumpRegisterState(registersState);
    sys_calls::doSystemCall(registersState);
}
}

void setupInterrupts() {
    // Sets the IDT pointer
    IDT_ptr = {.limit = sizeof(InterruptDescriptor) * NUM_IDT_ENTRIES - 1,
            .base = reinterpret_cast<Address>(&idt)};

    // Initialize IDT with zeros
    memset(&idt, 0, sizeof(InterruptDescriptor) * NUM_IDT_ENTRIES);

    // Nullify all the interrupt handlers.
    memset(&interruptHandlers, 0, sizeof(InterruptHandler) * 256);

    remapIRQTable();

    // Add any new ISRs to the IDT
    idtSetGate(0, (Address) isr0, SYSTEM_CS, IDT_FLAG);
    idtSetGate(1, (Address) isr1, SYSTEM_CS, IDT_FLAG);
    idtSetGate(2, (Address) isr2, SYSTEM_CS, IDT_FLAG);
    idtSetGate(3, (Address) isr3, SYSTEM_CS, IDT_FLAG);
    idtSetGate(4, (Address) isr4, SYSTEM_CS, IDT_FLAG);
    idtSetGate(5, (Address) isr5, SYSTEM_CS, IDT_FLAG);
    idtSetGate(6, (Address) isr6, SYSTEM_CS, IDT_FLAG);
    idtSetGate(7, (Address) isr7, SYSTEM_CS, IDT_FLAG);
    idtSetGate(8, (Address) isr8, SYSTEM_CS, IDT_FLAG);
    idtSetGate(9, (Address) isr9, SYSTEM_CS, IDT_FLAG);
    idtSetGate(10, (Address) isr10, SYSTEM_CS, IDT_FLAG);
    idtSetGate(11, (Address) isr11, SYSTEM_CS, IDT_FLAG);
    idtSetGate(12, (Address) isr12, SYSTEM_CS, IDT_FLAG);
    idtSetGate(13, (Address) isr13, SYSTEM_CS, IDT_FLAG);
    idtSetGate(14, (Address) isr14, SYSTEM_CS, IDT_FLAG);
    idtSetGate(15, (Address) isr15, SYSTEM_CS, IDT_FLAG);
    idtSetGate(16, (Address) isr16, SYSTEM_CS, IDT_FLAG);
    idtSetGate(17, (Address) isr17, SYSTEM_CS, IDT_FLAG);
    idtSetGate(18, (Address) isr18, SYSTEM_CS, IDT_FLAG);
    idtSetGate(19, (Address) isr19, SYSTEM_CS, IDT_FLAG);
    idtSetGate(20, (Address) isr20, SYSTEM_CS, IDT_FLAG);
    idtSetGate(21, (Address) isr21, SYSTEM_CS, IDT_FLAG);
    idtSetGate(22, (Address) isr22, SYSTEM_CS, IDT_FLAG);
    idtSetGate(23, (Address) isr23, SYSTEM_CS, IDT_FLAG);
    idtSetGate(24, (Address) isr24, SYSTEM_CS, IDT_FLAG);
    idtSetGate(25, (Address) isr25, SYSTEM_CS, IDT_FLAG);
    idtSetGate(26, (Address) isr26, SYSTEM_CS, IDT_FLAG);
    idtSetGate(27, (Address) isr27, SYSTEM_CS, IDT_FLAG);
    idtSetGate(28, (Address) isr28, SYSTEM_CS, IDT_FLAG);
    idtSetGate(29, (Address) isr29, SYSTEM_CS, IDT_FLAG);
    idtSetGate(30, (Address) isr30, SYSTEM_CS, IDT_FLAG);
    idtSetGate(31, (Address) isr31, SYSTEM_CS, IDT_FLAG);
    idtSetGate(32, (Address) irq0, SYSTEM_CS, IDT_FLAG);
    idtSetGate(33, (Address) irq1, SYSTEM_CS, IDT_FLAG);
    idtSetGate(34, (Address) irq2, SYSTEM_CS, IDT_FLAG);
    idtSetGate(35, (Address) irq3, SYSTEM_CS, IDT_FLAG);
    idtSetGate(36, (Address) irq4, SYSTEM_CS, IDT_FLAG);
    idtSetGate(37, (Address) irq5, SYSTEM_CS, IDT_FLAG);
    idtSetGate(38, (Address) irq6, SYSTEM_CS, IDT_FLAG);
    idtSetGate(39, (Address) irq7, SYSTEM_CS, IDT_FLAG);
    idtSetGate(40, (Address) irq8, SYSTEM_CS, IDT_FLAG);
    idtSetGate(41, (Address) irq9, SYSTEM_CS, IDT_FLAG);
    idtSetGate(42, (Address) irq10, SYSTEM_CS, IDT_FLAG);
    idtSetGate(43, (Address) irq11, SYSTEM_CS, IDT_FLAG);
    idtSetGate(44, (Address) irq12, SYSTEM_CS, IDT_FLAG);
    idtSetGate(45, (Address) irq13, SYSTEM_CS, IDT_FLAG);
    idtSetGate(46, (Address) irq14, SYSTEM_CS, IDT_FLAG);
    idtSetGate(47, (Address) irq15, SYSTEM_CS, IDT_FLAG);
    for (int i = 48; i < NUM_IDT_ENTRIES; i++) {
        idtSetGate(i, (Address) defaultIRQ, SYSTEM_CS, IDT_FLAG);
    }

    idtSetGate(0x80, (Address) isr128, SYSTEM_CS, IDT_FLAG);  // 0xEE present, ring 3
    idtLoad();

    Logger::instance().println("[KERNEL] Setting up system calls...");
    sys_calls::populateSyscallArray();
    Logger::instance().println("[KERNEL] Finished setting up system calls.");

    Logger::instance().println("[KERNEL] Finished setting up interrupts");
}

const char *exceptionMessages[] = {"Division By Zero", "Debug",
                                   "Non Maskable Interrupt", "Breakpoint", "Into Detected Overflow",
                                   "Out of Bounds", "Invalid Opcode", "No Coprocessor", "Double Fault!",
                                   "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
                                   "Stack Fault", "General Protection Fault!", "Page Fault",
                                   "Unknown Interrupt", "Coprocessor Fault", "Alignment Check Exception",
                                   "Machine Check Exception", "SIMD fp Exception",
                                   "Virtualization Exception", "Reserved", "Reserved", "Reserved",
                                   "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                   "Reserved", "Reserved"};

extern "C"
{

void isrHandler(RegistersState *state) {
    const auto int_no = state->int_no;

    Logger::instance().println("[INTERRUPTS] In ISR handler no: %X", int_no);
    if (int_no < 32) {
        // TODO kException(state, exceptionMessages[int_no]);
        Logger::instance().println("[INTERRUPTS] Called exception ISR nr: %X, %s",
                                   int_no, exceptionMessages[int_no]);
        kPanic("[INTERRUPTS] Exceptions not implemented");
    } else {
        if (interruptHandlers[int_no]) {
            Logger::instance().println("[INTERRUPTS] Calling ISR handler nr: %X", int_no);
            Logger::instance().println("[INTERRUPTS] Maybe syscall with number %X, first arg %X",
                                       state->rax, state->rbx);
            auto handler = interruptHandlers[int_no];
            handler();
        } else {
            Logger::instance().println("[INTERRUPTS] Called not registered ISR nr: %X", int_no);
            kPanic("[INTERRUPTS] Unhandled interrupt!");
        }
    }
}

bool isInterestingInterrupt(uint64_t intNo) {
    // Interrupt 0x2e has to do with the ATA disk, but we are using PIO mode, so we don't need the interrupt
    return intNo != 0x20 && intNo != 0x21 && intNo != 0x2e;
}

void irqHandler(RegistersState *state) {
    if (isInterestingInterrupt(state->int_no))
        Logger::instance().println("[INTERRUPTS] In IRQ handler nr: %X", state->int_no);
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (state->int_no >= 40) {
        // Send reset signal to slave.
        Port8Bit::write8(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    Port8Bit::write8(0x20, 0x20);

    if (interruptHandlers[state->int_no]) {
        if (isInterestingInterrupt(state->int_no)) {
            Logger::instance().println("[INTERRUPTS] Calling IRQ handler nr: %X", state->int_no);
        }
        auto handler = interruptHandlers[state->int_no];
        handler();
    } else {
        Logger::instance().println("[INTERRUPTS] Called not registered IRQ nr: %X", state->int_no);
        kPanic("[INTERRUPTS] Unhandled IRQ");
    }
}

void defHandler() {
    Logger::instance().println("[INTERRUPTS] Called default interrupt handler");
    kPanic("[INTERRUPTS] Interrupt on unset gate!");
}
}

void setInterruptHandler(Byte num, InterruptHandler handler) {
    interruptHandlers[num] = handler;
    auto x = (unsigned int) num;
    Logger::instance().println("[INTERRUPTS] Setup interrupts handler with number %x", x);
}