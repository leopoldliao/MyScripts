#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
    A simple C program for extracing atom charge information from GROMACS
    .tpr file. 
*/

typedef struct {
    char Name[256], Natm[256];
    int Nmol;
    char (*Qtam)[256], (*Tatm)[256];
} t_mol;

void Header();

int main() {
    char filename[256], command[256];
    Header();
    
    printf("Please input .tpr file or a txt file from \"gmx dump\":\n");
    scanf("%s", filename);
    printf("\nPlease waiting...\n");
    if(strstr(filename, ".tpr") != NULL) {
        sprintf(command, "gmx dump -quiet -s %s > dump.txt 2>&1", filename);
        system(command);
    }

    FILE *fp;
    char buff[1024];
    int i, j, Ntyp, Imol, tmp, im = 0, num, maximol = 0;

    if(strstr(filename, ".tpr") != NULL) {
        fp = fopen("dump.txt", "r");
    }
    else {
        fp = fopen(filename, "r");
    }
    if(fp == NULL) {
        printf("Can not find a txt file\n");
        exit(-1);
    }
    
    t_mol mol[100];
    while(fgets(buff, sizeof(buff), fp) != NULL) {
        if(strstr(buff, "#molblock") != NULL) {
            sscanf(buff, "%*s%*s%d", &Ntyp);
            for(i = 0; i < Ntyp; i++) {
                mol[i].Nmol = 0;
            }
        } 

        if((strstr(buff, "moltype") != NULL) && (strstr(buff, "(") == NULL)) {
            sscanf(buff, "%*s%*s%d", &Imol);
            maximol = (maximol > Imol)? maximol : Imol;
            sscanf(buff, "%*s%*s%*d%*[^\"]\"%[^\"]", mol[Imol].Name);
            fgets(buff, sizeof(buff), fp);
            sscanf(buff, "%*s%*s%d", &tmp);
            mol[Imol].Nmol += tmp;
            //printf("%d %s %d\n", Imol, mol[Imol].Name, mol[Imol].Nmol);
        }

        if((strstr(buff, "moltype (") != NULL)) {
            fgets(buff, sizeof(buff), fp);
            fgets(buff, sizeof(buff), fp);
            fgets(buff, sizeof(buff), fp);
            sscanf(buff, "%*s%*[^(](%[^)])", mol[im].Natm);
            num = atoi(mol[im].Natm);
            mol[im].Qtam = (char(*)[256])malloc(256*sizeof(char)*num);
            mol[im].Tatm = (char(*)[256])malloc(256*sizeof(char)*num);
            for(i = 0; i < num; i++) {
                fgets(buff, sizeof(buff), fp);
                sscanf(buff, "%*[^q]%*[^=]=%[^,]", mol[im].Qtam[i]);
                //fprintf(fp1, "%3d %15s %s\n", i, mol[im].Qtam[i], mol[im].Name);
            }
            fgets(buff, sizeof(buff), fp);
            for(i = 0; i < num; i++) {
                fgets(buff, sizeof(buff), fp);
                sscanf(buff, "%*[^\"]\"%[^\"]", mol[im].Tatm[i]);
            }
            im++;
        }
    }
    fclose(fp);
    
    FILE *fp1;
    int Ntot = 0, n = 0;
    printf("The System has %d molblock, %d unique moltype, %d repetition.\n", 
            Ntyp, im, Ntyp - im);
    fp1 = fopen("charge.txt", "w");
    printf("%5s %20s %5s %5s\n", "Index", "Mol", "Atom", "Num");
    for(i = 0; i <= maximol; i++) {
        printf("%5d %20s %5s %5d\n", i+1, mol[i].Name, mol[i].Natm, mol[i].Nmol);
        for(n = 0; n < mol[i].Nmol; n++) {
            for(j = 0; j <  atoi(mol[i].Natm); j++) {
                Ntot++;
                fprintf(fp1, "%6d %15s %6d %-15s %5d %s\n", Ntot, mol[i].Qtam[j],
                        n+1, mol[i].Name, j+1, mol[i].Tatm[j]);
            }
        }
    }
    fclose(fp1);
    
    for(i = 0; i < im; i++) {
        free(mol[i].Qtam);
        free(mol[i].Tatm);
    }
    
    printf("\n--> Done! charge.txt file has been generated.\n");
    
#ifdef _WIN32
    printf("Please press enter to exit!\n");
    system("PAUSE");
#endif

    return 0;
}

void Header() {
    printf("\n\t**************************************************************\n");
    printf("\t*                  Read Atomic Charge Program                *\n");
    printf("\t*                      Author: Yujie Liu                     *\n");
    printf("\t*                    First release: 2019.08.01               *\n");
    printf("\t*                    Last   update: 2020.08.15               *\n");
    printf("\t**************************************************************\n\n");
    printf(">>> DESCRIPTION:\n");
    printf("\tThis program can read atomic charge from .tpr file (Need install gromacs)\n");
    printf("\tor txt file from \"gmx dump\", a charge.txt file will be generated.\n");
    printf("\t===================================================================\n\n\n");
}
