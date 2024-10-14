#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "osrms_API.h"
#include "../osrms_File/osrms_File.h"

char* global_memory_path = NULL;
osrmsMemory* global_memory;

void os_mount(char* memory_path){
    printf("----------------------------------------\n");
    printf("Mounting memory at %s\n", memory_path);
    global_memory_path = strdup(memory_path);
    if (global_memory_path == NULL) {
        perror("Failed to allocate memory for global_memory_path");
        exit(EXIT_FAILURE);
    }
    printf("Mounted memory at %s\n", global_memory_path);

    global_memory = start_osrms_memory();
    printf("----------------------------------------\n");
}

void os_ls_processes(){
    printf("----------------------------------------\n");
    printf("Listing processes...\n");
    int counter = 0;

    for (int i = 0; i < 32; i++) {
        if (global_memory->pcb_tables[i].state == 1) {
            printf("Process ID: %d\n", global_memory->pcb_tables[i].process_id);
            printf("Process Name: %s\n", global_memory->pcb_tables[i].process_name);
            counter++;
        }
    }

    if (counter == 0) {
        printf("No processes found.\n");
    }
    printf("----------------------------------------\n");
}

int os_exists(int process_id, char* file_name){
    printf("----------------------------------------\n");
    printf("Checking if file exists...\n");
    
    for (int i = 0; i < 32; i++) {
        if (global_memory->pcb_tables[i].process_id == process_id) {
            for (int j = 0; j < 32; j++) {
                if (global_memory->pcb_tables[i].file_tables[j].valid == 1) {
                    if (strcmp(global_memory->pcb_tables[i].file_tables[j].file_name, file_name) == 0) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void os_ls_files(int process_id) {
    printf("----------------------------------------\n");
    printf("Listing files...\n");
    int counter = 0;
    
    for (int i = 0; i < 32; i++) {
        if (global_memory->pcb_tables[i].process_id == process_id) {
            for (int j = 0; j < 32; j++) {
                if (global_memory->pcb_tables[i].file_tables[j].valid == 1) {
                    printf("File Name: %s\n", global_memory->pcb_tables[i].file_tables[j].file_name);
                    counter++;
                }
            }
        }
    }

    if (counter == 0) {
        printf("No files found.\n");
    }
    printf("----------------------------------------\n");
}

void os_frame_bitmap() {
    int counter_ocuppied = 0;
    int counter_free = 0;
    printf("----------------------------------------\n");
    printf("Frame Bitmap...\n");

    for (int byteIndex = 0; byteIndex < 8192; byteIndex++) {
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            if (global_memory->frame_bitmap[byteIndex] & (1 << bitIndex)) {
                counter_ocuppied++;
                printf("1");
            } else {
                counter_free++;
                printf("0");
            }
        }
    }

    printf("\nOccupied frames: %d\n", counter_ocuppied);
    printf("Free frames: %d\n", counter_free);
    printf("\n");
    printf("----------------------------------------\n");
}

void os_tp_bitmap() {
    int counter_ocuppied = 0;
    int counter_free = 0;
    printf("----------------------------------------\n");
    printf("Page Table Bitmap...\n");

    for (int byteIndex = 0; byteIndex < 128; byteIndex++) {
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            if (global_memory->page_table_bitmap[byteIndex] & (1 << bitIndex)) {
                counter_ocuppied++;
                printf("1");
            } else {
                counter_free++;
                printf("0");
            }
            }
    }

    printf("\nOccupied page tables: %d\n", counter_ocuppied);
    printf("Free page tables: %d\n", counter_free);
    printf("\n");
    printf("----------------------------------------\n");
}

void os_start_process(int process_id, char* process_name){
    printf("----------------------------------------\n");
    printf("Starting process...\n");
    for (int i = 0; i < 32; i++) {
        if (global_memory->pcb_tables[i].state == 1 & process_id == global_memory->pcb_tables[i].process_id) {
            printf("Process Id already exists.\n");
            printf("----------------------------------------\n");
            return;
        }
    }
    for (int i = 0; i < 32; i++) {
        if (global_memory->pcb_tables[i].state == 0) {
            global_memory->pcb_tables[i].state = 1;
            global_memory->pcb_tables[i].process_id = process_id;
            strcpy(global_memory->pcb_tables[i].process_name, process_name);
            printf("Process started successfully.\n");
            printf("----------------------------------------\n");
            return;
        }
    }
    printf("No available process slots.\n");
    printf("----------------------------------------\n");
}

void os_finish_process(int process_id){
    printf("----------------------------------------\n");
    printf("Freeing process...\n");
    for (int i = 0; i < 32; i++) {
        if (global_memory->pcb_tables[i].state == 1) {
            if (global_memory->pcb_tables[i].process_id == process_id) {
                global_memory->pcb_tables[i].state = 0;

                for (int j = 0; j < 5; j++) {
                    if (global_memory->pcb_tables[i].file_tables[j].valid == 1) {
                        uint32_t address = calculate_physical_address(global_memory->pcb_tables[i].file_tables[j].virtual_address, global_memory, i);
                        uint32_t frame_number = address / 32768;
                        uint32_t byte_index = frame_number / 8;
                        uint8_t bit_position = frame_number % 8;
                        uint8_t byte_value = global_memory->frame_bitmap[byte_index];
                        uint8_t frame_status = (byte_value >> bit_position) & 1;

                        if (frame_status == 1) {
                            global_memory->frame_bitmap[byte_index] &= ~(1 << bit_position);
                        }
                        global_memory->pcb_tables[i].file_tables[j].valid = 0;
                    }
                }
                printf("Process freed successfully.\n");
                printf("----------------------------------------\n");
                return;
            }
        }
    }
  
    printf("Process not found.\n");
    printf("----------------------------------------\n");
}

osrmsFile* os_open(int process_id, char* file_name, char mode){
    if (mode == 'r' && os_exists(process_id, file_name) == 0) {
        printf("File not found.\n");
        return NULL;
    } else if (mode == 'w' && os_exists(process_id, file_name) == 1) {
        printf("File already exists.\n");
        return NULL;
    } else if (mode == 'r') {
        for (int i = 0; i<32; i++){
            if (global_memory->pcb_tables[i].process_id == process_id) {
                for (int j = 0; j < 5; j++) {
                    if (global_memory->pcb_tables[i].file_tables[j].valid == 1 && strcmp(global_memory->pcb_tables[i].file_tables[j].file_name, file_name) == 0) {
                        uint32_t virtual_address = global_memory->pcb_tables[i].file_tables[j].virtual_address;
                        uint32_t physical_address = -1;

                        if (global_memory->pcb_tables[i].file_tables[j].size > 0){
                            physical_address = calculate_physical_address(virtual_address, global_memory, process_id);
                        } else {
                            printf("File is empty.\n");
                            return NULL;
                        }
                        
                        osrmsFile* file = (osrmsFile*) malloc(sizeof(osrmsFile));
                        file->file_name = strdup(file_name);
                        file->size = global_memory->pcb_tables[i].file_tables[j].size;
                        file->address = physical_address;

                        printf("File opened successfully.\n");
                        
                        return file;
                    }
                }
            }
        }
    } else if (mode == 'w') {
        for (int i = 0; i<32; i++){
            if (global_memory->pcb_tables[i].process_id == process_id) {
                for (int j = 0; j < 5; j++) {
                    if (global_memory->pcb_tables[i].file_tables[j].valid == 0) {
                        global_memory->pcb_tables[i].file_tables[j].valid = 1;
                        strcpy(global_memory->pcb_tables[i].file_tables[j].file_name, file_name);
                        global_memory->pcb_tables[i].file_tables[j].size = 0;
                        global_memory->pcb_tables[i].file_tables[j].virtual_address = 0;
                        
                        osrmsFile* file = (osrmsFile*) malloc(sizeof(osrmsFile));
                        file->file_name = strdup(file_name);
                        file->size = 0;
                        file->address = -1;

                        printf("File opened successfully.\n");
                        
                        return file;
                    }
                }
            }
        }
    } else {
        printf("Invalid mode. Please use 'r' for read or 'w' for write.\n");
        return NULL;
    }

    return NULL;
}

int os_read_file(osrmsFile* file_desc, char* dest) {
    FILE* dest_file = fopen(dest, "wb");
    if (!dest_file) {
        perror("Error opening destination file");
        return -1;
    }

    uint32_t file_size = file_desc->size;

    uint32_t bytes_read = 0;
    uint8_t buffer[32768];

    while (bytes_read < file_size) {
        uint32_t physical_address = file_desc->address;

        uint32_t bytes_to_read = (file_size - bytes_read < 32768) ? (file_size - bytes_read) : 32768;
        memcpy(buffer, global_memory->physical_memory[physical_address / 32768], bytes_to_read);
        fwrite(buffer, 1, bytes_to_read, dest_file);

        bytes_read += bytes_to_read;
        physical_address += bytes_to_read;
    }
    fclose(dest_file);
    return bytes_read;
}

int os_write_file(osrmsFile* file_desc, char* src) {
    FILE* src_file = fopen(src, "rb");
    if (!src_file) {
        perror("Error opening source file");
        return -1;
    }
    
    int pcb_index = find_running_process(global_memory);
    uint32_t virtual_address = find_first_free_virtual_address(global_memory, pcb_index);
    if (virtual_address == (uint32_t)-1) {
        fclose(src_file);
        return -1;
    }

    uint32_t bytes_written = 0;
    uint8_t buffer[32768];
    size_t bytes_to_write;
    size_t total_file_size = 0;

    while ((bytes_to_write = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        total_file_size += bytes_to_write;

        uint32_t physical_address = calculate_physical_address(virtual_address, global_memory, pcb_index);
        if (physical_address == (uint32_t)-1) {
            fclose(src_file);
            return -1;
        }

        uint32_t frame_offset = virtual_address % 32768;
        uint32_t available_space = 32768 - frame_offset;

        uint32_t bytes_to_copy = (bytes_to_write < available_space) ? bytes_to_write : available_space;
        memcpy(global_memory->physical_memory[physical_address / 32768] + frame_offset, buffer, bytes_to_copy);

        bytes_written += bytes_to_copy;
        virtual_address += bytes_to_copy;
        bytes_to_write -= bytes_to_copy;

        while (bytes_to_write > 0) {
            virtual_address = find_next_virtual_address(virtual_address, global_memory, pcb_index);
            if (virtual_address == (uint32_t)-1) {
                fclose(src_file);
                return -1;
            }

            physical_address = calculate_physical_address(virtual_address, global_memory, pcb_index);
            if (physical_address == (uint32_t)-1) {
                fclose(src_file);
                return -1;
            }

            bytes_to_copy = (bytes_to_write < 32768) ? bytes_to_write : 32768;
            memcpy(global_memory->physical_memory[physical_address / 32768], buffer + bytes_written, bytes_to_copy);
            bytes_written += bytes_to_copy;
            virtual_address += bytes_to_copy;
            bytes_to_write -= bytes_to_copy;
        }
    }

    fclose(src_file);

    file_desc->size = total_file_size;
    file_desc->address = virtual_address;

    return bytes_written;
}

void os_close(osrmsFile* file_desc) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 5; j++) {
            if (global_memory->pcb_tables[i].file_tables[j].valid == 1 && global_memory->pcb_tables[i].file_tables[j].file_name == file_desc->file_name) {
                global_memory->pcb_tables[i].file_tables[j].valid = 0;
                free(file_desc->file_name);
                free(file_desc);
                return;
            }
        }
    }

    printf("File not found.\n");
    return;
}