#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv)
{

	FILE *fid;
	FILE *ft;
	FILE *fr;
	
	char IDhash[128];
	char ID[32];
	char title[255];
	
	fid = fopen("ID.txt", "r");
	ft = fopen("PS2.txt", "r");
	fr =  fopen("result.txt", "w");
	
	while(fgets(IDhash, 128, fid))
	{
		strncpy(ID, IDhash, 11);
		ID[4]='-';
		ID[8]=ID[9];
		ID[9]=ID[10];
		ID[10]=0;
		strtok(IDhash, "\r\n");
		
		fseek(ft, 0, SEEK_SET);
		while(fgets(title, 255, ft)){

			if(strstr(title, ID) != NULL) {
				fputs(IDhash, fr);
				fputs(strchr(title,'\t'), fr);
				break;
			}
		}
		
	
	}
	fclose(fid);
	fclose(ft);
	fclose(fr);
	
	return 0;
}