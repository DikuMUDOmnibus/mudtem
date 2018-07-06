#include "include.h"
#include <sys/types.h>

#include <FPL/FPL.h>
#include <FPL/reference.h>
#include <FPL/FPL_protos.h>

#define REG(x)
#define CALLER
#define ASM

void * fplAnchor;

long ASM fplInterfaceFunction(REG(a0) struct fplArgument *);

enum variables
{
	VAR_BLAH
};

enum funciones
{
	FN_OUTPUT,
	FN_SEND_TO_CHAR
};

void inicializar_fpl( void )
{
    long Version_I, Revision_I;

    fplAnchor = fplInitTags( fplInterfaceFunction, FPLTAG_DONE );

    fplSendTags( fplAnchor,	FPLSEND_GETVERSION, &Version_I,
				FPLSEND_GETREVISION, &Revision_I, FPLSEND_DONE);
    flog("Usando FPL version %d.%d.", Version_I, Revision_I);

    fplAddFunction( fplAnchor, "output", FN_OUTPUT, FPL_INTARG, "O", NULL);
    fplAddFunction( fplAnchor, "send_to_char", FN_SEND_TO_CHAR, FPL_INTARG, "S", NULL);

    fplAddVariable( fplAnchor, "blah", VAR_BLAH, FPL_INTARG, (void *) 99, NULL );
}

long ASM fplInterfaceFunction(REG(a0) struct fplArgument *arg)
{
	char * string;

	switch(arg->ID)
	{
		case FN_SEND_TO_CHAR:
		break;

		case FN_OUTPUT:
		if(arg->format[0]==FPL_STRARG)  /* we got a string! */
			string="%s";
		else
			string="%d";
		fprintf(stderr, string, arg->argv[0]);
		break;

		case FPL_GENERAL_ERROR:
		{
			char buffer[FPL_ERRORMSG_LENGTH];
			int col, ret;
			char *name;

			fplSendTags(fplAnchor,
				FPLSEND_GETVIRLINE, &col,
				FPLSEND_GETVIRFILE, &name,
				FPLSEND_DONE);
			if(*name=='\"')
			{
				ret=0;
				name++;
				while(name[ret] && name[ret]!='\"')
					ret++;
				string=(char *)fplAlloca(fplAnchor, ret+1);
				memcpy(string, name, ret);
				string[ret]='\0';
			}
			else
			{
				string=name;
				ret=0;
 			}
			printf("\n>>> %s\n",
				fplGetErrorMsg(arg->key, (long)arg->argv[0], buffer));
			printf(">>> Line %d in file \"%s\". <<<\n", col, string);
			if(ret)
				fplDealloca(fplAnchor, string);
		}
		break;
	}

	return FPL_OK;
}

DO_FUN_DEC( do_temp )
{
	char * script [] = { argument, NULL };

	fplExecuteScript( fplAnchor, script, 1, NULL );

	return;
}
