#include "mbed.h"

#include "SPIFBlockDevice.h"
#include "LittleFileSystem2.h"
#include "ProfilingBlockDevice.h"

using namespace std::chrono;

#define FILE_COUNT      (100)

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
    
    printf("dr: %6lld dw: %6lld de: %6lld  r: %6lld w: %6lld e: %6lld\n", 
      readCount - readCountOld, writeCount - writeCountOld, eraseCount - eraseCountOld,
      readCount, writeCount, eraseCount);

    readCountOld = readCount;
    writeCountOld = writeCount;
    eraseCountOld = eraseCount;
}

void printDirectory(FileSystem *fs, const char* dirname) {
    Dir dir;
    struct dirent ent;

    dir.open(fs, dirname);
    printf("contents of dir: %s\n", dirname);
    printf("----------------------------------------------------\n");

    while (1) {
        size_t res = dir.read(&ent);
        if (0 == res) {
            break;
        }
        printf(ent.d_name);
        printf("\n");
    }
    dir.close();
}


int main()
{
    printf("Hello from "  MBED_STRINGIFY(TARGET_NAME) "\n");
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    printf("initial counts: ");
    printStatistics();

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
    printf("\ncounts after format and mount: ");
    printStatistics();


    printf("write files\nfileNo  time [ms]\n");
    char fileName[64];
    for (int i=0; i < FILE_COUNT; i++) {
        auto startTime = Kernel::Clock::now(); 

        snprintf(fileName, sizeof(fileName), "/fs/file_%d.txt", i);

        FILE *f = fopen(fileName, "w+");
        err = fprintf(f, "123456789012\n");
        debug_if(err <= 0, "write error: %d\n", err);

        err = fclose(f);
        debug_if(err != 0, "close error: %d\n", err);

        milliseconds elapsed_time = Kernel::Clock::now() - startTime;
        printf("%6d %6d ms   ", i, int(elapsed_time.count()));
        printStatistics();
    }

    printf("\n\n");
    printDirectory(&fs, "/");

    return 0;
}