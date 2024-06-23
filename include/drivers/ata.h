/*
 * ata.h
 *
 *  Created on: 6/3/24.
 *      Author: Cezar PP
 */


#pragma once

#include "arch/x86_64/io_ports.h"
#include "arch/x86_64/logging.h"
#include "arch/x86_64/exceptions.h"
#include "disk_driver.h"

/*
 * ATA - Advanced Technology Attachment
 * PIO - Programmed IO - less efficient, but easier to implement
 * 28-bit mode & 48-bit mode - address sectors
 * sector = 512 bytes
 *
 * CHS - Cylinder, Head, Section - legacy mode of addressing
 * LBA - Logical Block Address - get the sector with a certain number
 * There is a formula to translate CHS to LBA
 *
 *
 * DMA - Direct Memory Access - faster because the processor can do other stuff during the transfer
 *
 * We are implementing PIO 28-bit
 * Port numbers are usually standard, although they could be detected
 *
 * On the same bus there are master & slave drives, but these words hold no meaning
 */

namespace ata {
    // Base addresses for primary and secondary ATA
    constexpr const unsigned int ATA_PRIMARY = 0x1F0;
    constexpr const unsigned int ATA_SECONDARY = 0x170;

    // I/O Controllers ports
    constexpr const unsigned int ATA_DATA = 0;
    constexpr const unsigned int ATA_ERROR = 1;
    constexpr const unsigned int ATA_NSECTOR = 2;
    constexpr const unsigned int ATA_SECTOR = 3;
    constexpr const unsigned int ATA_LCYL = 4;
    constexpr const unsigned int ATA_HCYL = 5;
    constexpr const unsigned int ATA_DRV_HEAD = 6;
    constexpr const unsigned int ATA_STATUS = 7;
    constexpr const unsigned int ATA_COMMAND = 7;
    constexpr const unsigned int ATA_DEV_CTL = 0x206;

    // Status bits
    constexpr const unsigned int ATA_STATUS_BSY = 0x80;
    constexpr const unsigned int ATA_STATUS_DRDY = 0x40;
    constexpr const unsigned int ATA_STATUS_DRQ = 0x08;
    constexpr const unsigned int ATA_STATUS_ERR = 0x01;
    constexpr const unsigned int ATA_STATUS_DF = 0x20;

    // Commands
    constexpr const unsigned int ATA_IDENTIFY = 0xEC;
    constexpr const unsigned int ATA_READ_BLOCK = 0x20;
    constexpr const unsigned int ATA_WRITE_BLOCK = 0x30;

    // Control bits
    constexpr const unsigned int ATA_CTL_SRST = 0x04;
    constexpr const unsigned int ATA_CTL_nIEN = 0x02;

    // Master/Slave on devices
    constexpr const unsigned int MASTER_BIT = 0;
    constexpr const unsigned int SLAVE_BIT = 1;

    constexpr const uint32_t CONTROLLER_TIMEOUT = 1'000'000'000;

    static constexpr const uint32_t SECTOR_SIZE = 512;


    class Ata final : public Disk {
    private:
        Port16Bit dataPort;
        Port8Bit errorPort;
        Port8Bit sectorCountPort;
        Port8Bit lbaLowPort;
        Port8Bit lbaMidPort;
        Port8Bit lbaHiPort;
        Port8Bit devicePort; ///> master or slave
        Port8Bit commandPort; ///> instruction - read, write
        Port8Bit controlPort;

        static volatile bool primary_invoked;

        bool detailedLoggingEnabled{false};

        static void primary_controller_handler() {
            Ata::primary_invoked = true;
        }

    public:
        bool isMaster;

        Ata(uint16_t portBase, bool isMaster) : Disk(),
                                                dataPort(portBase + ATA_DATA), errorPort(portBase + ATA_ERROR),
                                                sectorCountPort(portBase + ATA_NSECTOR),
                                                lbaLowPort(portBase + ATA_SECTOR),
                                                lbaMidPort(portBase + ATA_LCYL), lbaHiPort(portBase + ATA_HCYL),
                                                devicePort(portBase + ATA_DRV_HEAD),
                                                commandPort(portBase + ATA_COMMAND),
                                                controlPort(portBase + ATA_DEV_CTL) {
            kAssert(isMaster, "[ATA] Only master is supported at the moment!");
            this->isMaster = isMaster;
            setInterruptHandler(0x2E, primary_controller_handler);
            setInterruptHandler(0xE, primary_controller_handler);
        }

        void enableDetailedLogging() {
            this->detailedLoggingEnabled = true;
        }

        bool select_device() {
            auto wait_mask = ATA_STATUS_BSY | ATA_STATUS_DRQ;

            if (!wait_for_controller(wait_mask, 0, CONTROLLER_TIMEOUT))
                return false;

            // Indicate the selected device
            devicePort.write(0xA0);

            if (!wait_for_controller(wait_mask, 0, CONTROLLER_TIMEOUT))
                return false;

            return true;
        }

        static void ata_wait_irq_primary() {
            volatile int x = 0;
            for (int i = 0;!primary_invoked && i < 10000; i++) {
                x += 1;
                asm volatile ("nop");
                asm volatile ("nop");
                asm volatile ("nop");
                asm volatile ("nop");
                x += 1;
                asm volatile ("nop");
            }

            primary_invoked = false;
        }

        inline void ata_400ns_delay() {
            // This makes sure the loop is not optimized away
            volatile int x = 0;
            for (uint8_t i = 0; i < 15; i++) {
                asm volatile ("nop");
                asm volatile ("nop");
                asm volatile ("nop");
                asm volatile ("nop");
                x += commandPort.read();
                asm volatile ("nop");
                asm volatile ("nop");
                asm volatile ("nop");
                asm volatile ("nop");
            }
        }

        uint32_t wait_for_controller(uint8_t mask, uint8_t value, uint32_t timeout) {
            uint8_t status;
            do {
                // Sleep at least 400ns before reading the status register
                ata_400ns_delay();

                // Final read of the controller status
                status = commandPort.read();
            } while ((status & mask) != value && --timeout);

            return timeout;
        }

        enum class sector_operation {
            READ,
            WRITE,
            CLEAR
        };

        void read_write_sector(uint64_t start, void *data, sector_operation operation) {
            //Select the device
            kAssert(select_device(), "[ATA] Could not select device!");

            uint8_t sc = start & 0xFF;
            uint8_t cl = (start >> 8) & 0xFF;
            uint8_t ch = (start >> 16) & 0xFF;
            uint8_t hd = (start >> 24) & 0x0F;

            auto command = operation == sector_operation::READ ? ATA_READ_BLOCK : ATA_WRITE_BLOCK;

            // Process the command
            sectorCountPort.write(1);
            lbaLowPort.write(sc);
            lbaMidPort.write(cl);
            lbaHiPort.write(ch);
            devicePort.write((1 << 6) | hd);
            commandPort.write(command);

            /**- Wait at most 30 seconds for BSY flag to be cleared */
            if (detailedLoggingEnabled)
                Logger::instance().println("[ATA] Waiting for controller...");
            kAssert(wait_for_controller(ATA_STATUS_BSY, 0, CONTROLLER_TIMEOUT), "[ATA] Error wait");
            if (detailedLoggingEnabled)
                Logger::instance().println("[ATA] Finished waiting!");
            // Verify if there are errors
            kAssert(!(commandPort.read() & ATA_STATUS_ERR), "[ATA] Error status");

            auto *buffer = reinterpret_cast<uint16_t *>(data);


            if (operation == sector_operation::WRITE) {
                for (int i = 0; i < 256; ++i)
                    dataPort.write(*buffer++);
            } else if (operation == sector_operation::CLEAR) {
                for (int i = 0; i < 256; ++i)
                    dataPort.write(0);
            }

            // Wait the IRQ to happen
            if (detailedLoggingEnabled)
                Logger::instance().println("[ATA] Waiting for IRQ primary...");
            ata_wait_irq_primary();
            if (detailedLoggingEnabled)
                Logger::instance().println("[ATA] Finished waiting!");

            // The device can report an error after the IRQ
            kAssert(!(commandPort.read() & ATA_STATUS_ERR), "[ATA] Error after IRQ");

            if (operation == sector_operation::READ) {
                // Read the disk sectors
                for (int i = 0; i < 256; ++i) {
                    *buffer++ = dataPort.read();
                }
            }
        }

        // Check if there is a hard drive and of what type
        void identity() {
            Logger::instance().println("[ATA] Identifying hard drives...");
            // Whether we want to talk to the master or the slave
            devicePort.write(isMaster ? 0xA0 : 0xB0);
            controlPort.write(0);

            devicePort.write(0xA0);
            uint8_t status = commandPort.read();
            if (status == 0xFF) {
                Logger::instance().println("[ATA] There is no hard on this bus!");
                kPanic("[ATA] No hard on this bus!");
                return;
            }

            devicePort.write(isMaster ? 0xA0 : 0xB0);
            sectorCountPort.write(0);
            lbaLowPort.write(0);
            lbaMidPort.write(0);
            lbaHiPort.write(0);
            commandPort.write(ATA_IDENTIFY); // identity command

            status = commandPort.read();
            if (status == 0x00) {
                // no device
                Logger::instance().println("[ATA] There is no device here!");
                kPanic("[ATA] No device!");
                return;
            }

            while (((status & 0x80) == 0x80)
                   && ((status & 0x01) != 0x01))
                status = commandPort.read();
            if ((status & 0x01)) {
                Logger::instance().println("[ATA] Error...");
                kPanic("[ATA] Error");
                return;
            }

            // Data is ready, read the information
            uint16_t info[256];
            for (unsigned short &b: info) {
                b = dataPort.read();
            }

            controlPort.write(0);

            // Get the size of the disk
            this->cntBlocks_ = *reinterpret_cast<uint32_t *>(reinterpret_cast<size_t>(&info[0]) + 114) / 2;
            this->totalSize_ = this->cntBlocks_ * SECTOR_SIZE;

            Logger::instance().println("[ATA] We have a disk with %X sectors of size %X:\n",
                                       this->cntBlocks_, this->totalSize_);
        }

        void read(size_t blockIndex, uint8_t *data) override {
            sanityCheck(blockIndex, data);
            read_write_sector(blockIndex, data, sector_operation::READ);
            cntReads_++;
        }

        void write(size_t blockIndex, uint8_t *data) override {
            sanityCheck(blockIndex, data);
            read_write_sector(blockIndex, data, sector_operation::WRITE);
            cntWrites_++;
        }

/*        void flush()  {
            devicePort.write(isMaster ? 0xE0 : 0xF0);
            commandPort.write(0xE7); // flush command

            uint8_t status = commandPort.read();
            if (status == 0x00)
                return;

            while (((status & 0x80) == 0x80)
                   && ((status & 0x01) != 0x01))
                status = commandPort.read();

            if (status & 0x01) {
                Logger::instance().println("[ATA] Error flushing cache...");
                return;
            }
        }*/
    };
}