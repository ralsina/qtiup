/** \file
 * \brief global attributes enviroment
 *
 * See Copyright Notice in iup.h
 * $Id: iglobalenv.c,v 1.2 2006/03/14 21:28:02 scuri Exp $
 */

#include <stdlib.h>              /* NULL */
#include <assert.h>

#include "iup.h"
#include "itable.h"
#include "iglobalenv.h"
#include "idrv.h"

static Itable *iglobalenv = NULL;

void iupGlobalEnvInit(void)
{
  iglobalenv = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupGlobalEnvFinish(void)
{
  iupTableDestroy(iglobalenv);
  iglobalenv = NULL;
}

void IupSetGlobal(const char *name, char *value)
{
  assert(name);
  if (!name) return;

  if (!iupdrvSetGlobal(name, value))
  {
    if (!value)
      iupTableRemove(iglobalenv, name);
    else
      iupTableSet(iglobalenv, name, value, IUP_POINTER);
  }
}

void IupStoreGlobal(const char *name, char *value)
{
  assert(name);
  if (!name) return;

  if (!iupdrvSetGlobal(name, value))
  {
    if (!value)
      iupTableRemove(iglobalenv, name);
    else
      iupTableSet(iglobalenv, name, value, IUP_STRING);
  }
}

char *IupGetGlobal(const char *name)
{
  char* v;
  assert(name);
  if (!name) return NULL;

  v = iupdrvGetGlobal(name);
  if (!v) v = (char*)iupTableGet(iglobalenv, name);

  return v;
}
