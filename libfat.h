/*
 * libfat.h
 *
 */

#ifndef LIBFAT_H_
#define LIBFAT_H_

#include <stdint.h>

typedef struct {
    u_char jmp[3];
    char oem[8];
    u_short sector_size;
    u_char sectors_per_cluster;
    u_short reserved_sectors;
    u_char number_of_fats;
    u_short root_dir_entries;
    u_short total_sectors_short;
    u_char media_descriptor;
    u_short fat_size_sectors;
    u_short sectors_per_track;
    u_short number_of_heads;
    u_int32_t hidden_sectors;
    u_int32_t total_sectors_long;

    u_char drive_number;
    u_char current_head;
    u_char boot_signature;
    u_int32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    u_short boot_sector_signature;
}  __attribute((packed)) BootSector;

typedef struct {
    u_char filename[8];
    u_char ext[3];
    u_char attributes;
    u_char reserved[10];
    u_short modify_time;
    u_short modify_date;
    u_short starting_cluster;
    u_int32_t file_size;
} __attribute((packed)) Entry;

#endif /* LIBFAT_H_ */
