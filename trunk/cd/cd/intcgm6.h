void setlinestyle ( int );
void setlinewidth ( double );
void setmarktype ( int );
void setmarksize ( double );
long int getcolor ( tcolor );
long int *setpattern ( pat_table );
int setfont ( int, int, double );
int setintstyle ( int );
int vdcx2canvas ( double );
int vdcy2canvas ( double );
int delta_vdc2canvas ( double );
double canvas2vdcx ( int );
double canvas2vdcy ( int );
void GetPolyBbox ( tpoint *, int, double *n, double *, double *, double * );
void GetIncPolyBbox ( tpoint *, int, double *, double *, double *, double * );
unsigned char *my_realloc ( unsigned char *, size_t );