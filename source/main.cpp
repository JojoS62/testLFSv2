#include "mbed.h"

#include "SPIFBlockDevice.h"
#include "littlefsv2/LittleFileSystem2.h"
//#include "storage/filesystem/littlefs/LittleFileSystem.h"

using namespace std::chrono;

SPIFBlockDevice spif(PA_7, PA_6, PA_5, PA_4);
LittleFileSystem2 fs("fs");

int main()
{
    printf("Hello from "  MBED_STRINGIFY(TARGET_NAME) "\n");
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    spif.init();
    printf("spif size: %llu\n",         spif.size());
    printf("spif read size: %llu\n",    spif.get_read_size());
    printf("spif program size: %llu\n", spif.get_program_size());
    printf("spif erase size: %llu\n",   spif.get_erase_size());

    int err = 0;

    err = fs.format(&spif);
    debug_if(err != 0, "format error: %d\n", err);

    err = fs.mount(&spif);
    debug_if(err != 0, "mount error: %d\n", err);

    printf("write file  fileNo  time [ms]\n");
    char fileName[64];
    for (int i=0; i < 1000; i++) {
        auto startTime = Kernel::Clock::now(); 

        snprintf(fileName, sizeof(fileName), "/fs/file_%d.txt", i);

        FILE *f = fopen(fileName, "w+");
        err = fprintf(f, "123456789012\n");
        debug_if(err <= 0, "write error: %d\n", err);

        err = fclose(f);
        debug_if(err != 0, "close error: %d\n", err);

        milliseconds elapsed_time = Kernel::Clock::now() - startTime;
        printf("%8d %8d ms\n", i, int(elapsed_time.count()));
    }

    return 0;
}