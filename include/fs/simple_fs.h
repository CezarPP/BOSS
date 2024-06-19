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
        void checkDiskNotMounted() const;

        void checkFsMounted() const;

        /**
         * @brief creates a new inode
         * @return the inumber of the newly created inode
        */
        ssize_t create();

        /**
         * @brief Allocates the first free block from free block bitmap
         * @return block number of the block allocated; 0 if no block is available
        */
        uint32_t allocate_block();

        /**
         * @brief Writes the node into the corresponding inode block
         * @param inumber index into inode table
         * @param node the inode to be written back to disk
         * @param ret the value that is returned by the function
         * @return returns the parameter ret
        */
        ssize_t write_ret(size_t inumber, Inode *node, int ret);

        /**
         * @brief Reads from buffer and writes to a block in the disk
         * @param offset starts writing at index = offset
         * @param read bytes read from buffer so far
         * @param length bytes to be written to the disk
         * @param data data buffer
         * @param blocknum index of block in free block bitmap
         * @return  void function; returns nothing
        */
        void read_buffer(int offset, int *read, int length, const uint8_t *data, uint32_t blockNum);

        /**
         * @brief Allocates a block if required; if no block is available in the disk; returns false
         * @param node used to set the size of inode in case allocation fails
         * @param read bytes read from buffer so far
         * @param orig_offset offset passed to write() function call
         * @param blockNum index of block in the free block bitmap
         * @param write_indirect true if the block is an indirect node
         * @param indirect the indirect node if required
         * @return true if allocation is successful; false otherwise
        */
        bool check_allocation(Inode *node, int read, int orig_offset, uint32_t &blockNum, bool write_indirect,
                              Block indirect);

        /**
         * @brief Reads the block from disk and changes the pointers accordingly
         * @param blockNum index into the free block bitmap
         * @param offset start reading from index = offset
         * @param length number of bytes to be read
         * @param data data buffer
         * @param ptr buffer to store the read data
         * @return void function; returns nothing
        */
        void read_helper(uint32_t blockNum, int offset, int *length, uint8_t **data, uint8_t **ptr);


        /**
         * @brief Helper function to remove directory from parent directory
         * @param parent Directory from which the other directory is to be removed.
         * @param name Name of the directory to be removed
         * @return Directory. Returns Directory with valid bit=0 in case of error.
         */
        Directory rmdir_helper(Directory parent, const char name[]);

    public:
        // Disk* disk; -> in the base class
        std::vector<bool> occupied_block; ///> Bitmap for free blocks
        SuperBlock MetaData{}; ///> File system metadata
        std::vector<int> inode_counter; ///> Stores the number of Inode contained in an Inode Block
        std::vector<uint32_t> dir_counter; ///> Stores the number of Directory contained in a Directory Block
        Directory curr_dir; ///> Caches the current directory to save a disk-read
        bool isMounted{}; ///> Check whether the filesystem has been mounted

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

        /**
         * @brief loads inode corresponding to inumber into node
         * @param inumber index into inode table
         * @param node pointer to inode
         * @return boolean value indicative of success of the load operation
        */
        bool load_inode(size_t inumber, Inode *node);

        /**
         * @brief removes the inode
         * @param inumber index into the inode table of the inode to be removed
         * @return true if the remove operation was successful; false otherwise
        */
        bool remove(size_t inumber);

        ssize_t stat(size_t inumber);

        //////// DIRECTORIES
        Directory add_dir_entry(Directory dir, uint32_t inum, uint32_t type, const char name[]);

        Directory read_dir_from_offset(uint32_t offset);

        void write_dir_back(Directory dir);

        int dir_lookup(Directory dir, const char name[]);

        bool ls_dir(const char name[], std::vector<vfs::file> &contents);

        void stat();

        Directory rm_helper(Directory dir, const char name[NAME_SIZE]);

        bool cd(const char name[NAME_SIZE]);

        /// Function implementations of the file system interface

        /**
         * @brief Mounts the file system onto the disk
         * @return void function; returns nothing, the mount should be successful
        */
        void mount() override;

        /**
         * @brief Read from disk
         * @param inumber index into the inode table of the corresponding inode
         * @param data data buffer
         * @param length bytes to be read from disk
         * @param offset start point of the read operation
         * @return bytes read from disk; -1 in case of an error
        */
        ssize_t read(size_t inumber, uint8_t *data, int length, size_t offset) override;

        /**
         * @brief write to the disk
         * @param inumber index into the inode table of the corresponding inode
         * @param data data buffer
         * @param length bytes to be written to disk
         * @param offset start point of the write operation
         * @return bytes written to disk; -1 in case of an error
        */
        ssize_t write(size_t inumber, const uint8_t *data, int length, size_t offset) override;

        std::expected<uint32_t> getInode(const char *name) override;

        bool rmdir(const char name[NAME_SIZE]) override;

        bool mkdir(const char name[NAME_SIZE]) override;

        bool touch(const char name[NAME_SIZE]) override;

        bool ls(std::vector<vfs::file> &contents) override;

        bool rm(const char name[]) override;

        void test();
    };
}