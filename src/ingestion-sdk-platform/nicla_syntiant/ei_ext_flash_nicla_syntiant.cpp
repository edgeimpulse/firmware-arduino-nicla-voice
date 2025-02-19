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
#include "ei_ext_flash_nicla_syntiant.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

/** 32-bit align write buffer size */
#define WORD_ALIGN(a)	((a & 0x3) ? (a & ~0x3) + 0x4 : a)
/** Align addres to given sector size */
#define SECTOR_ALIGN(a, sec_size)	((a & (sec_size-1)) ? (a & ~(sec_size-1)) + sec_size : a)

/**
 * @brief 
 * 
 */
void EiExtFlashMemory::test_flash(void)
{
    //fflush(stdout);

    ei_printf("Init spifi\n");
    
    printf("spif size: %ld\n",         memory_size);
    printf("spif block size: %ld\n",   block_size);
    printf("spif config size: %ld\n",   config_size);
    //printf("spif base address: %ld\n",   base_address);
    printf("spif read size: %llu\n",    _spif.get_read_size());
    printf("spif program size: %llu\n", _spif.get_program_size());
    printf("spif erase size: %llu\n",   _spif.get_erase_size());

    printf("Mount ok\n");

    const uint16_t size_test = 4000;
    uint8_t test_wr[size_test] = {0};
    uint32_t result;
    uint64_t start_time = 0;
    uint64_t stop_time = 0;

    for (uint16_t i = 0; i < size_test; i ++){
        test_wr[i] = i;
    }
        
    result = erase_data(0, 4096);    
    printf("Erase data result: %ld\n", result);

    start_time =  rtos::Kernel::get_ms_count();
    result = write_data(test_wr, 0, size_test/4);
    stop_time =  rtos::Kernel::get_ms_count();
    printf("Write data result: %ld\n", result);
    printf("Written %d bytes and took %ld ms\n", result, (stop_time-start_time));

    result = erase_data(0, 4096);    
    printf("Erase data result: %ld\n", result);

    start_time =  rtos::Kernel::get_ms_count();
    result = write_data(test_wr, 0, size_test/2);
    stop_time =  rtos::Kernel::get_ms_count();
    printf("Write data result: %ld\n", result);
    printf("Written %d bytes and took %ld ms\n", result, (stop_time-start_time));

    result = erase_data(0, 4096);    
    printf("Erase data result: %ld\n", result);

    start_time =  rtos::Kernel::get_ms_count();
    result = write_data(test_wr, 0, size_test);
    stop_time =  rtos::Kernel::get_ms_count();
    printf("Write data result: %ld\n", result);
    printf("Written %d bytes and took %ld ms\n", size_test, (stop_time-start_time));
    printf("\n");
}

/**
 * @brief 
 * 
 * @param data 
 * @param address 
 * @param num_bytes 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::write_data(const uint8_t *data, uint32_t address, uint32_t num_bytes)
{
    int ret_val = _spif.program(data, base_address+address, num_bytes);
    if (ret_val == SPIF_BD_ERROR_OK)
    {
        return num_bytes;
    }
    else{
        printf("Error tryng to write to address %ld this bytes %ld", address, num_bytes);
        return 0;
    }    
}

/**
 * @brief 
 * 
 * @param data 
 * @param address 
 * @param num_bytes 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::read_data(uint8_t *data, uint32_t address, uint32_t num_bytes)
{
    int ret_val = _spif.read(data, base_address + address, num_bytes);
    if (ret_val == SPIF_BD_ERROR_OK)
    {
        return num_bytes;
    }
    else{
        return 0;
    } 
}

/**
 * @brief 
 * 
 * @param address 
 * @param num_bytes 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::erase_data(uint32_t address, uint32_t num_bytes)
{
    uint32_t bytes_to_really_erase = num_bytes/erase_block_size;
    uint16_t i;
    uint32_t retval = 0;

    if ((num_bytes%erase_block_size) != 0){
        bytes_to_really_erase++;
    }

    for (i = 0; i < bytes_to_really_erase; i++){
        retval = _spif.erase(base_address + address + (i*erase_block_size), erase_block_size);
        if (retval != SPIF_BD_ERROR_OK){
            ei_printf("Error in erease %ld", retval);
            break;
        }
    }

    return retval;    
}

/**
 * @brief 
 * 
 * @param sample_data 
 * @param address 
 * @param sample_data_size 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::read_sample_data(uint8_t *sample_data, uint32_t address, uint32_t sample_data_size)
{
    return read_data(sample_data, address, sample_data_size);
}

/**
 * @brief 
 * 
 * @param sample_data 
 * @param address 
 * @param sample_data_size 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::write_sample_data(const uint8_t *sample_data, uint32_t address, uint32_t sample_data_size)
{
    return write_data(sample_data, address, sample_data_size);
}

/**
 * @brief 
 * 
 * @param address 
 * @param num_bytes 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::erase_sample_data(uint32_t address, uint32_t num_bytes)
{
    return erase_data(address, num_bytes);
}

/**
 * @brief 
 * 
 * @param filename 
 * @return true 
 * @return false 
 */
bool EiExtFlashMemory::get_file_exist(std::string filename)
{
    bool file_exist = false;

    std::string full_file_path = _root_path + filename;
    //fflush(stdout);
    FILE* fp = fopen(full_file_path.c_str(), "rb");

    if (fp != nullptr){ // NULL or nullptr ?
        file_exist = true;
    }

    //fflush(stdout);
            
    fclose(fp);

    return file_exist;
}

/**
 * @brief 
 * 
 * @param filelist 
 * @return true 
 * @return false 
 */
void EiExtFlashMemory::print_file_list(void)
{
    bool retval = false;
    DIR *d = opendir("/fs");    

    if (d){    
        retval = true;
        while (true) {
            struct dirent *e = readdir(d);            
            
            if (!e) {
                break;
            }

            if ((strcmp(e->d_name, ".") != 0) &&
                (strcmp(e->d_name, "..") != 0)
                ){
                    //filelist += e->d_name;
                    ei_printf("%s\n", e->d_name);
            }              
        }
    }
    else{
        ei_printf("Error in opening /fs \n");
    }
    closedir(d);
}

/**
 * @brief 
 * 
 * @param filename 
 */
bool EiExtFlashMemory::delate_file(std::string filename)
{
    bool deleted = false;
    int retval;
    std::string full_file_path = _root_path + filename;

    fflush(stdout);
    retval = remove(full_file_path.c_str());
    fflush(stdout);

    if (retval == 0){
        deleted = true;
    }else{
        ei_printf("File not deleted, errcode: %d\n", retval);
    }

    return deleted;
}

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool EiExtFlashMemory::format_flash(void)
{    
    int retval;
    bool formatted = true;
    fflush(stdout);
    
    retval = _lfs.format(&_spif);
    if (retval != 0){
        formatted = false;
        ei_printf("Error %d when trying to format ext flash\n", retval);
    }

    return formatted;
}

/**
 * @brief 
 * 
 */
void EiExtFlashMemory::init_fs(void)
{
    _spif.init();
    _lfs.mount(&_spif);
}

/**
 * @brief 
 * 
 */
void EiExtFlashMemory::deinit_fs(void)
{
    _lfs.unmount();
    _spif.deinit();
}

/**
 * @brief Construct a new Ei Flash Memory:: Ei Flash Memory object
 * 
 * @param config_size 
 */
EiExtFlashMemory::EiExtFlashMemory(void) 
    : EiDeviceMemory(0, 90, 0, 256),
    _spif(SPI_PSELMOSI0, SPI_PSELMISO0, SPI_PSELSCK0, CS_FLASH, 8000000),
    _lfs("fs"),
    erase_block_size(4096)  /* from data sheet */
{
    _spif.init();    
    /*
     * TODO calc base address and used blocks (?) maybe useless ?
    */    

    _lfs.mount(&_spif);
    memory_size = (uint32_t)_spif.size()/256;
    
    base_address = _spif.size()/2;
    
    _lfs.unmount();
    _spif.deinit();
}
