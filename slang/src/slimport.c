/* Copyright (c) 1998, 1999 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

#define SLANG_HAS_DYNAMIC_LINKING 1

#ifndef HAVE_DLFCN_H
# undef SLANG_HAS_DYNAMIC_LINKING
# define SLANG_HAS_DYNAMIC_LINKING	0
#endif

/* The rest of this file is in the if block */
#if SLANG_HAS_DYNAMIC_LINKING

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

static char *Module_Path;
#define MODULE_PATH_ENV_NAME	"SLANG_MODULE_PATH"

typedef struct _Handle_Type
{
   struct _Handle_Type *next;
   char *name;
   VOID_STAR handle;
}
Handle_Type;

static Handle_Type *Handle_List;

static void delete_handles (void)
{
   while (Handle_List != NULL)
     {
	Handle_Type *next = Handle_List->next;

	(void) dlclose (Handle_List->handle);
	SLang_free_slstring (Handle_List->name);
	SLfree ((char *)Handle_List);
	Handle_List = next;
     }
}

static Handle_Type *save_handle (char *name, VOID_STAR h)
{
   Handle_Type *l;

   l = (Handle_Type *) SLmalloc (sizeof (Handle_Type));
   if (l == NULL)
     return NULL;
   memset ((char *) l, 0, sizeof(Handle_Type));
   if (NULL == (l->name = SLang_create_slstring (name)))
     {
	SLfree ((char *) l);
	return NULL;
     }
   l->handle = h;
   l->next = Handle_List;
   Handle_List = l;

   return l;
}

static Handle_Type *find_handle (char *name)
{
   Handle_Type *l;

   l = Handle_List;
   while (l != NULL)
     {
	if (0 == strcmp (l->name, name))
	  break;
	l = l->next;
     }
   return l;
}

static int import_from_library (char *name, char *file)
{
   VOID_STAR *handle;
   int (*init_fun) (void);
   char *err;
   char filebuf[1024];

   if (NULL != find_handle (name))
     return 0;			       /* already loaded */

   while (1)
     {
#ifndef RTLD_GLOBAL
# define RTLD_GLOBAL 0
#endif
#ifdef RTLD_NOW
	handle = (VOID_STAR) dlopen (file, RTLD_NOW | RTLD_GLOBAL);
#else
	handle = (VOID_STAR) dlopen (file, RTLD_LAZY | RTLD_GLOBAL);
#endif

	if (handle != NULL)
	  break;

	if (NULL == strchr (file, '/'))
	  {
	     _SLsnprintf (filebuf, sizeof (filebuf), "./%s", file);
	     file = filebuf;
	     continue;
	  }

	if (NULL == (err = (char *) dlerror ()))
	  err = "UNKNOWN";

	SLang_verror (SL_INTRINSIC_ERROR,
		      "Error linking to %s: %s", file, err);
	return -1;
     }

   init_fun = (int (*)(void)) dlsym (handle, name);

   if (init_fun == NULL)
     {
	if (NULL == (err = (char *) dlerror ()))
	  err = "UNKNOWN";

	dlclose (handle);
	SLang_verror (SL_INTRINSIC_ERROR,
		      "Unable to get symbol %s from %s: %s",
		      name, file, err);
	return -1;
     }

   if (-1 == (*init_fun) ())
     {
	dlclose (handle);
	return -1;
     }

   (void) save_handle (name, handle);
   return 0;
}

static void import_module (char *module)
{
   char module_name[256];
   char symbol_name[256];
   char *path;
   char *file;

   _SLsnprintf (symbol_name, sizeof(symbol_name), "init_%s_module", module);
   _SLsnprintf (module_name, sizeof(module_name), "%s-module.so", module);

   if (Module_Path != NULL)
     file = SLpath_find_file_in_path (Module_Path, module_name);
   else file = NULL;

   if ((file == NULL)
       && (NULL != (path = getenv (MODULE_PATH_ENV_NAME))))
     file = SLpath_find_file_in_path (path, module_name);

   if (file != NULL)
     {
	(void) import_from_library (symbol_name, file);
	SLfree (file);
     }
   else
     {
	/* Maybe the system loader can find it in LD_LIBRARY_PATH */
	(void) import_from_library (symbol_name, module_name);
     }
}

static void set_import_module_path (char *path)
{
   (void) SLang_set_module_load_path (path);
}

static char *get_import_module_path (void)
{
   if (Module_Path == NULL) return "";
   return Module_Path;
}

static SLang_Intrin_Fun_Type Module_Intrins [] =
{
   MAKE_INTRINSIC_S("import", import_module, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("set_import_module_path", set_import_module_path, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("get_import_module_path", get_import_module_path, SLANG_STRING_TYPE),
   SLANG_END_TABLE
};

#endif				       /* SLANG_HAS_DYNAMIC_LINKING */

int SLang_set_module_load_path (char *path)
{
#if SLANG_HAS_DYNAMIC_LINKING
   if (NULL == (path = SLang_create_slstring (path)))
     return -1;
   SLang_free_slstring (Module_Path);
   Module_Path = path;
   return 0;
#else
   (void) path;
   return -1;
#endif
}

int SLang_init_import (void)
{
#if SLANG_HAS_DYNAMIC_LINKING
   (void) SLang_add_cleanup_function (delete_handles);
   return SLadd_intrin_fun_table (Module_Intrins, "__IMPORT__");
#else
   return 0;
#endif
}
