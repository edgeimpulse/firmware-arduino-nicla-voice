/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

    get_first_usable_block();

    //printf("statvfs\n");
    //struct statvfs my_statvfs;
    //_lfs.statvfs("/fs", &my_statvfs);
    //printf("number of blocks: %ld \n", my_statvfs.f_blocks);        // ?
    //printf("number of free blocks: %ld \n", my_statvfs.f_bfree);    // ?
    //uint32_t len_file = get_sample_file_len();
    //printf("Sample file size %ld\n", len_file);

    const uint16_t size_test = 4000;
    // test format read write read
    uint8_t test_wr[size_test] = {0};
    //uint8_t test_rd[size_test] = {0};
    uint32_t result;
    uint64_t start_time = 0;
    uint64_t stop_time = 0;

    for (uint16_t i = 0; i < size_test; i ++){
        test_wr[i] = i;
    }

    /*
    printf("Test read 1 \n");
    result = read_sample_file(test_rd, 90, 10);
    printf("Read %d bytes\n", result);
    for (uint8_t i = 0; i < result; i ++){
        printf("%d, ", test_rd[i]);
    }
    printf("\n");
    */
    /* test erase data*/
        
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
    
    /*
    result = read_data(test_rd, 0, size_test);
    printf("Read data result: %ld\n", result);
    for (int i = 0; i< size_test; i++){
        printf("%d, ", test_rd[i]);
    }
    */

#if 0
    printf("Test erase\n");
    erase_sample_file();
    rtos::ThisThread::sleep_for(100);
    printf("Test write \n");
    start_time =  rtos::Kernel::get_ms_count();
    result = write_sample_file(test_wr, 100);
    stop_time =  rtos::Kernel::get_ms_count();
    printf("Written %d bytes and took %ld ms\n", result, (stop_time-start_time));
    rtos::ThisThread::sleep_for(100);

    printf("Test erase\n");
    erase_sample_file();
    rtos::ThisThread::sleep_for(100);
    printf("Test write \n");
    start_time =  rtos::Kernel::get_ms_count();
    result = write_sample_file(test_wr, 10);
    stop_time =  rtos::Kernel::get_ms_count();
    printf("Written %d bytes and took %ld ms\n", result, (stop_time-start_time));
    rtos::ThisThread::sleep_for(100);

    printf("Test erase\n");
    erase_sample_file();
    rtos::ThisThread::sleep_for(100);
    printf("Test write \n");
    start_time =  rtos::Kernel::get_ms_count();
    result = write_sample_file(test_wr, size_test);        
    stop_time =  rtos::Kernel::get_ms_count();
    rtos::ThisThread::sleep_for(100);
    printf("Written %d bytes and took %ld ms\n", result, (stop_time-start_time));
#endif
    /*
    printf("Test read 2\n");
    result = read_sample_file(test_rd, 0, 100);
    printf("Read %d bytes\n", result);
    for (uint8_t i = 0; i < result; i ++){
        printf("%d, ", test_rd[i]);
    }
    */
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
 * @param data 
 * @param num_bytes 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::write_sample_file(const uint8_t *data, uint32_t num_bytes)
{
    std::string full_file_path;
    uint32_t ret_val = 0;

    if ((data != nullptr) && (num_bytes != 0)){

        full_file_path = _root_path + _local_file_name;
        //fflush(stdout);
        FILE* fp = fopen(full_file_path.c_str(), "rb");

        if (fp == nullptr){ // NULL or nullptr ?
        //fflush(stdout);
            fclose(fp);
            //fflush(stdout);
            fp = fopen(full_file_path.c_str(), "wb");

        }
        else
        {
            //fflush(stdout);
            fclose(fp);
            //fflush(stdout);
            fp = fopen(full_file_path.c_str(), "ab");
        }

        if (fp != nullptr){
            ret_val = fwrite(data, sizeof(uint8_t), num_bytes, fp);
        }
        else{
            ei_printf("[ERR] Error in creating sample file!\n");
        }
        //fflush(stdout);
        fclose(fp);
                
    }
    else
    {
        /* code */        
    }
    
    return ret_val;
}

/**
 * @brief Read content of file
 * 
 * @param data 
 * @param offset 
 * @param num_bytes 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::read_sample_file(uint8_t *data, uint32_t offset, uint32_t num_bytes)
{
    std::string full_file_path;
    uint32_t ret_val = 0;

    if ((data != nullptr) && (num_bytes != 0)){
        full_file_path = _root_path + _local_file_name;
        uint32_t file_len = get_file_len(full_file_path);

        //fflush(stdout);
        FILE* fp = fopen(full_file_path.c_str(), "rb");

        if (fp == nullptr){ // NULL or nullptr ?
            ei_printf("[ERR] The sample file doesn't exist\n");
        }
        else
        {            
            if (file_len >= (offset + num_bytes)){
                //fflush(stdout);
                ret_val = fseek(fp, offset, SEEK_SET);
                if (ret_val == 0){
                    //fflush(stdout);
                    ret_val = fread(data, 1, num_bytes, fp);
                }   
                else{
                    ei_printf("fseek error in file read\n");
                    ret_val = 0;
                }  
            }
            else{
                ei_printf("Offset outside file size\nfile size %ld\noffset %ld\nbytes_to_read %ld\n", file_len, offset, num_bytes);
            }
                   
            /* code */
        }
        //fflush(stdout);
        fclose(fp);        
        
    }
    else
    {
        /* code */        
    }
    
    return ret_val;
}

/**
 * @brief 
 * 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::erase_sample_file(void)
{
    FILE *fp;
    std::string temp;

    temp = _root_path + _local_file_name;
    //fflush(stdout);
    fp = fopen(temp.c_str(), "rb");

    if (fp != NULL) {
        int err_code = 0;
        //fflush(stdout);
        fclose(fp);

        err_code = remove(temp.c_str());

        if (err_code != 0){
            ei_printf("Error in remove file %s\n", temp.c_str());
            ei_printf("Error code %d\n", err_code);
        }
    }
    else{
        //fflush(stdout);
        fclose(fp);
    }
    
}

/**
 * @brief 
 * 
 * @return uint32_t 
 */
uint32_t EiExtFlashMemory::get_file_len(std::string file_name)
{
    FILE *fp;
    uint32_t file_len = 0;

    //fflush(stdout);
    fp = fopen(file_name.c_str(), "rb");

    if (fp != NULL) {
        //fflush(stdout);
        fseek(fp, 0, SEEK_END);
        //fflush(stdout);
        file_len = ftell(fp);
    }
    //fflush(stdout);
    fclose(fp);
    
    return file_len;
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
 */
uint32_t EiExtFlashMemory::get_first_usable_block(void)
{
    uint32_t pos = 0;
    uint32_t temp_pos = 0;

    //printf("spif size: %ld\n",         memory_size);
    //printf("spif block size: %ld\n",   block_size);
    //printf("spif config size: %ld\n",   config_size);
    //printf("spif base address: %ld\n",   base_address);
    //printf("spif read size: %llu\n",    _spif.get_read_size());
    //printf("spif program size: %llu\n", _spif.get_program_size());
    //printf("spif erase size: %llu\n",   _spif.get_erase_size());
    uint32_t erase_size = _spif.get_erase_size();
    uint32_t program_size = _spif.get_program_size();
    uint32_t read_size = _spif.get_read_size();

    DIR *d = opendir("/fs");

    if (d){    
        while (true) {
            struct dirent *e = readdir(d);            
            
            if (!e) {
                break;
            }

            if ((strcmp(e->d_name, ".") != 0) &&
                (strcmp(e->d_name, "..") != 0)
                ){
                    std::string name = std::string("/fs/") + e->d_name;
                    temp_pos = get_file_len(name.c_str());
                    ei_printf("\t%s\n", e->d_name);                    
                    ei_printf("\tsize %ld\n", temp_pos);
            }
            //printf("\t%s\n", e->d_name);                
        }
    }
    else{
        ei_printf("Error in opening /fs \n");
    }
    closedir(d);

    return pos;
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
