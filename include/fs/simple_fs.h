/*
 * simple_fs.h
 *
 *  Created on: 6/10/24.
 *      Author: Cezar PP
 */


#pragma once

#include "file_system.h"
#include "drivers/ata.h"
#include "std/array.h"
#include "std/algorithm.h"
#include "simple_fs_structures.h"

namespace simple_fs {
    /**
     * @brief A class representing the SimpleFS file system
     */

    struct SimpleFS : public vfs::FileSystem {
    private:
        // Disk* in the base class
        std::vector<bool> occupied_block; ///> Bitmap for free blocks
        SuperBlock MetaData{}; ///> File system metadata
        std::vector<int> inode_counter; ///> Stores the number of Inode contained in an Inode Block
        std::vector<uint32_t> dir_counter; ///> Stores the number of Directory contained in a Directory Block
        Directory curr_dir; ///> Caches the current directory to save a disk-read
        bool isMounted{}; ///> Check whether the filesystem has been mounted

        void checkDiskNotMounted() const ;

        void checkFsMounted() const;

    public:
        explicit SimpleFS(Disk *disk) : FileSystem(disk) {}

        /**
         * @brief prints the basic outline of the disk
         * @return void function; returns nothing
        */
        void debug();

        /**
         * @brief formats the entire disk
         * @return void function; returns nothing
        */
        void format();

        void mount();

        /**
         * @brief creates a new inode
         * @return the inumber of the newly created inode
        */
        size_t create();


        bool load_inode(size_t inumber, Inode *node);


        /**
         * @brief removes the inode
         * @param inumber index into the inode table of the inode to be removed
         * @return true if the remove operation was successful; false otherwise
        */
        bool remove(size_t inumber);

        size_t stat(size_t inumber);

        void read_helper(uint32_t blocknum, int offset, int *length, uint8_t **data, uint8_t **ptr);

        size_t read(size_t inumber, uint8_t *data, int length, size_t offset);

        uint32_t allocate_block();

        size_t write_ret(size_t inumber, Inode *node, int ret);

        void read_buffer(int offset, int *read, int length, uint8_t *data, uint32_t blocknum);

        bool check_allocation(Inode *node, int read, int orig_offset, uint32_t &blocknum, bool write_indirect,
                              Block indirect);


        /**
         * @brief write to the disk
         * @param inumber index into the inode table of the corresponding inode
         * @param data data buffer
         * @param length bytes to be written to disk
         * @param offset start point of the write operation
         * @return bytes written to disk; -1 in case of an error
        */
        size_t write(size_t inumber, uint8_t *data, int length, size_t offset);


        //////// DIRECTORIES
        Directory add_dir_entry(Directory dir, uint32_t inum, uint32_t type, const char name[]);

        Directory read_dir_from_offset(uint32_t offset);

        void write_dir_back(Directory dir);

        int dir_lookup(Directory dir, const char name[]);

        bool ls_dir(const char name[]);

        bool mkdir(const char name[NAME_SIZE]);

        Directory rmdir_helper(Directory parent, const char name[]);

        void stat();

        ///////// FILES
        Directory rm_helper(Directory dir, const char name[NAME_SIZE]);

        bool rmdir(char name[NAME_SIZE]);

        bool touch(const char name[NAME_SIZE]);

        bool cd(const char name[NAME_SIZE]);

        bool ls();

        bool rm(const char name[]);
    };
}