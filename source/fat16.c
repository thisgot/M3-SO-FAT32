#include "fat16.h"
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <err.h>

/* Calcula o endereço inicial da FAT */
uint32_t bpb_faddress(struct fat_bpb *bpb)
{
	// Sabendo que área reservada contém [bytes_p_sect] bytes, multiplica por [reserved_sect] para obter o endereço
	// inicial da FAT
	return bpb->reserved_sect * bpb->bytes_p_sect;
}

/* Calcula o endereço do diretório raiz */
uint32_t bpb_froot_addr(struct fat_bpb *bpb)
{
    if (bpb->sect_per_fat_16 != 0) {
        // FAT16
        return bpb_faddress(bpb) + bpb->n_fat * bpb->sect_per_fat_16 * bpb->bytes_p_sect;
    } else {
        // FAT32
        return bpb_fdata_addr(bpb) + (bpb->root_cluster - 2) * bpb->sector_p_clust * bpb->bytes_p_sect;
    }
}


/* Calculo do endereço inicial dos dados */
uint32_t bpb_fdata_addr(struct fat_bpb *bpb)
{
    if (bpb->sect_per_fat_16 != 0) {
        // FAT16
        return bpb_froot_addr(bpb) + bpb->root_entry_count * 32;
    } else {
        // FAT32
        return bpb_faddress(bpb) + bpb->n_fat * bpb->sect_per_fat_32 * bpb->bytes_p_sect;
    }
}

/* Calcula a quantidade de setores/blocos de dados (Um setor contém muitos bytes de um arquivo até um limite) */
uint32_t bpb_fdata_sector_count(struct fat_bpb *bpb)
{
    uint32_t total_sectors = (bpb->total_sectors_16 != 0) ? bpb->total_sectors_16 : bpb->total_sectors_32;
    uint32_t fat_size = (bpb->sect_per_fat_16 != 0) ? bpb->sect_per_fat_16 : bpb->sect_per_fat_32;
    uint32_t data_sectors = total_sectors - (bpb->reserved_sect + (bpb->n_fat * fat_size) + (bpb->root_entry_count * 32 / bpb->bytes_p_sect));
    return data_sectors;
}

/* Calcula a quantidade de setores/blocos de dados (Um setor contém muitos bytes de um arquivo até um limite) */
static uint32_t bpb_fdata_sector_count_s(struct fat_bpb* bpb)
{
    uint32_t total_sectors = (bpb->total_sectors_16 != 0) ? bpb->total_sectors_16 : bpb->total_sectors_32;
    return total_sectors - bpb_fdata_addr(bpb) / bpb->bytes_p_sect;
}

/* Calcula a quantidade de clusters de dados (Um cluster contém um ou mais setores) */
uint32_t bpb_fdata_cluster_count(struct fat_bpb* bpb)
{
    uint32_t data_sectors = bpb_fdata_sector_count_s(bpb);
    return data_sectors / bpb->sector_p_clust;
}

/*
 * allows reading from a specific offset and writting the data to buff
 * returns RB_ERROR if seeking or reading failed and RB_OK if success
 */
int read_bytes(FILE *fp, unsigned int offset, void *buff, unsigned int len)
{

	if (fseek(fp, offset, SEEK_SET) != 0)
	{
		error_at_line(0, errno, __FILE__, __LINE__, "warning: error when seeking to %u", offset);
		return RB_ERROR;
	}
	if (fread(buff, 1, len, fp) != len)
	{
		error_at_line(0, errno, __FILE__, __LINE__, "warning: error reading file");
		return RB_ERROR;
	}

	return RB_OK;
}

/* read fat16's bios parameter block */
void rfat(FILE *fp, struct fat_bpb *bpb)
{

	read_bytes(fp, 0x0, bpb, sizeof(struct fat_bpb));

	return;
}
