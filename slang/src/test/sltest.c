#include <stdio.h>
#include <slang.h>

static void c_exit (int *code)
{
   exit (*code);
}

static SLang_Intrin_Fun_Type Intrinsics [] =
{
   MAKE_INTRINSIC_I("exit", c_exit, VOID_TYPE),
   SLANG_END_TABLE
};


int main (int argc, char **argv)
{
   if (argc < 2)
     {
	fprintf (stderr, "Usage: %s FILE...\n", argv[0]);
	return 1;
     }
   
   if ((-1 == SLang_init_all ())
       || (-1 == SLadd_intrin_fun_table (Intrinsics, NULL)))
     return 1;
   
   SLang_Traceback = 1;

   if (-1 == SLang_set_argc_argv (argc, argv))
     return 1;

   if (-1 == SLang_load_file (argv[1]))
     return 1;
   
   return SLang_Error;
}

	
