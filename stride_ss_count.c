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
    fprintf(stderr, "\t%s stride_utput_file\n", name);
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
    E   B-Sheet
    B/b Isolated bridge
    T   Turn
    I   PI-helix
    H   Alpha helix
    G   3-10 helix
    C   Coil (none of the above)
    */
    unsigned long nr;
    unsigned long temp = 0, Chain = 1;
    char buff[256];
    
    char ssbuff[ReadLines(argv[1])];
    FILE *fp = fopen(argv[1], "r");
    
    //skip useless lines
    do {
        fgets(buff, sizeof(buff), fp);
    }
    while(!(strstr(buff, "Residue") != NULL && strstr(buff, "Phi") != NULL));
    
    //from Residue/Phi line start, read residue information and save it 
    unsigned long number1 = 8;
    for(nr = 0; (fgets(buff, sizeof(buff), fp) != NULL); nr++) {
        sscanf(buff+11, "%4d", &number1);
        if(temp != 0 && (number1 != temp+1)) {Chain++;}
        sscanf(buff+11, "%4d", &temp);
        ssbuff[nr] = buff[24];
    }
    fclose(fp);
    
    //Calculate Each Structure percentage
    unsigned long isE, isB, isT, isH, isG, isCoil, isI;
    isE = isB = isT = isH = isG = isCoil = isI = 0;
    for(unsigned long i = 0; i < nr; i++) {
        if(ssbuff[i] == 'E') {isE++;}
        else if(ssbuff[i] == 'B' || ssbuff[i] == 'b') {isB++;}
        else if(ssbuff[i] == 'T') {isT++;}
        else if(ssbuff[i] == 'H') {isH++;}
        else if(ssbuff[i] == 'G') {isG++;}
        else if(ssbuff[i] == 'I') {isI++;}
        else if(ssbuff[i] == 'C') {isCoil++;}
    }
    unsigned long Nres = nr;
    
    //print result on screen
    printf("\nThis Protein has %2lu Chains,\n"
           "Total Number of Residue is %4lu,\n"
           "Each Structure percentage as follows:\n\n",
            Chain, Nres);
    PrintS("B-Sheet", isE/(double)Nres);
    PrintS("B-Bridge", isB/(double)Nres);
    PrintS("PI-helix", isI/(double)Nres);
    PrintS("Turn", isT/(double)Nres);
    PrintS("A-Helix", isH/(double)Nres);
    PrintS("3-10 helix", isG/(double)Nres);
    PrintS("Coil", isCoil/(double)Nres);
    
    return 0;
}