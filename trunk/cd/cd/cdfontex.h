#ifndef _CD_FONTEX_
#define _CD_FONTEX_

#ifdef __cplusplus
extern "C" {
#endif

void cdTextSizeEx(char *s, int *width, int *height);
void cdFontDimEx(int *max_width, int *line_height, int *ascent, int *descent);
void cdFontEx(int type_face, int style, int size);

#ifdef __cplusplus
}
#endif

#endif 


