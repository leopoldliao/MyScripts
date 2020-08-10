#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

double Dist(double *x[3], const int a, const int b) {
    return sqrt((x[0][a]-x[0][b])*(x[0][a]-x[0][b])+(x[1][a]-x[1][b])*(x[1][a]-x[1][b])+
            (x[2][a]-x[2][b])*(x[2][a]-x[2][b]));
}

void VectorNormal(double *x[3], const int a, const int b, double v[3]) {
    v[0] = x[0][b]-x[0][a];
    v[1] = x[1][b]-x[1][a];
    v[2] = x[2][b]-x[2][a];
    double dist = Dist(x, a, b);
    v[0] /= dist;
    v[1] /= dist;
    v[2] /= dist;
}

static void Header() {
    printf("\n\t**************************************************************\n");
    printf("\t*                     DISTANCE SCAN PROGRAM                  *\n");
    printf("\t*                      AUTHOR: Yujie Liu                     *\n");
    printf("\t*                    First release: 2020.08                  *\n");
    printf("\t*                    Last update  : 2020.08                  *\n");
    printf("\t**************************************************************\n\n");
    printf(">>> DESCRIPTION:\n");
    printf("\tThis program can scan dimer distance by using given two atom index\n");
    printf("\tof monomer, also can use same index of one monomer, but you need\n");
    printf("\tindicate which axis you want scan!\n");
    printf("\n\tOriginal reference can be found http://sobereva.com/469\n");
    printf("\t===================================================================\n\n\n");
}

int main(int argc, char *argv[]) {
    Header();
    
    FILE *fp = NULL;
    char fname[256];
    char buff[256];
    int NatomA, NatomB;
    double *x[3];
    char (*Satom)[256];

    printf("\nInput path of the file containing the dimer structure, e.g. C:\\dimer.txt\n");
    scanf("%s", fname);

    fp = fopen(fname, "r");
    if(fp == NULL) {printf("Error! Not found %s file\n", fname); exit(1);}
    fgets(buff, sizeof(buff), fp);
    sscanf(buff, "%d %d\n", &NatomA, &NatomB);
    
    //Memory
    for(int i = 0; i < 3; i++) {
        x[i] = (double*)calloc(NatomA+NatomB, sizeof(double));
    }
    Satom = (char (*)[256])calloc(NatomA+NatomB, 256);

    fgets(buff, sizeof(buff), fp); //skip
    for(int i = 0; i < NatomA+NatomB; i++) {
        fgets(buff, sizeof(buff), fp);
        sscanf(buff, "%s %lf %lf %lf", Satom[i], &x[0][i], &x[1][i], &x[2][i]);
    }
    fclose(fp);
    
    //Obtain input parameters
    int Atom1, Atom2, step;
    double Initdist, delta;
    printf("\nInput index of an atom in monomer 1 and index of an atom in monomer 2, or two same index of one monomer, their distance will be scanned. e.g. 3 6\n");
    scanf("%d %d", &Atom1, &Atom2);
    Atom1--; Atom2--; //for array
    
    printf("\nInput the initial distance in Angstrom, e.g. 2.2\n");
    scanf("%lf", &Initdist);
    
    printf("\nInput the number of steps, e.g. 10\n");
    scanf("%d", &step);
    
    printf("\nInput stepsize in Angstrom, e.g. 0.15\n");
    scanf("%lf", &delta);
    
    //Processing
    double Ordist, dd, normal[3];
    Ordist = Dist(x, Atom1, Atom2);
    if(Ordist < 1E-6) {
        printf("\nNOTE: Your given two atom coordinate is same\n");
        printf("You need indicate which axis you want scan!\n");
        loop:printf("Input x, y, or z:\n");
        char axis[5];
        int type = 4;
        scanf("%s", axis);
        if(strstr(axis, "x")) {type = 1;}
        else if(strstr(axis, "y")) {type = 2;}
        else if(strstr(axis, "z")) {type = 3;}

        switch(type) {
            case 1:
                normal[0] = 1; normal[1] = normal[2] = 0;
                break;
            case 2:
                normal[1] = 1; normal[0] = normal[2] = 0;
                break;
            case 3:
                normal[2] = 1; normal[0] = normal[1] = 0;
                break;
            default:
                goto loop;
                break;
        }
    }
    else {VectorNormal(x, Atom1, Atom2, normal);}
    
    double temp_x, temp_y, temp_z, l;
    FILE *fout = NULL;
    fout = fopen("new_scan.xyz", "w");
    for(int i = 0; i <= step; i++) {
        fprintf(fout, "%d\n", NatomA+NatomB);
        fprintf(fout, "%6d %12.6lf\n", i, Initdist + i*delta);
        for(int k = 0; k < NatomA; k++) {
            fprintf(fout, "%4s %12.6lf %12.6lf %12.6lf\n", Satom[k], x[0][k], x[1][k], x[2][k]);
        }
        l = Initdist + i*delta - Ordist;
        for(int j = NatomA; j < NatomA+NatomB; j++) {
            temp_x = l*normal[0]+x[0][j];
            temp_y = l*normal[1]+x[1][j];
            temp_z = l*normal[2]+x[2][j];
            fprintf(fout, "%4s %12.6lf %12.6lf %12.6lf\n", Satom[j], temp_x, temp_y, temp_z);
        }
        
    }
    fclose(fout);
    
    //free
    for(int i = 0; i < 3; i++) {free(x[i]);};
    free(Satom);
    
    printf("\n>>>>> Done! new_scan.xyz file has been generated successfully\n");
    while(1) {
        char rrg[5];
        printf("Please input \"q\" to exit program\n");
        scanf("%s", rrg);
        if(strstr(rrg, "q") != NULL) {break;}
    }
    
    return 0;
}