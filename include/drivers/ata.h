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
    static constexpr const uint16_t SECTOR_SIZE = 512;

    struct AtaDriver {
        static void ataInterruptHandler() {
            // do nothing as we are using PIO
        }
    };

    class Ata final : public Disk {
    private:
        Port16Bit dataPort;
        Port8Bit errorPort;
        Port8Bit sectorCountPort;
        Port8Bit lbaLowPort;
        Port8Bit lbaMidPort;
        Port8Bit lbaHiPort;
        Port8Bit devicePort; // master or slave
        Port8Bit commandPort; // instruction - read, write
        Port8Bit controlPort;

    public:
        bool isMaster;

        constexpr Ata(uint16_t portBase, bool isMaster) : Disk(),
                                                          dataPort(portBase), errorPort(portBase + 1),
                                                          sectorCountPort(portBase + 2), lbaLowPort(portBase + 3),
                                                          lbaMidPort(portBase + 4), lbaHiPort(portBase + 5),
                                                          devicePort(portBase + 6), commandPort(portBase + 7),
                                                          controlPort(portBase + 0x206) {
            this->isMaster = isMaster;

            uint8_t a[SECTOR_SIZE] = "sal boss";
            uint8_t b[SECTOR_SIZE];
            write(1, a);
            flush();
            read(1, a);
            Logger::instance().println("READ FROM ATA %s", b);
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
            commandPort.write(0xEC); // identity command

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

            // Get the size of the disk
            this->cntBlocks_ = *reinterpret_cast<uint32_t *>(reinterpret_cast<size_t>(&info[0]) + 114);
            this->totalSize_ = this->cntBlocks_ * SECTOR_SIZE;

            Logger::instance().println("[ATA] We have a disk with %X sectors of size %X:\n",
                                       this->cntBlocks_, this->totalSize_);
        }

        void read(size_t blockIndex, uint8_t *data) override {
            kAssert(blockIndex <= 0x0FFFFFFF, "[ATA] SectorNumber too big!");

            devicePort.write((isMaster ? 0xE0 : 0xF0) | ((blockIndex & 0x0F000000) >> 24));
            errorPort.write(0);
            sectorCountPort.write(1);
            lbaLowPort.write(blockIndex & 0x000000FF);
            lbaMidPort.write((blockIndex & 0x0000FF00) >> 8);
            lbaLowPort.write((blockIndex & 0x00FF0000) >> 16);
            commandPort.write(0x20); // read command

            uint8_t status = commandPort.read();
            while (((status & 0x80) == 0x80)
                   && ((status & 0x01) != 0x01))
                status = commandPort.read();

            if (status & 0x01) {
                Logger::instance().println("[ATA] Error reading...");
                return;
            }

            Logger::instance().println("[ATA] Reading drive: ");

            for (int i = 0; i < SECTOR_SIZE; i += 2) {
                uint16_t w_data = dataPort.read();

                data[i] = (char) (w_data & 0xFF);
                data[i + 1] = (char) ((w_data >> 8) & 0xFF);
            }

            cntReads_++;
        }

        void write(size_t blockIndex, const uint8_t *data) override {
            kAssert(blockIndex <= 0x0FFFFFFF, "[ATA] SectorNumber too big!");

            devicePort.write((isMaster ? 0xE0 : 0xF0) | ((blockIndex & 0x0F000000) >> 24));
            errorPort.write(0); // clear previous error messages
            sectorCountPort.write(1); // write a single sector
            lbaLowPort.write(blockIndex & 0x000000FF); // split sector number into these 3 ports
            lbaMidPort.write((blockIndex & 0x0000FF00) >> 8);
            lbaLowPort.write((blockIndex & 0x00FF0000) >> 16);
            commandPort.write(0x30); // write command

            Logger::instance().println("[ATA] Writing to ATA Drive...");

            for (int i = 0; i < SECTOR_SIZE; i += 2) {
                uint16_t w_data = data[i];
                w_data |= ((uint16_t) data[i + 1]) << 8;
                dataPort.write(w_data);
            }

            cntWrites_++;
            Logger::instance().println("[ATA] Successfully written to ATA disk");
        }

        void flush() override {
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
        }
    };
}