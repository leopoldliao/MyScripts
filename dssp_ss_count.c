#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//print information
void PrintS(const char *Term, double Value) {
    fprintf(stderr, "%10s is: %8.3lf%s\n", Term, Value*100, "%");
}

//print usage
void Usage(const char *name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s dssp_output_file\n", name);
    exit(1);
}

//read file and determine lines, larger than actual number of protein residue
unsigned long ReadLines(const char *fname) {
    FILE *fp = fopen(fname, "r");
    if(fp == NULL) {
        fprintf(stderr, "Error! File(%s) not open!\n", fname);
        exit(1);
    }
    char buff[256];
    unsigned long Lines = 0;
    while(fgets(buff, sizeof(buff), fp) != NULL) {
        Lines++;
    }
    fclose(fp);
    return Lines;
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Error! Not found Input file\n");
        Usage(argv[0]);
    }
    
    /*
    E B-Sheet
    B B-Bridge
    S Bend
    T Turn
    H A-Helix
    G 3-Helix
    = Chain separator
    ~ Coil
    */
    unsigned long nr;
    char buff[256];
    char SSTP;
    
    char ssbuff[ReadLines(argv[1])];
    FILE *fp = fopen(argv[1], "r");
    
    //skip useless lines
    do {
        fgets(buff, sizeof(buff), fp);
    }
    while(strstr(buff, "KAPPA") == NULL);
    
    //from KAPPA line start, read residue information and save it 
    for(nr = 0; (fgets(buff, sizeof(buff), fp) != NULL); nr++) {
        if(buff[13] == '!') {
            SSTP = '=';
        }
        else {
            SSTP = buff[16] == ' ' ? '~' : buff[16];
        }
        ssbuff[nr] = SSTP;
    }
    fclose(fp);
    
    //Calculate Each Structure percentage
    unsigned long isE, isB, isS, isT, isH, isG, isCoil, No;
    isE = isB = isS = isT = isH = isG = isCoil = No = 0;
    for(unsigned long i = 0; i < nr; i++) {
        if(ssbuff[i] == 'E') {isE++;}
        else if(ssbuff[i] == 'B') {isB++;}
        else if(ssbuff[i] == 'S') {isS++;}
        else if(ssbuff[i] == 'T') {isT++;}
        else if(ssbuff[i] == 'H') {isH++;}
        else if(ssbuff[i] == 'G') {isG++;}
        else if(ssbuff[i] == '~') {isCoil++;}
        else if(ssbuff[i] == '=') {No++;}
    }
    unsigned long Nres = nr - No;
    
    //print result on screen
    printf("\nThis Protein has %2lu Chains,\n"
           "Total Number of Residue is %4lu,\n"
           "Each Structure percentage as follows:\n\n",
            No+1, Nres);
    PrintS("B-Sheet", isE/(double)Nres);
    PrintS("B-Bridge", isB/(double)Nres);
    PrintS("Bend", isS/(double)Nres);
    PrintS("Turn", isT/(double)Nres);
    PrintS("A-Helix", isH/(double)Nres);
    PrintS("3-Helix", isG/(double)Nres);
    PrintS("Coil", isCoil/(double)Nres);
    
    return 0;
}