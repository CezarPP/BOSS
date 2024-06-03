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

void ataInterruptHandler() {
    // do nothing as we are using PIO
}

struct Ata {
    static constexpr const uint16_t BYTES_PER_SECTOR = 512;

    Port16Bit dataPort;
    Port8Bit errorPort;
    Port8Bit sectorCountPort;
    Port8Bit lbaLowPort;
    Port8Bit lbaMidPort;
    Port8Bit lbaHiPort;
    Port8Bit devicePort; // master or slave
    Port8Bit commandPort; // instruction - read, write
    Port8Bit controlPort;

    bool isMaster;

    constexpr Ata(uint16_t portBase, bool isMaster) :
            dataPort(portBase), errorPort(portBase + 1),
            sectorCountPort(portBase + 2), lbaLowPort(portBase + 3),
            lbaMidPort(portBase + 4), lbaHiPort(portBase + 5),
            devicePort(portBase + 6), commandPort(portBase + 7),
            controlPort(portBase + 0x206) {
        this->isMaster = isMaster;

        uint8_t a[1024] = "sal boss";
        write28(1, a, 9);
        flush();
        read28(1, 9);
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

        // Data is ready
        Logger::instance().println("[ATA] Identify:\n");
        for (uint16_t i = 0; i < 256; i++) {
            uint16_t data = dataPort.read();
            char foo[] = "  \0";
            foo[1] = (char) ((data >> 8) & 0xFF);
            foo[0] = (char) (data & 0xFF);
            Logger::instance().printStr(foo);
        }
        Logger::instance().println("");
    }

    void read28(uint32_t sectorNumber, int count) {
        // TODO return a vector
        kAssert(sectorNumber <= 0x0FFFFFFF, "[ATA] SectorNumber too big!");

        devicePort.write((isMaster ? 0xE0 : 0xF0) | ((sectorNumber & 0x0F000000) >> 24));
        errorPort.write(0);
        sectorCountPort.write(1);
        lbaLowPort.write(sectorNumber & 0x000000FF);
        lbaMidPort.write((sectorNumber & 0x0000FF00) >> 8);
        lbaLowPort.write((sectorNumber & 0x00FF0000) >> 16);
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

        uint8_t data[1024];
        for (int i = 0; i < count; i += 2) {
            uint16_t w_data = dataPort.read();

            data[i] = (char) (w_data & 0xFF);
            if (i + 1 < count)
                data[i + 1] = (char) ((w_data >> 8) & 0xFF);
            else
                data[i + 1] = 0;

            char text[] = "  \0";
            text[0] = (char) (w_data & 0xFF);
            if (i + 1 < count)
                text[1] = (char) ((w_data >> 8) & 0xFF);
            else
                text[1] = '\0';
            Logger::instance().println(text);
        }

        // Always read a full sector
        for (int i = count + (count % 2); i < BYTES_PER_SECTOR; i += 2)
            dataPort.read();
    }

    void write28(uint32_t sectorNumber, const uint8_t *data, int count) {
        kAssert(sectorNumber <= 0x0FFFFFFF, "[ATA] SectorNumber too big!");
        kAssert(count <= BYTES_PER_SECTOR, "[ATA] Count should be less that 512");

        devicePort.write((isMaster ? 0xE0 : 0xF0) | ((sectorNumber & 0x0F000000) >> 24));
        errorPort.write(0); // clear previous error messages
        sectorCountPort.write(1); // write a single sector
        lbaLowPort.write(sectorNumber & 0x000000FF); // split sector number into these 3 ports
        lbaMidPort.write((sectorNumber & 0x0000FF00) >> 8);
        lbaLowPort.write((sectorNumber & 0x00FF0000) >> 16);
        commandPort.write(0x30); // write command


        Logger::instance().println("[ATA] Writing to ATA Drive...");

        for (int i = 0; i < count; i += 2) {
            uint16_t w_data = data[i];
            if (i + 1 < count)
                w_data |= ((uint16_t) data[i + 1]) << 8;
            dataPort.write(w_data);

            /*char text[] = "  \0";
            text[0] = (char) ((w_data >> 8) & 0xFF);
            text[1] = (char) (w_data & 0xFF);
            Logger::instance().println(text);*/
        }

        // Always write a full sector
        for (int i = count + (count % 2); i < BYTES_PER_SECTOR; i += 2)
            dataPort.write(0x0000);
        Logger::instance().println("[ATA] Successfully written to ATA disk");
    }

    // Flush the cache of the hard drive
    void flush() {
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