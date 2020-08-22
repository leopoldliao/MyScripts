/*
    A simple code for convert .xpm into .dat file with three column,
    can not deal with dssp.xpm file
    
    Author: Yujie Liu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void Usage(const char *);
void Header();

typedef struct {
    int nres, nframe, npic; //column, line, number of color
    double *x_axis, *y_axis;
    char (*colindex)[256];
    char **matrix;
    double dx, dy;
    char label_x[256], label_y[256], legend[256];
} t_xpm;

int main(int argc, char *argv[]) 
{
    if(argc < 2)
    {
        puts("\nError! Need Input file");
        Header();
        Usage(argv[0]);
        exit(-1);
    }
    Header();
    
    FILE *fp = NULL;
    char buff[256];
    
    if((fp = fopen(argv[1], "r")) == NULL) 
    {
        printf("\nError! Input file %s not found\n", argv[1]);
        exit(-1);
    }
    
    //new struct t_xpm
    t_xpm *fesxpm;
    fesxpm = (t_xpm *)malloc(sizeof(t_xpm));
    memset(fesxpm->label_x, '\0', 256);
    memset(fesxpm->label_y, '\0', 256);
    memset(fesxpm->legend, '\0', 256);

    //find axis label and legend
    do 
    {
        fgets(buff, sizeof(buff), fp);
        if(strstr(buff, "x-label") != NULL)
        {
            sscanf(buff, "%*[^\"]%*[\"]%[^\"]", fesxpm->label_x);
        }
        else if(strstr(buff, "y-label") != NULL)
        {
            sscanf(buff, "%*[^\"]%*[\"]%[^\"]", fesxpm->label_y);
        }
        else if(strstr(buff, "legend:") != NULL)
        {
            sscanf(buff, "%*[^\"]%*[\"]%[^\"]", fesxpm->legend);
        }
    }
    while(strstr(buff, "static char") == NULL);
    fgets(buff, sizeof(buff), fp);
    
    
    //column and line
    sscanf(buff, "%*[^0-9] %d %d %d", &fesxpm->nframe, &fesxpm->nres, &fesxpm->npic);

    fesxpm->colindex  = (char (*)[256])malloc(fesxpm->npic* 256*sizeof(char));
    for(int i = 0; i < fesxpm->npic; i++) 
    {
        fgets(buff, sizeof(buff), fp);
        //printf("%c\n", buff[1]);
        strcpy(fesxpm->colindex[i], buff);
    }

    //save x and y axis value
    /*
    If both x and y represent time, should have fesxpm->nframe+1, fesxpm->nres+1 label,
    but data is nres*nframe. if represent residue, should be fesxpm->nres, here I used fesxpm->nframe
    and fesxpm->nres for universal
    */
    fesxpm->x_axis = (double *)malloc(fesxpm->nframe * sizeof(double));
    fesxpm->y_axis = (double *)malloc(fesxpm->nres * sizeof(double));
    char temp[50];
    for(int i = 0; i < fesxpm->nframe; i++) 
    {
        fscanf(fp, "%s", temp);
        
        if(strstr(temp, "/*") != NULL || strstr(temp, "x-axis") != NULL
           || strstr(temp, "*/") != NULL)
        {
            --i;
        }
        else 
        {
            sscanf(temp, "%lf", &fesxpm->x_axis[i]);
            //printf("%8.6lf -- %d\n",  fesxpm->x_axis[i], i);
        }
    }
    fscanf(fp, "%*[^\n]%*c", temp); 
    
    for(int i = 0; i < fesxpm->nres; i++) 
    {
        fscanf(fp, "%s", temp);
        
        if(strstr(temp, "/*") != NULL || strstr(temp, "y-axis") != NULL
           || strstr(temp, "*/") != NULL)
        {
            --i;
        }
        else 
        {
            sscanf(temp, "%lf", &fesxpm->y_axis[i]);
            //printf("%8.6lf -- %d\n",  fesxpm->y_axis[i], i);
        }
    }
    fscanf(fp, "%*[^\n]%*c", temp);
    
    //char Matrix, fesxpm->nres * fesxpm->nframe
    char tempc;
    fesxpm->matrix = (char **)malloc(fesxpm->nres * sizeof(char *));
    for(int i = 0; i < fesxpm->nres; i++) 
    {
        fesxpm->matrix[i] = (char *)malloc(fesxpm->nframe * sizeof(char));
    }
    for(int i = 0; i < fesxpm->nres; i++) 
    {
        for(int j = 0; j < fesxpm->nframe; j++) 
        {
            fscanf(fp, "%c", &tempc);
            if(tempc == '"' || tempc == ',' || tempc == '\n') 
            {
                --j;
            }
            else
            {
                fesxpm->matrix[i][j] = tempc;
                //printf("%c", tempc);
            }
        }
        //printf("\n");
    }
    //printf("%c\n", fesxpm->matrix[fesxpm->nres-1][fesxpm->nframe-1]);
    fclose(fp);

    //processing
    //needed ? this section refer to https://jerkwin.github.io/gmxtool/xpm2all/xpm2all.bsh
    /*
    fesxpm->dx = fesxpm->x_axis[1] - fesxpm->x_axis[0];
    fesxpm->dy = fesxpm->y_axis[1] - fesxpm->y_axis[0];
    */
    
    double x, y;
    double value;
    FILE *fout = NULL;
    fout = fopen("out.dat", "w");
    fprintf(fout, "#%10s %12s %12s      grid: %4d * %4d\n", fesxpm->label_x, 
                  fesxpm->label_y, fesxpm->legend, fesxpm->nres, fesxpm->nframe);
    
    for(int i = 0; i < fesxpm->nres; i++) 
    {
        //y = fesxpm->y_axis[0]+i*fesxpm->dy;
        for(int j = 0; j < fesxpm->nframe; j++)
        {
            //x = fesxpm->x_axis[0]+j*fesxpm->dx;
            for(int k = 0; k < fesxpm->npic; k++) 
            {
                if(fesxpm->colindex[k][1] == fesxpm->matrix[fesxpm->nres-1-i][j])
                {
                    sscanf(fesxpm->colindex[k], "%*[^/]%*s%*[^0-9] %lf", &value);
                    break;
                }
            }
            //fprintf(fout, "%12.6lf %12.6lf %12.6lf\n", x, y, value);
            fprintf(fout, "%12.6lf %12.6lf %12.6lf\n", fesxpm->x_axis[j], fesxpm->y_axis[i], value);
        }
    }
    fclose(fout);
    puts("\n--> Done! out.dat has been generated successfully\n");
   
    //free memory
    free(fesxpm->x_axis);
    free(fesxpm->y_axis);
    free(fesxpm->colindex);
    for(int i = 0; i < fesxpm->nres; i++) 
    {
        free(fesxpm->matrix[i]);
    }
    free(fesxpm->matrix);
    free(fesxpm);
}

void Usage(const char *name)
{
    puts("\nUsage:\n");
    printf("\t%s    *.xpm\n", name);
    puts("\n\tNOTE: Unsupport dssp.xpm from gmx do_dssp\n");
}

void Header() 
{
    printf("\n\t**************************************************************\n");
    printf("\t*                  Convert XPM into Txt Data                 *\n");
    printf("\t*                      Author: Yujie Liu                     *\n");
    printf("\t*                    First release: 2019.08.17               *\n");
    printf("\t*                    Last   update: 2020.08.21               *\n");
    printf("\t**************************************************************\n\n");
    printf(">>> DESCRIPTION:\n");
    printf("\tThis program can read a xpm file and convert it into plain txt file\n");
    printf("\twith three columns. Then you can use other program to draw beautiful\n");
    printf("\tfigure.\n");
    printf("\n\t\t\tCompiled in %s %s\n", __DATE__, __TIME__);
    printf("\t===================================================================\n\n");
}
