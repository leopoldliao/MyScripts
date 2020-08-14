#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int atoms, derivs, charge, spin;
    char buff[256], temp[73];
    
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
    
    system("rm -rf charges energy xtbrestart gradient hessian xtbout");
    system("rm -rf hessian xtb_normalmodes g98_canmode.out g98.out wbo xtbhess.coord");
    
    printf("Generating mol.xyz via genxyz\n");
    system("./genxyz");
    
    int uhf = spin - 1;
    char command[256];
    if(derivs == 2) {
        printf("Running: xtb mol.xyz --chrg %d --uhf %d --hess --grad > xtbout\n", charge, uhf);
        sprintf(command, "xtb mol.xyz --chrg %d --uhf %d --hess --grad > xtbout\n", charge, uhf);
        system(command);
    }
    else if (derivs == 1) {
        printf("Running: xtb mol.xyz --chrg %d --uhf %d --grad > xtbout\n", charge, uhf);
        sprintf(command, "xtb mol.xyz --chrg %d --uhf %d --grad > xtbout\n", charge, uhf);
        system(command);
    }
    
    printf("xtb running finished!\n");
    printf("Extracting data from xtb outputs via extderi\n");
    sprintf(command, "./extderi %s %d %d\n", argv[3], atoms, derivs);
    system(command);
    
    system("rm -rf charges energy xtbrestart gradient hessian xtbout mol.xyz tmpxx vibspectrum");
    system("rm -rf hessian xtb_normalmodes g98_canmode.out g98.out wbo xtbhess.coord .tmpxtbmodef");
    system("rm -rf .engrad xtbtopo.mol xtbhess.xyz");
    
    return 0;
}
