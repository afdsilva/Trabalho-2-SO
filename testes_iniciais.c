#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

//http://codeandlife.com/2012/04/02/simple-fat-and-sd-tutorial-part-1/
void print_file_info(Entry *entry);
void dif_fat(FILE * in, int nr_fat_sectors, u_int32_t start_fat1, u_int32_t start_fat2, bool show);
void ajuda();
int main(int argc, char ** argv) {
    
    if (argc == 2) {
		if (argv[1][0] == '-') {
			char * argument = &argv[1][1];
			if (strcmp(argument,"vf") == 0) {
				printf("Veriﬁcar FATs:\n");
			} else if (strcmp(argument,"bl") == 0) {
				printf("Imprimir lista de blocos livres\n");
			} else if (strcmp(argument,"bd") == 0) {
				printf("Imprimir lista de blocos livres com dados\n");
			} else if (strcmp(argument,"cf1") == 0) {
				printf("Corrigir a primeira cópia da FAT\n");
			} else if (strcmp(argument,"cf2") == 0) {
				printf("Corrigir a segunda cópia da FAT\n");
			} else {
				printf("Argumento desconhecido!\n");
				ajuda();
			}
			//if (argument == '')
			//
		}
		
	}
	
	return 0;
    
    FILE * in = fopen("disco", "rb");
    int i;
    Entry entry;
    u_int32_t start_fat1, start_fat2, root_start;
    u_short cluster = 2;
    fseek(in, 0x0, SEEK_SET);
    BootSector bs;
    fread(&bs, sizeof(BootSector), 1, in);
    u_int32_t total_blocks = (bs.total_sectors_short != 0 ? bs.total_sectors_short : bs.total_sectors_long );
    printf("Total blocks: %d\n", total_blocks);
    start_fat1 = sizeof(BootSector) + (bs.reserved_sectors-1) * bs.sector_size;
    start_fat2 = sizeof(BootSector) + start_fat1 + (bs.fat_size_sectors - 1) * bs.sector_size;
    root_start = sizeof(BootSector) + start_fat2 + (bs.fat_size_sectors - 1) * bs.sector_size;
    printf("fat1: 0x%X, fat2: 0x%X, root: 0x%X\n", start_fat1, start_fat2, root_start);

    int nr_fat_sectors = bs.sector_size * 2;
    
    dif_fat(in,nr_fat_sectors, start_fat1, start_fat2,false);
    printf("Nr sectors: %d Fat size: %d , sectors_per_cluster: %d\n", nr_fat_sectors, bs.fat_size_sectors, bs.sectors_per_cluster);

    fclose(in);
    return 0;
}

void print_file_info(Entry *entry) {
    switch(entry->filename[0]) {
    case 0x00:
        return; // unused entry
    case 0xE5:
        printf("Deleted file: [?%.7s.%.3s]\n", entry->filename+1, entry->ext);
        return;
    case 0x05:
        printf("File starting with 0xE5: [%c%.7s.%.3s]\n", 0xE5, entry->filename+1, entry->ext);
        break;
    case 0x2E:
        printf("Directory: [%.8s.%.3s]\n", entry->filename, entry->ext);
        break;
    default:
        printf("File: [%.8s.%.3s]\n", entry->filename, entry->ext);
    }

    printf("  Modified: %04d-%02d-%02d %02d:%02d.%02d    Start: [%04X]    Size: %d\n",
        1980 + (entry->modify_date >> 9), (entry->modify_date >> 5) & 0xF, entry->modify_date & 0x1F,
        (entry->modify_time >> 11), (entry->modify_time >> 5) & 0x3F, entry->modify_time & 0x1F,
        entry->starting_cluster, entry->file_size);
}

void dif_fat(FILE * in, int nr_fat_sectors, u_int32_t start_fat1, u_int32_t start_fat2, bool show) {
    u_short fat1[nr_fat_sectors];
    fseek(in, start_fat1 + 4, SEEK_SET);
    fread(&fat1, sizeof(fat1), 1, in);
    
    u_short fat2[nr_fat_sectors];
    fseek(in, start_fat2 + 4, SEEK_SET);
    fread(&fat2, sizeof(fat2), 1, in);
    int i = 0;
    for (i = 0; i < nr_fat_sectors; i++) {
		if (fat1[i] != fat2[i]) {
			printf("DIF %d:%ud,%ud\n",i,fat1[i],fat2[i]);
		}
	}
	if (show) {
		printf("FAT:\n");
		for (i = 0; i < nr_fat_sectors; i++) {
			printf("%d:%ud,%ud\n",i,fat1[i],fat2[i]);
		}
	}
	
}
void ajuda() {
	printf("Ajuda: \n");
	printf("	-vf: Veriﬁcar se as duas FATs são iguais – caso não sejam, imprime uma lista de diferenças no seguinte formato, com uma linha para cada diferença.\n");
	printf("	-bl: Imprime os índices de todos os blocos que estão livres (ou seja, não são apontados pela FAT) em uma única linha.\n");
	printf("	-bd: Imprime os índices de todos os blocos que estão livres e que tem conteúdo diferente de zeros, em uma única linha.\n");
	printf("	-cf1: Copia o conteúdo da segunda cópida da FAT na primeira cópia.\n");
	printf("	-cf2: Copia o conteúdo da primeira cópida da FAT na segunda cópia.\n");
}
