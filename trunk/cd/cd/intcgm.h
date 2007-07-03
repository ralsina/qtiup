typedef int (*CGM_FUNC) (void);

#ifdef _INTCGM1_C_

tlimit vdc_ext;
double scale_factor_x;
double scale_factor_y;
double scale_factor_mm_x;
double scale_factor_mm_y;
double scale_factor;
int xmin, ymin, xmax, ymax;
int view_clip = 0;
int view_xmin, view_ymin, view_xmax, view_ymax;
double clip_xmin, clip_ymin, clip_xmax, clip_ymax;

t_cgm cgm;

_line_att line_att = { 1, LINE_SOLID, 1., {1} };

_marker_att marker_att = { 3, MARK_ASTERISK, 1., {1} };

_text_att text_att = { 1, 1, NULL, CD_SYSTEM, CD_PLAIN, 8,  STRING, 1., 0., {1}, .1,
                       {0,1}, {1,0}, PATH_RIGHT, {NORMHORIZ,NORMVERT,0.,0.} };

_fill_att fill_att = { 1, HOLLOW, {1}, 1, 1, {0,0}, NULL, {{0.,0.},{0.,0.}} };

_edge_att edge_att = { 1, EDGE_SOLID, 1., {1}, OFF };

trgb *color_table;
int block;

TList *asf_list;

tpoint *point_list;
int npoints;

CGM_FUNC funcs[] = { NULL, &cgmb_rch, &cgmt_rch };

#else

extern t_cgm x;
extern t_cgm cgm;

extern tlimit vdc_ext;
extern double scale_factor_x;
extern double scale_factor_y;
extern double scale_factor_mm_x;
extern double scale_factor_mm_y;
extern double scale_factor;
extern int xmin, ymin, xmax, ymax;
extern int view_clip;
extern int view_xmin, view_ymin, view_xmax, view_ymax;
extern double clip_xmin, clip_ymin, clip_xmax, clip_ymax;

extern _line_att line_att;

extern _marker_att marker_att;

extern _text_att text_att;

extern _fill_att fill_att;

extern _edge_att edge_att;

extern trgb *color_table;
extern int block;

extern TList *asf_list;

extern tpoint *point_list;
extern int npoints;

extern CGM_FUNC *funcs;

extern int(*cdsizecb)(cdContext *driver, int w, int h, double w_mm, double h_mm);

extern int(*cdcgmbegmtfcb)(cdContext *driver, int *xmin, int *ymin, int *xmax, int *ymax);

extern int(*cdcgmcountercb)(cdContext *driver, double size);

extern int(*cdcgmsclmdecb)(cdContext *driver, short scl_mode, short *drw_mode, double *factor);

extern int(*cdcgmvdcextcb)(cdContext *driver, short type, double *xmn, double *ymn, double *xmx, double *ymx);

extern int(*cdcgmbegpictcb)(cdContext *driver, char *pict );

extern int(*cdcgmbegpictbcb)(cdContext *driver, double scale_x, double scale_y, 
                             double vdc_x2pix, double vdc_y2pix, double vdc_x2mm,
					                   double vdc_y2mm, int drw_mode,
					                   double xmin, double ymin, double xmax, double ymax );

#endif

typedef struct _tasf {
                     short type;
                     short value;
                    } tasf;

typedef struct _pat_table {
                           long index;
                           long nx, ny;
                           tcolor *pattern;
                          } pat_table;


int begmtf ( void ); /* begin metafile */
int endmtf ( void ); /* end metafile */
int begpic ( void ); /* begin picture */
int begpib ( void ); /* begin picture body */
int endpic ( void ); /* end picture */
int mtfver ( void ); /* metafile version */
int mtfdsc ( void ); /* metafile description */
int vdctyp ( void ); /* vdc type */
int intpre ( void ); /* integer precision */
int realpr ( void ); /* real precision */
int indpre ( void ); /* index precision */
int colpre ( void ); /* colour precision */
int colipr ( void ); /* colour index precision */
int maxcoi ( void ); /* maximum colour index */
int covaex ( void ); /* colour value extent */
int mtfell ( void ); /* metafile element list */
int bmtfdf (void );  /* begin metafile defaults */
int emtfdf ( void ); /* end metafile defaults */
int fntlst ( void ); /* font list */
int chslst ( void ); /* character set list */
int chcdac ( void ); /* character coding announcer */
int sclmde ( void ); /* scaling mode */
int clslmd ( void ); /* colour selection mode */
int lnwdmd ( void ); /* line width specification mode */
int mkszmd ( void ); /* marker size specification mode */
int edwdmd ( void ); /* edge width specification mode */
int vdcext ( void ); /* vdc extent */
int bckcol ( void ); /* backgound colour */
int vdcipr ( void ); /* vdc integer precision */
int vdcrpr ( void ); /* vdc real precision */
int auxcol ( void ); /* auxiliary colour */
int transp ( void ); /* transparency */
int clprec ( void ); /* clip rectangle */
int clpind ( void ); /* clip indicator */
int polyln ( void ); /* polyline */
int incply ( void ); /* incremental polyline */
int djtply ( void ); /* disjoint polyline */
int indjpl ( void ); /* incremental disjoint polyline */
int polymk ( void ); /* polymarker */
int incplm ( void ); /* incremental polymarker */
int text ( void );   /* text */
int rsttxt ( void ); /* restricted text */
int apdtxt ( void ); /* append text */
int polygn ( void ); /* polygon */
int incplg ( void ); /* incremental polygon */
int plgset ( void ); /* polygon set */
int inpgst ( void ); /* incremental polygon set */
int cellar ( void ); /* cell array */
int gdp ( void );    /* generalized drawing picture */
int rect ( void );   /* rectangle */
int circle ( void ); /* circle */
int circ3p ( void ); /* circular arc 3 point */
int cir3pc ( void ); /* circular arc 3 point close */
int circnt ( void ); /* circular arc centre */
int ccntcl ( void ); /* circular arc centre close */
int ellips ( void ); /* ellipse */
int ellarc ( void ); /* elliptical arc */
int ellacl ( void ); /* elliptical arc close */
int lnbdin ( void ); /* line bundle index */
int lntype ( void ); /* line type */
int lnwidt ( void ); /* line width */
int lncolr ( void ); /* line colour */
int mkbdin ( void ); /* marker bundle index */
int mktype ( void ); /* marker type */
int mksize ( void ); /* marker size */
int mkcolr ( void ); /* marker colour */
int txbdin ( void ); /* text bundle index */
int txftin ( void ); /* text font index */
int txtprc ( void ); /* text precision */
int chrexp ( void ); /* char expansion factor */
int chrspc ( void ); /* char spacing */
int txtclr ( void ); /* text colour */
int chrhgt ( void ); /* char height */
int chrori ( void ); /* char orientation */
int txtpat ( void ); /* text path */
int txtali ( void ); /* text alignment */
int chseti ( void ); /* character set index */
int achsti ( void ); /* alternate character set index */
int fillin ( void ); /* fill bundle index */
int intsty ( void ); /* interior style */
int fillco ( void ); /* fill colour */
int hatind ( void ); /* hatch index */
int patind ( void ); /* pattern index */
int edgind ( void ); /* edge bundle index */
int edgtyp( void );  /* edge type */
int edgwid ( void ); /* edge width */
int edgcol ( void ); /* edge colour */
int edgvis ( void ); /* edge visibility */
int fillrf ( void ); /* fill reference point */
int pattab ( void ); /* pattern table */
int patsiz ( void ); /* pattern size */
int coltab ( void ); /* colour table */
int asf ( void );    /* asfs */
int escape ( void ); /* escape */
int messag ( void ); /* message */
int appdta ( void ); /* application data */
