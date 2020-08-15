#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #define popen  _popen
    #define pclose _pclose
#endif

void genxyz() {
    char *element_list[] = {
         "","H ","He", //1-2
         "Li","Be","B ","C ","N ","O ","F ","Ne", //3-10
         "Na","Mg","Al","Si","P ","S ","Cl","Ar", //11-18
         "K ","Ca","Sc","Ti","V ","Cr","Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr", //19-36
         "Rb","Sr","Y ","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd","In","Sn","Sb","Te","I ","Xe", //37-54
         "Cs","Ba","La","Ce","Pr","Nd","Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu", //55-71
         "Hf","Ta","W ","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po","At","Rn", //72-86
         "Fr","Ra","Ac","Th","Pa","U ","Np","Pu","Am","Cm","Bk","Cf","Es","Fm","Md","No","Lr", //87-103
         "Rf","Db","Sg","Bh","Hs","Mt","Ds","Rg","Cn","Ut","Fl","Up","Lv","Us","Uo","Un","Ux" //104-
         };
    const double b2a = 0.529177249;
    char buff[256];
    int Natom, index;
    double x, y, z;
    
    FILE *fp = fopen("mol.tmp", "r");
    FILE *fout = fopen("mol.xyz", "w");
    fgets(buff, sizeof(buff), fp);
    sscanf(buff, "%d", &Natom);
    fprintf(fout, "%d\n\n", Natom);
    fgets(buff, sizeof(buff), fp);
    for(int i = 0; i< Natom; i++) {
        fgets(buff, sizeof(buff), fp);
        sscanf(buff, "%d %lf %lf %lf", &index, &x, &y, &z);
        fprintf(fout, "%s %lf %lf %lf\n", element_list[index], b2a*x, b2a*y, b2a*z);
    }
    
    fclose(fp); remove("mol.temp");
    fclose(fout);
}

void extderi(char *outname, const int atoms, const int derivs) {
    FILE *ftest = NULL, *fout;
    char buff[256];
    double ene, fx, fy, fz;
    
    ftest = fopen("xtbout", "r");
    if(ftest == NULL) {
        printf("Error: xtbout is not existed in current folder!\n");
        exit(-1);
    }
    while(fgets(buff, sizeof(buff), ftest)) {
        if(strstr(buff, "total energy") != NULL)
            break;
    }
    sscanf(buff+30, "%lf", &ene);
    fclose(ftest);
    
    fout = fopen(outname, "w");
    fprintf(fout, "%20.12E%20.12E%20.12E%20.12E\n", ene, 0.0, 0.0, 0.0);
    
    ftest = fopen("gradient", "r");
    if(ftest == NULL) {
        printf("Error: gradient is not existed in current folder!\n");
        exit(-1);
    }
    fgets(buff, sizeof(buff), ftest);
    fgets(buff, sizeof(buff), ftest);
    for(int i = 0; i < atoms; i++) {
        fgets(buff, sizeof(buff), ftest);
    }
    
    for(int i = 0; i < atoms; i++) {
        fgets(buff, sizeof(buff), ftest);
        sscanf(buff, "%lf %lf %lf", &fx, &fy, &fz);
        fprintf(fout, "%20.12E%20.12E%20.12E\n", fx, fy, fz);
    }
    fclose(ftest);
  
    if(derivs == 2){
        double polar = 0.0, ddip = 0.0, hess[3*atoms][3*atoms];
        fprintf(fout, "%20.12E%20.12E%20.12E\n", polar, polar, polar);
        fprintf(fout, "%20.12E%20.12E%20.12E\n", polar, polar, polar);
        for(int i = 0; i< 3*atoms; i++) {
             fprintf(fout, "%20.12E%20.12E%20.12E\n", ddip, ddip, ddip);
        }
        ftest = fopen("hessian", "r");
        if(ftest == NULL) {
            printf("Error: hessian is not existed in current folder!\n");
            exit(-1);
        }
        fgets(buff, sizeof(buff), ftest);
         
        for(int i = 0; i < 3*atoms; i++) {
            for(int j = 0; j < 3*atoms; j++) {
                fscanf(ftest, "%lf", &hess[i][j]);
            }
        }
        
        int count = 0;
        for(int i = 0; i < 3*atoms; i++) {
            for(int j = 0; j <= i; j++) {
                fprintf(fout, "%20.12E", hess[i][j]);
                count++;
                if(count%3 == 0) {
                    fprintf(fout, "\n");
                }
            }
        }
        
        fclose(ftest);
    }
    fclose(fout);
}

int main(int argc, char *argv[]) {
    int atoms, derivs, charge, spin;
    char buff[256], temp[73];
    FILE *tapeout;
    
    FILE *f2 = fopen(argv[2], "r");
    fgets(buff, sizeof(buff), f2);
    sscanf(buff, "%d %d %d %d", &atoms, &derivs, &charge, &spin);

    printf("Generating mol.tmp\n");
    FILE *fp = fopen("mol.tmp", "w");
    fprintf(fp, "%d\n\n", atoms);
    for(int i = 1; i <= atoms; i++) {
        fgets(buff, sizeof(buff), f2);
        memset(temp, '\0', sizeof(temp));
        strncpy(temp, buff, 72); temp[72] = '\0';
        fprintf(fp, "%s\n", temp);
    }
    fclose(fp);
    fclose(f2);
    
    char *removelist[] = {"charges", "energy", "xtbrestart", "gradient", "hessian", "xtbout",
                          "hessian", "xtb_normalmodes", "g98_canmode.out", "g98.out", "wbo", 
                          "xtbhess.coord"};
    for(int i = 0; i < 12; i++) {remove(removelist[i]);}
    
    printf("Generating mol.xyz via genxyz\n");
    genxyz();
    
    int uhf = spin - 1;
    char command[256];
    if(derivs == 2) {
        printf("Running: xtb mol.xyz --chrg %d --uhf %d --hess --grad > xtbout\n", charge, uhf);
        sprintf(command, "xtb mol.xyz --chrg %d --uhf %d --hess --grad > xtbout\n", charge, uhf);
#ifdef _WIN32
        if (NULL == (tapeout = popen(command, "r"))) {
            printf("ERROR!\n");
            exit(-1);
        }
        pclose(tapeout);
#else
        system(command);
#endif
    }
    else if (derivs == 1) {
        printf("Running: xtb mol.xyz --chrg %d --uhf %d --grad > xtbout\n", charge, uhf);
        sprintf(command, "xtb mol.xyz --chrg %d --uhf %d --grad > xtbout\n", charge, uhf);
#ifdef _WIN32
        if (NULL == (tapeout = popen(command, "r"))) {
            printf("ERROR!\n");
            exit(-1);
        }
        pclose(tapeout);
#else
        system(command);
#endif
    }
    
    printf("xtb running finished!\n");
    printf("Extracting data from xtb outputs via extderi\n");
    extderi(argv[3], atoms, derivs);
    
    char *removelist1[] = {"charges", "energy", "xtbrestart", "gradient", "hessian", "xtbout", "mol.xyz",
                        "tmpxx", "vibspectrum", "hessian", "xtb_normalmodes", "g98_canmode.out", "g98.out",
                        "wbo", "xtbhess.coord", ".tmpxtbmodef", ".engrad", "xtbtopo.mol", "xtbhess.xyz"};
    for(int i = 0; i < 19; i++) {remove(removelist1[i]);}
    
    return 0;
}
