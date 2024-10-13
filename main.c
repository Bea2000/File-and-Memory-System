#include "osrms_API/osrms_API.h"
#include "osrms_File/osrms_File.h"

int main() {
    os_mount("mem_filled.bin");
    os_ls_processes();
    os_ls_files(117);
    return 0;
}
