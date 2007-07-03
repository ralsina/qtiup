/** \file
 * \brief cdiuputil. CD and IUP utilities for the IupControls
 *
 * See Copyright Notice in iup.h
 * $Id: cdiuputil.h,v 1.5 2005/11/01 13:48:16 scuri Exp $
 */
 
#ifndef __CDIUPUTIL_H 
#define __CDIUPUTIL_H 

#ifdef __cplusplus
extern "C" {
#endif

void cdIupCalcShadows(long bgcolor, long *light_shadow, long *mid_shadow, long *dark_shadow);
long cdIupConvertColor(char *color);
char *cdIupGetParentBgColor (Ihandle* self);
char *cdIupGetBgColor(Ihandle* self);
void cdIupDrawSunkenRect(int x1, int y1, int x2, int y2,  
                         long light_shadow, long mid_shadow, long dark_shadow);
void cdIupDrawRaisenRect(int x1, int y1, int x2, int y2,  
                         long light_shadow, long mid_shadow, long dark_shadow);
void cdIupDrawVertSunkenMark(int x, int y1, int y2, long light_shadow, long dark_shadow);
void cdIupDrawHorizSunkenMark(int x1, int x2, int y, long light_shadow, long dark_shadow);
void cdIupDrawFocusRect(Ihandle* h, int x1, int y1, int x2, int y2);
char* cdIupSetFontBold(char* font, int bold);

#define cdIupInvertYAxis(_y, _h) ((_h) - (_y) - 1);


#ifdef __cplusplus
}
#endif

#endif
