#include <x86/idt.h>
#include <x86/pci.h>
#include <types.h>
#include <drivers/driverManager.h>
#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/timer.h>
#include <mem/pmm.h>
#include <mem/vmm/vmm.h>

#include <qemulog.h>

#define MEMORY_MAP_REGION 0xA00

typedef struct {
    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
} memory_map_t;

typedef struct {
    uint16_t memory_map_size;
    uint16_t kernel_size;
} mem_desc_t;

void stage3(mem_desc_t *mem_desc) {
    asm volatile("cli");
    log("aStage 3 started\n");
    clean_screen();
    printf("HERE");
    idt_setup();
    asm volatile("sti");
    // init_timer();

    printf("Kernel size: "); printd(mem_desc->kernel_size);
    printf(" KB\nMemory map:\n");
    uint32_t ram_size = 0;
    memory_map_t *memory_map = (memory_map_t *)MEMORY_MAP_REGION;
    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            ram_size = memory_map[i].startLo + memory_map[i].sizeLo;
        }
    }

    printh(ram_size);

    pmm_init(0x100000, mem_desc->kernel_size, ram_size);

    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        printh(memory_map[i].startLo); printf(" ");
        printd(memory_map[i].sizeLo); printf(" ");
        printh(memory_map[i].type); printf(" ");
        if (memory_map[i].type == 1) {
            pmm_init_region(memory_map[i].startLo, memory_map[i].sizeLo);
        }
        printf("\n");
    }

    pmm_deinit_mat(); // mat deinit
    pmm_deinit_region(0x0, 0x100000); // kernel stack deinit
    pmm_deinit_region(0x100000, mem_desc->kernel_size * 1024); // kernel deinit

    if (vmm_init()) {
        printf("\n\nVM Remapped\n\n");
    }
    
    uint8_t* kek1 = pmm_alloc_block();
    vmm_map_page(kek1, 0x50000000);
    printh(kek1);
    uint8_t* kekt = 0x50000000;
    kekt[0] = 0;
    kekt[1] = 1;
    printd(kekt[1]);
    //
    // // testing PMM
    //
    // // printf("1st pointer: ");
    // // printh(kek1);
    // // printf("\n");
    // // pmm_free_block(kek1);
    // printf("Deleting pointer 1\n");
    // void* kek2 = pmm_alloc_block();
    // void* kekBig = pmm_alloc_blocks(5);
    // void* kek3 = pmm_alloc_block();
    //
    // printf("2nd pointer: ");
    // printh(kek2);
    // printf("\n");
    // printf("Big pointer: ");
    // printh(kekBig);
    // printf("\n");
    // printf("3rd pointer: ");
    // printh(kek3);
    // printf("\n");
    //
    // pmm_free_blocks(kekBig, 5);
    // void* kek4 = pmm_alloc_blocks(3);
    // void* kek5 = pmm_alloc_blocks(4);
    //
    // printf("4th pointer: ");
    // printh(kek4);
    // printf("\n");
    // printf("5th pointer: ");
    // printh(kek5);
    // printf("\n");
    //
    register_drivers();
    start_all_drivers();
    find_pci_devices();

    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    ata_write(&ata0m, "test", 4);
    ata_flush(&ata0m);
    ata_read(&ata0m);

    asm volatile("int $0"); // test interrupts

    while (1) {}
}
