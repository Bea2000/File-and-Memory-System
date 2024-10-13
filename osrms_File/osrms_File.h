#ifndef OSRMS_FILE_H
#define OSRMS_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// File Table 115B
typedef struct FileTable {
    uint8_t valid;
    char file_name[14];
    uint32_t size;
    uint32_t virtual_address;
} FileTable;

// PCB table entry 256B
typedef struct PCBTable {
    uint8_t state;
    uint8_t process_id;
    char process_name[11];
    FileTable file_tables[5];
    uint16_t first_order_page_tables[64];
} PCBTable;

// Second order page table 128B
typedef struct SecondOrderPageTable {
    uint16_t table[64];
} SecondOrderPageTable;

// Physical Memory 2GB
typedef struct PhysicalMemory {
    uint8_t bytes[65536][32768];
} PhysicalMemory;

typedef struct osrmsMemory {
    PCBTable pcb_tables[32];
    uint8_t page_table_bitmap[128];
    SecondOrderPageTable second_order_page_tables[1024];
    uint8_t frame_bitmap[8192];
    PhysicalMemory physical_memory;
} osrmsMemory;

osrmsMemory* start_osrms_memory();

uint32_t calculate_physical_address(uint32_t virtual_address, osrmsMemory* memory, int pcb_index);

uint32_t find_first_free_virtual_address(osrmsMemory* memory, int pcb_index);

int find_running_process(osrmsMemory* memory);

uint32_t find_next_virtual_address(uint32_t virtual_address, osrmsMemory* memory, int pcb_index);

#endif