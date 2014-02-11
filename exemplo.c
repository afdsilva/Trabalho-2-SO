#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "libfat.h"

void print_file_info(Entry *entry);
void dif_fat(FILE * in, BootSector bs, bool show);
void blocos_livres(FILE * in, BootSector bs);
void blocos_deletados(FILE * in, BootSector bs);

void copia_fat(FILE * in, FILE * out,int fat_in, int fat_out);
//void seek_dir(FILE * in,
void ajuda();
FILE * init(const char * fileName);
BootSector ReadBootSector(FILE * in);

int main(int argc, char ** argv) {
    
    if (argc >= 2) {
		
		FILE * fp = init("disco");
		
		if (!fp) {
			printf("Nao foi possivel abrir o disco virtual, verifique se existe ou crie-o:\n");
			printf("dd if=/dev/zero of=disco bs=1M count=2\n");
			printf("mkfs.vfat -F16 disco\n");
			return -1;
		}
		
	    BootSector bs = ReadBootSector(fp);
		
		if (argv[1][0] == '-') {
			char * argument = &argv[1][1];
			if (strcmp(argument,"vf") == 0) {
				printf("Veriﬁcar FATs:\n");
				dif_fat(fp,bs,false);
			} else if (strcmp(argument,"bl") == 0) {
				printf("Imprimir lista de blocos livres\n");
			} else if (strcmp(argument,"bd") == 0) {
				printf("Imprimir lista de blocos livres com dados\n");
				blocos_deletados(fp,bs);
			} else if (strcmp(argument,"cf1") == 0) {
				printf("Corrigir a primeira cópia da FAT\n");
			} else if (strcmp(argument,"cf2") == 0) {
				printf("Corrigir a segunda cópia da FAT\n");
			} else if (strcmp(argument,"cdf1") == 0) {
				/**
				 * Copia a fat1 do disco 2 para fat1 do disco 2 para gerar caso de testes
				 **/
				FILE * fp2 = init("disco_2");
			} else if (strcmp(argument,"cdf2") == 0) {
				/**
				 * Copia a fat1 do disco 2 para fat2 do disco 1 para gerar caso de testes
				 **/
			} else {
				printf("Argumento desconhecido!\n");
				ajuda();
			}
		}
		fclose(fp);
	} else {
		ajuda();
	}
	
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
        printf("File: [%.8s.%.3s][offset:0x%04X]\n", entry->filename, entry->ext,entry->starting_cluster);
    }

    printf("  Modified: %04d-%02d-%02d %02d:%02d.%02d    Start: [%04X]    Size: %d\n",
        1980 + (entry->modify_date >> 9), (entry->modify_date >> 5) & 0xF, entry->modify_date & 0x1F,
        (entry->modify_time >> 11), (entry->modify_time >> 5) & 0x3F, entry->modify_time & 0x1F,
        entry->starting_cluster, entry->file_size);
}
void dif_fat(FILE * in, BootSector bs, bool show) {
	
	int nr_fat_sectors = (bs.fat_size_sectors * bs.sector_size / 2) - 2;
	u_int32_t start_fat1 = sizeof(BootSector) + (bs.reserved_sectors-1) * bs.sector_size;;
	u_int32_t start_fat2 = sizeof(BootSector) + start_fat1 + (bs.fat_size_sectors - 1) * bs.sector_size;
	u_int32_t root_start = sizeof(BootSector) + start_fat2 + (bs.fat_size_sectors - 1) * bs.sector_size;
	int sizeStarts = start_fat2 - start_fat1 - 2;
	int sizeRootF2 = root_start - start_fat2;
    u_short fat1[nr_fat_sectors];
    u_short fat2[nr_fat_sectors];
    int sizeOf = sizeof(fat1);
    
    //printf("sizeStarts: %d sizeOf: %d sizeRootF2: %d\n",sizeStarts, sizeOf, sizeRootF2);
    //printf("Nr setores: %d \nPosicao inicial fat1: 0x%X\n", nr_fat_sectors, start_fat1);
    fseek(in, start_fat1 + 4, SEEK_SET);
    fread(&fat1, sizeof(fat1), 1, in);
    //printf("Posicao final fat1: 0x%lX\n", ftell(in));
    
    //printf("Posicao inicial fat2: 0x%X\n", start_fat2);
    fseek(in, start_fat2 + 4, SEEK_SET);
    fread(&fat2, sizeof(fat2), 1, in);
    //printf("Posicao final fat2: 0x%lX\n root: 0x%X\n", ftell(in),root_start);
    
    int i = 0;
    int count = 0;
    for (i = 0; i < nr_fat_sectors; i++) {
		if (fat1[i] != fat2[i]) {
			count++;
			printf("DIF %d:%ud,%ud\n",i,fat1[i],fat2[i]);
		}
	}
	if (count == 0) 
		printf("nenhuma diferenca entre as FATs\n");
	if (show) {
		printf("FAT:\n");
		for (i = 0; i < nr_fat_sectors; i++) {
			printf("%d:%ud,%ud\n",i,fat1[i],fat2[i]);
		}
	}
}
void blocos_livres(FILE * in, BootSector bs) {
	
}
void blocos_deletados(FILE * in, BootSector bs) {

	int nr_fat_sectors = (bs.fat_size_sectors * bs.sector_size / 2) - 2;
    int i, j;
	u_int32_t start_fat1 = sizeof(BootSector) + (bs.reserved_sectors-1) * bs.sector_size;;
	u_int32_t start_fat2 = sizeof(BootSector) + start_fat1 + (bs.fat_size_sectors - 1) * bs.sector_size;
	u_int32_t root_start = start_fat1 + bs.fat_size_sectors * bs.number_of_fats * bs.sector_size;
	u_int32_t data_start = root_start + (sizeof(Entry) * (bs.root_dir_entries));
    u_short fat1[nr_fat_sectors];
    u_int32_t cluster_size = bs.sectors_per_cluster * bs.sector_size;
	u_int32_t cluster[bs.sector_size];
	u_int32_t pos = 0;
    
    fseek(in, data_start, SEEK_SET);
	int count = 2;
	while(!feof(in)) {
		pos = data_start + (count - 2 ) * cluster_size;
		fread(&cluster, cluster_size, 1, in);
		//if (cluster[0] != 0)
		//	printf("Endereco Utilizadas: 0x%04X Pos: %d\n", pos, count);
		count++;
	}
		
	//printf("Nr clusters: %d\n", count);
    fseek(in, start_fat1, SEEK_SET);
    fread(&fat1, sizeof(fat1), 1, in);
    printf("REMOVIDOS ");
    int c = 0;
    int achou = 0;
	for (i = 2; i < count; i++) {
		fseek(in, data_start + (i - 2 ) * cluster_size, SEEK_SET);
		fread(&cluster, cluster_size, 1, in);
		if (cluster[0] != 0) {
			achou = 0;
			for (j = 0; j < nr_fat_sectors; j++) {
				if (fat1[j] != 0 && fat1[j] != 0xFFFF) {
					if ((fat1[j] - 2) == i) {
						achou = 1;
					}
				}
			}
			if (!achou) {
				if (c > 0)
					printf(",");
				printf("%d",i);
				c++;
			}
		}
	}
	printf("\n");
}
void copia_fat(FILE * in, FILE * out,int fat_in, int fat_out) {
	
}


void ajuda() {
	printf("Ajuda: \n");
	printf("	-vf: Veriﬁcar se as duas FATs são iguais – caso não sejam, imprime uma lista de diferenças no seguinte formato, com uma linha para cada diferença.\n");
	printf("	-bl: Imprime os índices de todos os blocos que estão livres (ou seja, não são apontados pela FAT) em uma única linha.\n");
	printf("	-bd: Imprime os índices de todos os blocos que estão livres e que tem conteúdo diferente de zeros, em uma única linha.\n");
	printf("	-cf1: Copia o conteúdo da segunda cópida da FAT na primeira cópia.\n");
	printf("	-cf2: Copia o conteúdo da primeira cópida da FAT na segunda cópia.\n");
}

FILE * init(const char * fileName) {
    FILE * fp = fopen(fileName, "rb");
    return fp;
}

BootSector ReadBootSector(FILE * in) {
	BootSector retorno;
    fread(&retorno, sizeof(BootSector), 1, in);
	return retorno;
	
}
