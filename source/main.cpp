#include "mbed.h"

#include "SPIFBlockDevice.h"
#include "LittleFileSystem2.h"
#include "ProfilingBlockDevice.h"

using namespace std::chrono;

SPIFBlockDevice bd(PA_7, PA_6, PA_5, PA_4);
ProfilingBlockDevice  spif(&bd);
LittleFileSystem2 fs("fs");

void printStatistics()
{
    static bd_size_t readCountOld;
    static bd_size_t writeCountOld;
    static bd_size_t eraseCountOld;
    
    bd_size_t readCount = spif.get_read_count();
    bd_size_t writeCount = spif.get_program_count();
    bd_size_t eraseCount = spif.get_erase_count();
    
    printf("dr: %8lld dw: %8lld de: %8lld  r: %8lld w: %8lld e: %8lld\n", 
      readCountOld - readCount, writeCountOld - writeCount, eraseCountOld - eraseCount,
      readCount, writeCount, eraseCount);

    readCountOld = readCount;
    writeCountOld = writeCount;
    eraseCountOld = eraseCount;
}

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

    // get blockdevice statistics up to now


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
        printf("%8d %8d ms   ", i, int(elapsed_time.count()));
        printStatistics();
    }

    return 0;
}