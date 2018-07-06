#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <slang.h>

static char *Slsh_Version = "0.3";
#define SLSHRC_FILE "slsh.rc"

typedef struct _AtExit_Type
{
   SLang_Name_Type *nt;
   struct _AtExit_Type *next;
}
AtExit_Type;

static AtExit_Type *AtExit_Hooks;

static void at_exit (SLang_Ref_Type *ref)
{
   SLang_Name_Type *nt;
   AtExit_Type *a;

   if (NULL == (nt = SLang_get_fun_from_ref (ref)))
     return;

   a = (AtExit_Type *) SLmalloc (sizeof (AtExit_Type));
   if (a == NULL)
     return;

   a->nt = nt;
   a->next = AtExit_Hooks;
   AtExit_Hooks = a;
}

static void c_exit (int *code)
{
   while (AtExit_Hooks != NULL)
     {
	AtExit_Type *next = AtExit_Hooks->next;
	if (SLang_Error == 0)
	  (void) SLexecute_function (AtExit_Hooks->nt);

	SLfree ((char *) AtExit_Hooks);
	AtExit_Hooks = next;
     }
   exit (*code);
}

/* Create the Table that S-Lang requires */
static SLang_Intrin_Fun_Type Intrinsics [] =
{
   MAKE_INTRINSIC_I("exit", c_exit, VOID_TYPE),
   MAKE_INTRINSIC_1("atexit", at_exit, VOID_TYPE, SLANG_REF_TYPE),
   SLANG_END_TABLE
};

static int try_to_load_file (char *path, char *file)
{
   file = SLpath_find_file_in_path (path, file);
   if (file == NULL)
     return 0;
   if (0 == SLang_load_file (file))
     {
	SLfree (file);
	return 1;
     }
   SLfree (file);
   return -1;
}

static int load_startup_file (void)
{
   char *dir;

   dir = getenv ("SLSH_LIB_DIR");
   if (NULL == dir)
     {
#ifdef SLSH_LIB_DIR
	dir = SLSH_LIB_DIR;
	if (dir != NULL)
	  {
	     status = try_to_load_file (dir, SLSHRC_FILE);
	     if (status == -1)
	       return -1;
	     if (status == 1)
	       return 0;
	  }
#endif
	dir = "/usr/local/lib/slsh:/usr/local/slsh:/usr/lib/slsh:/etc/slsh";
     }

   if (-1 == try_to_load_file (dir, SLSHRC_FILE))
     return -1;

   return 0;
}

static int is_script (char *file)
{
   FILE *fp;
   char buf[3];
   int is;

   if (NULL == (fp = fopen (file, "r")))
     return 0;

   is = ((NULL != fgets (buf, sizeof(buf), fp))
	 && (buf[0] == '#') && (buf[1] == '!'));

   fclose (fp);
   return is;
}

static void usage (void)
{
   fprintf (stderr, "Usage: slsh [--help ] [ --version ] [FILENAME ...]\n");
   exit (1);
}

static void version (void)
{
   fprintf (stdout, "slsh version %s\n", Slsh_Version);
   fprintf (stdout, "S-Lang Library Version: %s\n", SLang_Version_String);
   if (SLANG_VERSION != SLang_Version)
     {
	fprintf (stdout, "\t** Note: This program was compiled against version %s.\n",
		 SLANG_VERSION_STRING);
     }
   exit (0);
}

int main (int argc, char **argv)
{
   char *file = NULL;

   if ((-1 == SLang_init_all ())
       || (-1 == SLang_init_import ()) /* dynamic linking */
       || (-1 == SLadd_intrin_fun_table (Intrinsics, NULL)))
     {
	fprintf(stderr, "Unable to initialize S-Lang.\n");
	return 1;
     }

   while (argc > 1)
     {
	if (0 == strcmp (argv[1], "--version"))
	  version ();

	if (0 == strcmp (argv[1], "--help"))
	  usage ();

	break;
     }

   if (argc == 1)
     {
	if (0 == isatty (fileno(stdin)))
	  file = NULL;
	else
	  usage ();
     }
   else
     {
	file = argv[1];
	if (is_script (file))
	  {
	     argv++;
	     argc--;
	  }
     }

   if (-1 == SLang_set_argc_argv (argc, argv))
     return 1;

   /* Turn on debugging */
   SLang_Traceback = 1;

   if (-1 == load_startup_file ())
     return SLang_Error;

   /* Now load an initialization file and exit */
   (void) SLang_load_file (file);

   return SLang_Error;
}
