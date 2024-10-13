#ifndef OSRMS_API_H
#define OSRMS_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

void os_mount(char* memory_path);

void os_ls_processes();

int os_exists(int process_id, char* file_name);

void os_ls_files(int process_id);

void os_frame_bitmap();

void os_tp_bitmap();

// process functions

void os_start_process(int process_id, char* process_name);

void os_finish_process(int process_id);

// file functions

typedef struct osrmsFile {
    size_t size;
    char* file_name;
    uint32_t address;
} osrmsFile;

osrmsFile* os_open(int process_id, char* file_name, char mode);

int os_read_file(osrmsFile* file_desc, char* dest);

int os_write_file(osrmsFile* file_desc, char* src);

void os_close(osrmsFile* file_desc);

char* global_memory_path;

#endif