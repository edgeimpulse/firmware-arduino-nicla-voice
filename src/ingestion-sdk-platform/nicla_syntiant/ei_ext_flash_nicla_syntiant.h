/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef EI_EXT_FLASH_NICLA_SYNTIANT_H
#define EI_EXT_FLASH_NICLA_SYNTIANT_H

#include "firmware-sdk/ei_device_memory.h"
#include "mbed.h"
#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"

/**
 * @brief Class to handle the extenral NOR flash
 * @note The externl flash is handled using the lfs filesystem
 * 
 */
class EiExtFlashMemory : public EiDeviceMemory {
public:    
    uint32_t read_sample_data(uint8_t *sample_data, uint32_t address, uint32_t sample_data_size);
    uint32_t write_sample_data(const uint8_t *sample_data, uint32_t address, uint32_t sample_data_size);
    uint32_t erase_sample_data(uint32_t address, uint32_t num_bytes);
    uint32_t get_erase_block_size(void)
    {
        return this->erase_block_size;
    }
    bool get_file_exist(std::string filename);
    void print_file_list(void);
    bool delate_file(std::string filename);
    bool format_flash(void);

protected:
    uint32_t read_data(uint8_t *data, uint32_t address, uint32_t num_bytes);
    uint32_t write_data(const uint8_t *data, uint32_t address, uint32_t num_bytes);
    uint32_t erase_data(uint32_t address, uint32_t num_bytes);
    
public:
    EiExtFlashMemory(void);
    void test_flash(void);
    void init_fs(void);
    void deinit_fs(void);
    
private:
    uint64_t base_address;
    const uint32_t erase_block_size;
    SPIFBlockDevice _spif;
    mbed::LittleFileSystem _lfs;
    bool _fs_is_init;
    const std::string _root_path = "/fs/";
    const std::string _local_file_name = "sample_file.bin";
};

#endif