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
	elf = fopen("ps2_gxemu.elf", "r");
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
	ft = fopen("get_title/result.txt", "r");
	if(ft==NULL) {
		printf("result.txt not found");
		return;
	}
	
	FILE *fo;
	fo = fopen("log.txt", "w");
	
	while(fgets(entry, 255, ft)) {
		sscanf(entry, "%s %llX", game_ID, (unsigned long long int *) &hash);
		
		sprintf(temp, "Title : %s", &strchr(entry, '\t')[1]); fputs(temp, fo);
		sprintf(temp, "Game ID : %s\n", game_ID); fputs(temp, fo);
		sprintf(temp, "Hash : 0x%llX\n", hash); fputs(temp, fo);
		
		hash = swap64(hash);
		
		for(n=0; n<elf_size; n++) {
			if(!memcmp(&elf_data[n], &hash, 8)) {
				sprintf(temp, "Hash Offset : 0x%X\n", n); fputs(temp, fo);
				
				memcpy(&cmd_offset, &elf_data[n+8], sizeof(u64));
				cmd_offset = swap64(cmd_offset);
				sprintf(temp, "Commands offset : 0x%llX\n", cmd_offset); fputs(temp, fo);
				
				memcpy(&cmd_count, &elf_data[n+16], sizeof(u32));
				cmd_count = swap32(cmd_count);
				sprintf(temp, "Commands count : 0x%X\n", cmd_count); fputs(temp, fo);
				
				//memcpy(&val32, &elf_data[n+20], sizeof(u32));
				//val32 = swap32(val32);
				//sprintf(temp, "unk : 0x%X\n", val32); fputs(temp, fo);
				
				for(i=0; i < cmd_count ; i++) {
										
					memcpy(&cmd_id, &elf_data[cmd_offset+0x10000], sizeof(u32));
					cmd_id = swap32(cmd_id);
					sprintf(temp, "\tCommand ID : 0x%02X\n", cmd_id); fputs(temp, fo);
					
					//memcpy(&val32, &elf_data[cmd_offset+0x10000+4], sizeof(u32));
					//val32 = swap32(val32);
					//sprintf(temp, "\tunk : 0x%X\n", val32); fputs(temp, fo);
				
					switch(cmd_id)
					{
						case 0x01 :
						{
							fputs("\tNetEMU ID : 0x01\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x02 :
						{
							fputs("\tNetEMU ID : 0x03\n", fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x03 :
						{
							fputs("\tNetEMU ID : 0x04\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x04 :
						{
							fputs("\tNetEMU ID : 0x05\n", fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x05 :
						{
							fputs("\tNetEMU ID : 0x06\n", fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x06 :
						{
							fputs("\tNetEMU ID : 0x07\n", fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x09 :
						{
							fputs("\tNetEMU ID : 0x0B\n", fo);
							
							u64 Data_offset;
							memcpy(&Data_offset, &elf_data[cmd_offset+0x10000+8], sizeof(u64));
							Data_offset = swap64(Data_offset);
							sprintf(temp, "\t\tData Offset : 0x%X\n", Data_offset); fputs(temp, fo);
														
							u32 Data_Number;
							memcpy(&Data_Number, &elf_data[cmd_offset+0x10000+0x10], sizeof(u32));
							Data_Number = swap32(Data_Number);
							sprintf(temp, "\t\tData Number : 0x%X\n", Data_Number); fputs(temp, fo);
														
							for(j=0; j < Data_Number; j++) {
								
								memcpy(&val32, &elf_data[Data_offset+0x10000], sizeof(u32));
								val32 = swap32(val32);
								sprintf(temp, "\t\t\tSector : 0x%X\n", val32); fputs(temp, fo);
																
								memcpy(&val32, &elf_data[Data_offset+0x10000+4], sizeof(u32));
								val32 = swap32(val32);
								sprintf(temp, "\t\t\tOffset : 0x%X\n", val32); fputs(temp, fo);
																
								u32 data_size;
								memcpy(&data_size, &elf_data[Data_offset+0x10000+0x18], sizeof(u32));
								data_size = swap32(data_size);
								sprintf(temp, "\t\t\tSize : 0x%X\n", data_size); fputs(temp, fo);
																
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
							fputs("\tNetEMU ID : 0x0C\n", fo);
							
							memcpy(&val16, &elf_data[cmd_offset+0x10000+8], sizeof(val16));
							val16 = swap16(val16);
							sprintf(temp, "\t\tParam 1 : 0x%X\n", val16); fputs(temp, fo);
							
							memcpy(&val16, &elf_data[cmd_offset+0x10000+0xA], sizeof(val16));
							val16 = swap16(val16);
							sprintf(temp, "\t\tParam 2 : 0x%X\n", val16); fputs(temp, fo);
							
							break;
						}
						case 0x0B :
						{
							fputs("\tNetEMU ID : 0x0D\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x0C :
						{
							fputs("\tNetEMU ID : 0x0E\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x0F :
						{
							fputs("\tNetEMU ID : 0x11\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x11 :
						{
							fputs("\tNetEMU ID : 0x13\n", fo);
							memcpy(&val64, &elf_data[cmd_offset+0x10000+8], sizeof(val64));
							val64 = swap64(val64);
							sprintf(temp, "\t\tParam : 0x%X\n", val64); fputs(temp, fo);
							break;
						}
						case 0x12 :
						{
							fputs("\tNetEMU ID : 0x13\n", fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x13 :
						{
							fputs("\tNetEMU ID : 0x0D\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x18 :
						{
							fputs("\tNetEMU ID : 0x1B\n", fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x19 :
						{
							fputs("\tNetEMU ID : 0x1C\n", fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+8], sizeof(val8));
							sprintf(temp, "\t\tParam 1 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+9], sizeof(val8));
							sprintf(temp, "\t\tParam 2 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+10], sizeof(val8));
							sprintf(temp, "\t\tParam 3 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+11], sizeof(val8));
							sprintf(temp, "\t\tParam 4 : 0x%X\n", val8); fputs(temp, fo);
							break;
						}
						case 0x1A :
						{
							fputs("\tNetEMU ID : 0x1D\n", fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+8], sizeof(val8));
							sprintf(temp, "\t\tParam 1 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+9], sizeof(val8));
							sprintf(temp, "\t\tParam 2 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+10], sizeof(val8));
							sprintf(temp, "\t\tParam 3 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+11], sizeof(val8));
							sprintf(temp, "\t\tParam 4 : 0x%X\n", val8); fputs(temp, fo);
							break;
						}
						case 0x1B :
						{
							fputs("\tNetEMU ID : 0x1E\n", fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+8], sizeof(val8));
							sprintf(temp, "\t\tParam 1 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+9], sizeof(val8));
							sprintf(temp, "\t\tParam 2 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+10], sizeof(val8));
							sprintf(temp, "\t\tParam 3 : 0x%X\n", val8); fputs(temp, fo);
							
							memcpy(&val8, &elf_data[cmd_offset+0x10000+11], sizeof(val8));
							sprintf(temp, "\t\tParam 4 : 0x%X\n", val8); fputs(temp, fo);
							break;
						}
						case 0x1C :
						{
							fputs("\tNetEMU ID : 0x1F\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x1D :
						{
							fputs("\tNetEMU ID : 0x20\n", fo);
							memcpy(&val64, &elf_data[cmd_offset+0x10000+8], sizeof(val64));
							val64 = swap64(val64);
							sprintf(temp, "\t\tParam : 0x%X\n", val64); fputs(temp, fo);
							break;
						}
						case 0x1E :
						{
							fputs("\tNetEMU ID : 0x21\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x1F :
						{
							fputs("\tNetEMU ID : 0x22\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x20 :
						{
							fputs("\tNetEMU ID : 0x24\n", fo);
							memcpy(&val64, &elf_data[cmd_offset+0x10000+8], sizeof(val64));
							val64 = swap64(val64);
							sprintf(temp, "\t\tParam : 0x%X\n", val64); fputs(temp, fo);
							break;
						}
						case 0x24 :
						{
							fputs("\tNetEMU ID : 0x28\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x25 :
						{
							fputs("\tNetEMU ID : 0x29\n", fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam 1: 0x%X\n", val32); fputs(temp, fo);
							
							memcpy(&val32, &elf_data[cmd_offset+0x10000+0xC], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam 2: 0x%X\n", val32); fputs(temp, fo);
							
							break;
						}
						case 0x26 :
						{
							fputs("\tNetEMU ID : 0x2A\n", fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x27 :
						{
							fputs("\tNetEMU ID : 0x2B\n", fo);
							fputs("\t\tNothing to do\n", fo);
							break;
						}
						case 0x28 :
						{
							fputs("\tNetEMU ID : 0x2C\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x2A :
						{
							fputs("\tNetEMU ID : 0x2E\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						case 0x2B :
						{
							fputs("\tNetEMU ID : 0x2F\n", fo);
							memcpy(&val32, &elf_data[cmd_offset+0x10000+8], sizeof(val32));
							val32 = swap32(val32);
							sprintf(temp, "\t\tParam : 0x%X\n", val32); fputs(temp, fo);
							break;
						}
						
						default :
							fputs("\t\tCommand unknown\n", fo);
							break;
					}
					
				
					cmd_offset += 0x18;
				}
				
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