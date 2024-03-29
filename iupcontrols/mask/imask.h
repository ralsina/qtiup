/** \file
 * \brief imask internal functions
 *
 * See Copyright Notice in iup.h
 * $Id: imask.h,v 1.1 2006/02/04 21:10:17 uid20021 Exp $
 */
 
#ifndef __IMASK_H 
#define __IMASK_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _iMask
{
  char ch;
  int command;
  int next1;
  int next2;
} iMask;

typedef int (*iMaskMatchFunc) (char which_one, long next_pos, long capture_pos, char *text, void* user_data);

/** Parse the mask and if it is ok create and returns the internal structure.
 */
int iMaskParse(char* mask, iMask** imk);

/** Do the pattern matching on the given text.
 */
int iMaskMatch(char* text, iMask* imk, long start, iMaskMatchFunc mask_func, void* user_data, char *addchar, int icase);

/** Change a control character.
 */
int iMaskSetChar(int char_number, char new_char);

/* iMaskMatch error codes */
#define IMK_PARSE_OK     0    /* No error      */
#define IMK_NOMATCH     -1    /* no match      */
#define IMK_MEM_ERROR   -2    /* memory error  */
#define IMK_PARSE_ERROR -3    /* parser error  */
#define IMK_PARTIALMATCH -4   /* partial match */


#ifdef __cplusplus
}
#endif

#endif
