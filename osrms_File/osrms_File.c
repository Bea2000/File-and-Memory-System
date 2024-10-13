#include "osrms_File.h"
#include "../osrms_API/osrms_API.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

osrmsMemory* start_osrms_memory() {
    FILE* file = fopen(global_memory_path, "rb");
    if (!file) {
        perror("Error opening memory file");
        exit(EXIT_FAILURE);
    }

    osrmsMemory* memory = (osrmsMemory*) calloc(1, sizeof(osrmsMemory));

    fread(memory->pcb_tables, sizeof(PCBTable), 32, file);

    fread(&memory->page_table_bitmap, sizeof(uint8_t), 1, file);

    fread(memory->second_order_page_tables, sizeof(SecondOrderPageTable), 1024, file);

    fread(&memory->frame_bitmap, sizeof(uint8_t), 8192, file);

    fread(memory->physical_memory.bytes, sizeof(uint8_t), (size_t)65536 * 32768, file);

    fclose(file);
    printf("Memory initialized\n");

    return memory;
}

uint32_t calculate_physical_address(uint32_t virtual_address, osrmsMemory* memory, int pcb_index) {
    uint32_t vpn = (virtual_address >> 15) & 0xFFF;
    uint32_t offset = virtual_address & 0x7FFF;
    uint32_t first_order_index = (vpn >> 6) & 0x3F;
    uint16_t second_order_page_table_num = memory->pcb_tables[pcb_index].first_order_page_tables[first_order_index];
    uint32_t second_order_index = vpn & 0x3F;
    uint16_t pfn = memory->second_order_page_tables[second_order_page_table_num].table[second_order_index];
    uint32_t physical_address = (pfn << 15) | offset;

    return physical_address;
}

uint32_t find_first_free_virtual_address(osrmsMemory* memory, int pcb_index) {
    PCBTable* pcb = &memory->pcb_tables[pcb_index];

    for (int i = 0; i < 64; i++) {
        uint16_t second_order_page_table_index = pcb->first_order_page_tables[i];

        if (second_order_page_table_index != 0) {
            SecondOrderPageTable* second_order_table = &memory->second_order_page_tables[second_order_page_table_index];

            for (int j = 0; j < 64; j++) {
                if (second_order_table->table[j] == 0) {
                    uint32_t virtual_address = (i << 12) | (j << 6);
                    return virtual_address;
                }
            }
        }
    }

    return (uint32_t)-1;
}

int find_running_process(osrmsMemory* memory) {
    for (int i = 0; i < 32; i++) {
        if (memory->pcb_tables[i].state == 1) {
            return i;
        }
    }

    return -1;
}

uint32_t find_next_virtual_address(uint32_t virtual_address, osrmsMemory* memory, int pcb_index) {
    uint32_t vpn = (virtual_address >> 15) & 0xFFF;
    uint32_t offset = virtual_address & 0x7FFF;
    uint32_t first_order_index = (vpn >> 6) & 0x3F;
    uint16_t second_order_page_table_num = memory->pcb_tables[pcb_index].first_order_page_tables[first_order_index];
    uint32_t second_order_index = vpn & 0x3F;
    uint16_t pfn = memory->second_order_page_tables[second_order_page_table_num].table[second_order_index];

    uint32_t next_virtual_address = (pfn << 15) | (offset + 1);
    return next_virtual_address;
}