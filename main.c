#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <dirent.h>

typedef uint8_t		u8;
typedef uint16_t 	u16;
typedef uint32_t	u32;
typedef uint64_t 	u64;

#define swap16(x) ((((u16)(x))>>8) | ((x) << 8))
#define swap32(x) ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))

static uint64_t swap64(uint64_t data)
{
	uint64_t ret = (data << 56) & 0xff00000000000000ULL;
	ret |= ((data << 40) & 0x00ff000000000000ULL);
	ret |= ((data << 24) & 0x0000ff0000000000ULL);
	ret |= ((data << 8) & 0x000000ff00000000ULL);
	ret |= ((data >> 8) & 0x00000000ff000000ULL);
	ret |= ((data >> 24) & 0x0000000000ff0000ULL);
	ret |= ((data >> 40) & 0x000000000000ff00ULL);
	ret |= ((data >> 56) & 0x00000000000000ffULL);
	return ret;
}

int main()
{	
	int n, i, j, k;
	char entry[255];
	char game_ID[20];
	char Title[255];
	u64 hash;
	char temp[255];
	u64 val64;
	u32 val32;
	u16 val16;
	u8 val8;
	
	u32 cmd_count;
	u64 cmd_offset;
	u32 cmd_id;
	
	FILE *elf;
	elf = fopen("ps2_gxemu.elf", "rb");
	if(elf==NULL) {
		printf("ps2_gxemu.elf not found");
		return;
	}
	fseek (elf , 0 , SEEK_END);
	u32 elf_size = ftell (elf);
	fseek(elf, 0, SEEK_SET);
	u8 *elf_data = (char *) malloc(elf_size);	
	fread(elf_data, elf_size, 1, elf);
	fclose(elf);
		
	FILE *ft;
	ft = fopen("get_title/result.txt", "rb");
	if(ft==NULL) {
		printf("result.txt not found");
		return;
	}
	
	system("rm -r files");
	mkdir("files");
	
	FILE *fo;
	fo = fopen("log.txt", "wb");
	
	while(fgets(entry, 255, ft)) {
		sscanf(entry, "%s %llX", game_ID, (unsigned long long int *) &hash);
		
		sprintf(temp, "Title : %s", &strchr(entry, '\t')[1]); fputs(temp, fo);
		sprintf(temp, "Game ID : %s\n", game_ID); fputs(temp, fo);
		sprintf(temp, "Hash : 0x%llX\n", hash); fputs(temp, fo);
		
		hash = swap64(hash);
		
		for(n=0; n<elf_size; n++) {
			if(!memcmp(&elf_data[n], &hash, 8)) {
			
				FILE *cfg;
				char config_file[64];
				char config_file_tofix[64];
				
				sprintf(config_file, "files/%s.CONFIG", game_ID);
				sprintf(config_file_tofix, "files/[tofix]%s.CONFIG", game_ID);
				
				u8 tofix=0;
				
				cfg = fopen(config_file, "wb");
				
				val32 = 0x3D;
				fwrite(&val32, 1, sizeof(u32), cfg);
		
				val32 = 0x4457;
				fwrite(&val32, 1, sizeof(u32), cfg);
				
				sprintf(temp, "Hash Offset : %X\n", n-0x10000); fputs(temp, fo);
				
				memcpy(&cmd_offset, &elf_data[n+8], sizeof(u64));
				cmd_offset = swap64(cmd_offset);
				sprintf(temp, "Commands offset : %llX\n", cmd_offset); fputs(temp, fo);
				
				memcpy(&cmd_count, &elf_data[n+16], sizeof(u32));
				cmd_count = swap32(cmd_count);
				sprintf(temp, "Commands count : 0x%X\n", cmd_count); fputs(temp, fo);
				
				//memcpy(&val32, &elf_data[n+20], sizeof(u32));
				//val32 = swap32(val32);
				//sprintf(temp, "unk : 0x%X\n", val32); fputs(temp, fo);
				
				for(i=0; i < cmd_count ; i++) {
					
					memcpy(&cmd_id, &elf_data[cmd_offset+0x10000], sizeof(u32));
					cmd_id = swap32(cmd_id);
					sprintf(temp, "\t[GX] Command ID : 0x%02X\n", cmd_id); fputs(temp, fo);
					
					//memcpy(&val32, &elf_data[cmd_offset+0x10000+4], sizeof(u32));
					//val32 = swap32(val32);
					//sprintf(temp, "\tunk : 0x%X\n", val32); fputs(temp, fo);
				
					switch(cmd_id)
					{
						case 0x00 :
						{
							val32 = 0x01;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							
							u64 func_offset;
							memcpy(&func_offset, &elf_data[cmd_offset+0x10000+0x10], sizeof(u64));
							func_offset = swap64(func_offset);
							sprintf(temp, "\t\t[GX] Function offset : 0x%X\n", func_offset); fputs(temp, fo);
							
							fputs("\t\t[Net] Function ID : ", fo);
							switch(func_offset)
							{
								case 0x34068 :
								{
									val32=0x02;
									break;
								}
								case 0x34144 :
								{
									val32=0x03;
									break;
								}
								case 0x34224 :
								{
									val32=0x06;
									break;
								}
								case 0x36EF0 :
								{
									val32=0x0A;
									break;
								}
								case 0x34354 :
								{
									val32=0x0B;
									break;
								}
								case 0x365F0 :
								{
									val32=0x0F;
									break;
								}
								case 0x36510 :
								{
									val32=0x10;
									break;
								}
								case 0x36430 :
								{
									val32=0x11;
									break;
								}
								case 0x36FC8 :
								{
									val32=0x2C;
									break;
								}
								default :
									val32=0xFFFFFFFF;
									break;
									
							}
							if(val32==0xFFFFFFFF) {
								tofix=1;
								fputs("unk\n", fo);
							} else {
								sprintf(temp, "0x%02X\n", val32); fputs(temp, fo);
							}
							fwrite(&val32, 1, sizeof(u32), cfg);
							break;
						}
						case 0x01 :
						{
							val32 = 0x02;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x02 :
						{
							val32 = 0x03;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x03 :
						{
							val32 = 0x04;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x04 :
						{
							val32 = 0x05;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x05 :
						{
							val32 = 0x06;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x06 :
						{
							val32 = 0x07;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x07 :
						{
							
							val32 = 0x08;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							u64 Data_offset;
							memcpy(&Data_offset, &elf_data[cmd_offset+0x10000+8], sizeof(u64));
							Data_offset = swap64(Data_offset);
							sprintf(temp, "\t\tData Offset : %08X\n", Data_offset); fputs(temp, fo);
											
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*0], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\t\tPatchedDataMask : %08X", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*1], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "%08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*2], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\t\tPatchedData : %08X", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*3], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "%08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*4], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\t\tOriginalDataMask : %08X", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*5], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "%08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*6], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\t\tOriginalData : %08X", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[Data_offset+0x10000+4*7], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "%08X\n", val32); fputs(temp, fo);
										
							break;
						}
						case 0x08 :
						{
							val32 = 0x09;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
								
							u64 Data_offset;
							memcpy(&Data_offset, &elf_data[cmd_offset+0x10000+8], sizeof(u64));
							Data_offset = swap64(Data_offset);
							sprintf(temp, "\t\tData Offset : %08X\n", Data_offset); fputs(temp, fo);
							
							u32 Data_Number;
							memcpy(&Data_Number, &elf_data[cmd_offset+0x10000+0x10], sizeof(u32));
							Data_Number = swap32(Data_Number);
							fwrite(&Data_Number, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tData Number : %08X\n", Data_Number); fputs(temp, fo);
							
							for(j=0; j < Data_Number; j++) {
								memcpy(&val32, &elf_data[Data_offset+0x10000], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tOffset : %08X\n", val32); fputs(temp, fo);
								
								memcpy(&val32, &elf_data[Data_offset+0x10000+4], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tOriginal 1 : %08X\n", val32); fputs(temp, fo);
								
								memcpy(&val32, &elf_data[Data_offset+0x10000+8], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tOriginal 2 : %08X\n", val32); fputs(temp, fo);
								
								memcpy(&val32, &elf_data[Data_offset+0x10000+0xC], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tPatched 1 : %08X\n", val32); fputs(temp, fo);
								
								memcpy(&val32, &elf_data[Data_offset+0x10000+0x10], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tPatched 2 : %08X\n", val32); fputs(temp, fo);
								
								Data_offset += 0x18;
							}
							
							break;
						}
						case 0x09 :
						{
							val32 = 0x0B;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							u64 Data_offset;
							memcpy(&Data_offset, &elf_data[cmd_offset+0x10000+8], sizeof(u64));
							Data_offset = swap64(Data_offset);
							sprintf(temp, "\t\tData Offset : %08X\n", Data_offset); fputs(temp, fo);
														
							u32 Data_Number;
							memcpy(&Data_Number, &elf_data[cmd_offset+0x10000+0x10], sizeof(u32));
							Data_Number = swap32(Data_Number);
							fwrite(&Data_Number, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tData Number : %08X\n", Data_Number); fputs(temp, fo);
														
							for(j=0; j < Data_Number; j++) {
								
								memcpy(&val32, &elf_data[Data_offset+0x10000], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tSector : %08X\n", val32); fputs(temp, fo);
																
								memcpy(&val32, &elf_data[Data_offset+0x10000+4], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tOffset : %08X\n", val32); fputs(temp, fo);
																
								u32 data_size;
								memcpy(&data_size, &elf_data[Data_offset+0x10000+0x18], sizeof(u32));
								data_size = swap32(data_size);
								fwrite(&data_size, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tSize : %08X\n", data_size); fputs(temp, fo);
																
								//memcpy(&val32, &elf_data[Data_offset+0x10000+0x1C], sizeof(u32));
								//val32 = swap32(val32);
								//sprintf(temp, "\t\t\tunk : 0x%X\n", val32); fputs(temp, fo);
								
								u32 *data =(u32 *) malloc(data_size+1);
								if(data==NULL) {
									printf("Failed to malloc");
									return;
								}
								
								memcpy(&val64, &elf_data[Data_offset+0x10000+0x8], sizeof(u64));
								val64 = swap64(val64);
								sprintf(temp, "\t\t\tPatched data offset : 0x%X\n", val64); fputs(temp, fo);
								
								memcpy(data, &elf_data[val64+0x10000], data_size);
								
								fputs("\t\t\tPatched data : ", fo);
								
								for(k=0; k<data_size/4; k++) {
									data[k] = swap32(data[k]);
									fwrite(&data[k], 1, sizeof(u32), cfg);
									sprintf(temp, "%08X", data[k]); fputs(temp, fo);
								}
								fputs("\n", fo);
								
								memcpy(&val64, &elf_data[Data_offset+0x10000+0x10], sizeof(u64));
								val64 = swap64(val64);
								sprintf(temp, "\t\t\tOriginal data offset : 0x%X\n", val64); fputs(temp, fo);
								
								memcpy(data, &elf_data[val64+0x10000], data_size);
															
								fputs("\t\t\tOriginal data : ", fo);
								
								for(k=0; k<data_size/4; k++) {
									data[k] = swap32(data[k]);
									fwrite(&data[k], 1, sizeof(u32), cfg);
									sprintf(temp, "%08X", data[k]); fputs(temp, fo);
								}
								fputs("\n", fo);
								
								free(data);
								
								Data_offset += 0x20;
							}
							break;
						}
						case 0x0A :
						{
							val32 = 0x0C;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 1 : %04X\n", val32 >> 16); fputs(temp, fo);
							sprintf(temp, "\t\tParam 2 : %04X\n", val32 & 0xFFFF); fputs(temp, fo);
														
							break;
						}
						case 0x0B :
						{
							val32 = 0x0D;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x0C :
						{
							val32 = 0x0E;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x0D :
						{
							val32 = 0x0F;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 1 : %08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 2 : %08X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x0E :
						{
							val32 = 0x10;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 1 : %08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 2 : %08X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x0F :
						{
							val32 = 0x11;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);

							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x10 :
						{
							val32 = 0x12;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							u64 Data_offset;
							memcpy(&Data_offset, &elf_data[cmd_offset+0x10000+8], sizeof(u64));
							Data_offset = swap64(Data_offset);
							sprintf(temp, "\t\tData Offset : %08X\n", Data_offset); fputs(temp, fo);
														
							u32 Data_Number;
							memcpy(&Data_Number, &elf_data[cmd_offset+0x10000+0x10], sizeof(u32));
							Data_Number = swap32(Data_Number);
							fwrite(&Data_Number, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tData Number : %08X\n", Data_Number); fputs(temp, fo);
							
							for(j=0; j < Data_Number; j++) {
								
								memcpy(&val32, &elf_data[Data_offset+0x10000], sizeof(u32));
								val32 = swap32(val32);
								fwrite(&val32, 1, sizeof(u32), cfg);
								sprintf(temp, "\t\t\tParam %d : %08X\n", j+1, val32); fputs(temp, fo);
								
								Data_offset+=4;
							}
							
							break;
						}
						case 0x11 :
						{
							val32 = 0x13;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "%08X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x12 :
						{
							val32 = 0x14;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x13 :
						{
							val32 = 0x15;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x14 :
						{
							val32 = 0x16;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x15 :
						{
							val32 = 0x17;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0x8], sizeof(u32));
							fwrite(&val32, 1, sizeof(u32), cfg);
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : %02X\n", val32>>24); fputs(temp, fo);
														
							break;
						}
						case 0x16 :
						{
							val32 = 0x18;
							fwrite(&val32, 1, sizeof(u32), cfg);
							
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							fputs("\t\tNothing to do\n", fo);
														
							break;
						}
						case 0x17 :
						{
							val32 = 0x1A;
							fwrite(&val32, 1, sizeof(u32), cfg);
							
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							fputs("\t\tNothing to do\n", fo);

							break;
						}
						case 0x18 :
						{
							val32 = 0x1B;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x19 :
						{
							val32 = 0x1C;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0x8], sizeof(u32));
							fwrite(&val32, 1, sizeof(u32), cfg);
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : %02X\n", val32>>24); fputs(temp, fo);
															
							break;
						}
						case 0x1A :
						{
							val32 = 0x1D;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0x8], sizeof(u32));
							fwrite(&val32, 1, sizeof(u32), cfg);
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : %02X\n", val32>>24); fputs(temp, fo);
															
							break;
						}
						case 0x1B :
						{
							val32 = 0x1E;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0x8], sizeof(u32));
							fwrite(&val32, 1, sizeof(u32), cfg);
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : %02X\n", val32>>24); fputs(temp, fo);
															
							break;
						}
						case 0x1C :
						{
							val32 = 0x1F;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);

							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x1D :
						{
							val32 = 0x20;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "%08X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x1E :
						{
							val32 = 0x21;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x1F :
						{
							val32 = 0x22;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x20 :
						{
							val32 = 0x24;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(u32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "%08X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x21 :
						{
							val32 = 0x25;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x22 :
						{
							val32 = 0x26;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 1 : %08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 2 : %08X\n", val32); fputs(temp, fo);
							
							break;
							break;
						}
						case 0x23 :
						{
							val32 = 0x27;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 1 : %08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 2 : %08X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x24 :
						{
							val32 = 0x28;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x25 :
						{
							val32 = 0x29;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 1 : %08X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam 2 : %08X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x26 :
						{
							val32 = 0x2A;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x27 :
						{
							val32 = 0x2B;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x28 :
						{
							val32 = 0x2C;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x29 :
						{
							val32 = 0x2D;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x2A :
						{
							val32 = 0x2E;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);

							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x2B :
						{
							val32 = 0x2F;
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t[Net] Command ID : 0x%02X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							fwrite(&val32, 1, sizeof(u32), cfg);
							sprintf(temp, "\t\tParam : %08X\n", val32); fputs(temp, fo);
							break;
						}
						default :
							fputs("\t\tCommand unknown\n", fo);
							break;
					}
					
				
					cmd_offset += 0x18;
				}
				
				val32 = 0;
				fwrite(&val32, 1, sizeof(u32), cfg);
				fclose(cfg);
				
				if(tofix) rename(config_file, config_file_tofix);
				break;
			}
			
		}
		
		fputs("\n", fo);
		
	}
	
	fclose(ft);
	fclose(fo);
	free(elf_data);
	
	return 0;
}