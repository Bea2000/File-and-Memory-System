#include "osrms_API/osrms_API.h"
#include "osrms_File/osrms_File.h"

int main() {
    os_mount("mem_filled.bin");
    os_ls_processes();
    os_ls_files(117);
    if (os_exists(117, "dino.jpg") == 1){
        printf("dino.jpg existe\n");
    } else {
        printf("dino.jpg no existe\n");
    }
    if (os_exists(117, "aaa.txt") == 1){
        printf("aaa.txt existe\n");
    } else {
        printf("aaa.txt no existe\n");
    }
    os_start_process(1, "Holi");
    os_ls_processes();
    os_finish_process(1);
    os_ls_processes();
    return 0;
}
