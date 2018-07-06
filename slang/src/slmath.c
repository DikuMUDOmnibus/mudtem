/* sin, cos, etc, for S-Lang */
/* Copyright (c) 1992, 1999 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include <math.h>

#include "slang.h"
#include "_slang.h"

#ifdef PI
# undef PI
#endif
#define PI 3.14159265358979323846264338327950288

#ifndef HAVE_STDLIB_H
  extern double atof ();
#endif

#if defined(__unix__)
#include <signal.h>
#include <errno.h>

#define SIGNAL  SLsignal

static void math_floating_point_exception (int sig)
{
   sig = errno;
   SLang_Error = SL_INTRINSIC_ERROR;
   (void) SIGNAL (SIGFPE, math_floating_point_exception);
   errno = sig;
}
#endif

double SLmath_hypot (double x, double y)
{
   double fr, fi, ratio;

   fr = fabs(x);
   fi = fabs(y);

   if (fr > fi)
     {
	ratio = y / x;
	x = fr * sqrt (1.0 + ratio * ratio);
     }
   else if (fi == 0.0) x = 0.0;
   else
     {
	ratio = x / y;
	x = fi * sqrt (1.0 + ratio * ratio);
     }

   return x;
}

/* usage here is a1 a2 ... an n x ==> a1x^n + a2 x ^(n - 1) + ... + an */
static double math_poly (void)
{
   int n;
   double xn = 1.0, sum = 0.0;
   double an, x;

   if ((SLang_pop_double(&x, NULL, NULL))
       || (SLang_pop_integer(&n))) return(0.0);

   while (n-- > 0)
     {
	if (SLang_pop_double(&an, NULL, NULL)) break;
	sum += an * xn;
	xn = xn * x;
     }
   return (double) sum;
}

static int double_math_op_result (int op, unsigned char a, unsigned char *b)
{
   (void) op;

   if (a != SLANG_FLOAT_TYPE)
     *b = SLANG_DOUBLE_TYPE;
   else
     *b = a;

   return 1;
}

static int double_math_op (int op,
			   unsigned char type, VOID_STAR ap, unsigned int na,
			   VOID_STAR bp)
{
   double *a, *b;
   unsigned int i;

   (void) type;
   a = (double *) ap;
   b = (double *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	for (i = 0; i < na; i++)
	  b[i] = sinh (a[i]);
	break;

#ifdef HAVE_ASINH
      case SLMATH_ASINH:
	for (i = 0; i < na; i++)
	  b[i] = asinh (a[i]);
	break;
#endif

      case SLMATH_COSH:
	for (i = 0; i < na; i++)
	  b[i] = cosh (a[i]);
	break;

#ifdef HAVE_ACOSH
      case SLMATH_ACOSH:
	for (i = 0; i < na; i++)
	  b[i] = acosh (a[i]);
	break;
#endif
      case SLMATH_TANH:
	for (i = 0; i < na; i++)
	  b[i] = tanh (a[i]);
	break;
#ifdef HAVE_ATANH
      case SLMATH_ATANH:
	for (i = 0; i < na; i++)
	  b[i] = atanh (a[i]);
	break;
#endif
      case SLMATH_TAN:
	for (i = 0; i < na; i++)
	  b[i] = tan (a[i]);
	break;

      case SLMATH_ASIN:
	for (i = 0; i < na; i++)
	  b[i] = asin (a[i]);
	break;

      case SLMATH_ACOS:
	for (i = 0; i < na; i++)
	  b[i] = acos (a[i]);
	break;

      case SLMATH_ATAN:
	for (i = 0; i < na; i++)
	  b[i] = atan (a[i]);
	break;

      case SLMATH_EXP:
	for (i = 0; i < na; i++)
	  b[i] = exp (a[i]);
	break;

      case SLMATH_LOG:
	for (i = 0; i < na; i++)
	  b[i] = log (a[i]);
	break;

      case SLMATH_LOG10:
	for (i = 0; i < na; i++)
	  b[i] = log10 (a[i]);
	break;

      case SLMATH_SQRT:
	for (i = 0; i < na; i++)
	  b[i] = sqrt (a[i]);
	break;

      case SLMATH_SIN:
	for (i = 0; i < na; i++)
	  b[i] = sin (a[i]);
	break;

      case SLMATH_COS:
	for (i = 0; i < na; i++)
	  b[i] = cos (a[i]);
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[i];
	break;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	break;

     }

   return 1;
}

static int float_math_op (int op,
			  unsigned char type, VOID_STAR ap, unsigned int na,
			  VOID_STAR bp)
{
   float *a, *b;
   unsigned int i;

   (void) type;
   a = (float *) ap;
   b = (float *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	for (i = 0; i < na; i++)
	  b[i] = (float)sinh (a[i]);
	break;

#ifdef HAVE_ASINH
      case SLMATH_ASINH:
	for (i = 0; i < na; i++)
	  b[i] = (float)asinh (a[i]);
	break;
#endif

      case SLMATH_COSH:
	for (i = 0; i < na; i++)
	  b[i] = (float)cosh (a[i]);
	break;

#ifdef HAVE_ACOSH
      case SLMATH_ACOSH:
	for (i = 0; i < na; i++)
	  b[i] = (float)acosh (a[i]);
	break;
#endif
      case SLMATH_TANH:
	for (i = 0; i < na; i++)
	  b[i] = (float)tanh (a[i]);
	break;
#ifdef HAVE_ATANH
      case SLMATH_ATANH:
	for (i = 0; i < na; i++)
	  b[i] = (float)atanh (a[i]);
	break;
#endif
      case SLMATH_TAN:
	for (i = 0; i < na; i++)
	  b[i] = (float)tan (a[i]);
	break;

      case SLMATH_ASIN:
	for (i = 0; i < na; i++)
	  b[i] = (float)asin (a[i]);
	break;

      case SLMATH_ACOS:
	for (i = 0; i < na; i++)
	  b[i] = (float)acos (a[i]);
	break;

      case SLMATH_ATAN:
	for (i = 0; i < na; i++)
	  b[i] = (float)atan (a[i]);
	break;

      case SLMATH_EXP:
	for (i = 0; i < na; i++)
	  b[i] = (float)exp (a[i]);
	break;

      case SLMATH_LOG:
	for (i = 0; i < na; i++)
	  b[i] = (float)log (a[i]);
	break;

      case SLMATH_LOG10:
	for (i = 0; i < na; i++)
	  b[i] = (float)log10 (a[i]);
	break;

      case SLMATH_SQRT:
	for (i = 0; i < na; i++)
	  b[i] = (float)sqrt (a[i]);
	break;

      case SLMATH_SIN:
	for (i = 0; i < na; i++)
	  b[i] = (float)sin (a[i]);
	break;

      case SLMATH_COS:
	for (i = 0; i < na; i++)
	  b[i] = (float)cos (a[i]);
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = (float)a[i];
	break;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = (float)0.0;
	break;

     }

   return 1;
}

static int generic_math_op (int op,
			    unsigned char type, VOID_STAR ap, unsigned int na,
			    VOID_STAR bp)
{
   double *b;
   unsigned int i;
   SLang_To_Double_Fun_Type to_double;
   unsigned int da;
   char *a;

   if (NULL == (to_double = SLarith_get_to_double_fun (type, &da)))
     return 0;

   b = (double *) bp;
   a = (char *) ap;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = sinh (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;
#ifdef HAVE_ASINH
      case SLMATH_ASINH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = asinh (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;
#endif
      case SLMATH_COSH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = cosh (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;
#ifdef HAVE_ACOSH
      case SLMATH_ACOSH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = acosh (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;
#endif
      case SLMATH_TANH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = tanh (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;
#ifdef HAVE_ATANH
      case SLMATH_ATANH:
	for (i = 0; i < na; i++)
	  {
	     b[i] = atanh (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;
#endif
      case SLMATH_TAN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = tan (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_ASIN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = asin (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_ACOS:
	for (i = 0; i < na; i++)
	  {
	     b[i] = acos (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_ATAN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = atan (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_EXP:
	for (i = 0; i < na; i++)
	  {
	     b[i] = exp (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_LOG:
	for (i = 0; i < na; i++)
	  {
	     b[i] = log (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_LOG10:
	for (i = 0; i < na; i++)
	  {
	     b[i] = log10 (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_SQRT:
	for (i = 0; i < na; i++)
	  {
	     b[i] = sqrt (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_SIN:
	for (i = 0; i < na; i++)
	  {
	     b[i] = sin (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_COS:
	for (i = 0; i < na; i++)
	  {
	     b[i] = cos (to_double((VOID_STAR) a));
	     a += da;
	  }
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  {
	     b[i] = to_double((VOID_STAR) a);
	     a += da;
	  }
	break;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	break;
     }

   return 1;
}

#if SLANG_HAS_COMPLEX
static int complex_math_op_result (int op, unsigned char a, unsigned char *b)
{
   (void) a;
   switch (op)
     {
      default:
	*b = SLANG_COMPLEX_TYPE;
	break;

      case SLMATH_REAL:
      case SLMATH_IMAG:
	*b = SLANG_DOUBLE_TYPE;
	break;
     }
   return 1;
}

static int complex_math_op (int op,
			    unsigned char type, VOID_STAR ap, unsigned int na,
			    VOID_STAR bp)
{
   double *a, *b;
   unsigned int i;
   unsigned int na2 = na * 2;

   (void) type;
   a = (double *) ap;
   b = (double *) bp;

   switch (op)
     {
      default:
      case SLMATH_ATANH:
      case SLMATH_ACOSH:
      case SLMATH_ASINH:
	return 0;

      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[2 * i];
	break;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = a[2 * i + 1];
	break;

      case SLMATH_EXP:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_exp (b + i, a + i);
	break;

      case SLMATH_LOG:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_log (b + i, a + i);
	break;

      case SLMATH_LOG10:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_log10 (b + i, a + i);
	break;

      case SLMATH_SQRT:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_sqrt (b + i, a + i);
	break;

      case SLMATH_SIN:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_sin (b + i, a + i);
	break;

      case SLMATH_COS:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_cos (b + i, a + i);
	break;

      case SLMATH_SINH:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_sinh (b + i, a + i);
	break;

      case SLMATH_COSH:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_cosh (b + i, a + i);
	break;

      case SLMATH_TANH:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_tanh (b + i, a + i);
	break;

      case SLMATH_TAN:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_tan (b + i, a + i);
	break;

      case SLMATH_ASIN:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_asin (b + i, a + i);
	break;

      case SLMATH_ACOS:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_acos (b + i, a + i);
	break;

      case SLMATH_ATAN:
	for (i = 0; i < na2; i += 2)
	  SLcomplex_atan (b + i, a + i);
	break;

      case SLMATH_CONJ:
	for (i = 0; i < na2; i += 2)
	  {
	     b[i] = a[i];
	     b[i+1] = -a[i+1];
	  }
	break;
     }

   return 1;
}
#endif

static SLang_DConstant_Type DConst_Table [] =
{
   MAKE_DCONSTANT("E", 2.718281828459045),
   MAKE_DCONSTANT("PI", 3.14159265358979323846264338327950288),
   SLANG_END_TABLE
};

static SLang_Math_Unary_Type SLmath_Table [] =
{
   MAKE_MATH_UNARY("sinh", SLMATH_SINH),
   MAKE_MATH_UNARY("asinh", SLMATH_ASINH),
   MAKE_MATH_UNARY("cosh", SLMATH_COSH),
   MAKE_MATH_UNARY("acosh", SLMATH_ACOSH),
   MAKE_MATH_UNARY("tanh", SLMATH_TANH),
   MAKE_MATH_UNARY("atanh", SLMATH_ATANH),
   MAKE_MATH_UNARY("sin", SLMATH_SIN),
   MAKE_MATH_UNARY("cos", SLMATH_COS),
   MAKE_MATH_UNARY("tan", SLMATH_TAN),
   MAKE_MATH_UNARY("atan", SLMATH_ATAN),
   MAKE_MATH_UNARY("acos", SLMATH_ACOS),
   MAKE_MATH_UNARY("asin", SLMATH_ASIN),
   MAKE_MATH_UNARY("exp", SLMATH_EXP),
   MAKE_MATH_UNARY("log", SLMATH_LOG),
   MAKE_MATH_UNARY("sqrt", SLMATH_SQRT),
   MAKE_MATH_UNARY("log10", SLMATH_LOG10),
#if SLANG_HAS_COMPLEX
   MAKE_MATH_UNARY("Real", SLMATH_REAL),
   MAKE_MATH_UNARY("Imag", SLMATH_IMAG),
   MAKE_MATH_UNARY("Conj", SLMATH_CONJ),
#endif
   SLANG_END_TABLE
};

static SLang_Intrin_Fun_Type SLang_Math_Table [] =
{
   MAKE_INTRINSIC_0("polynom", math_poly, SLANG_DOUBLE_TYPE),
   SLANG_END_TABLE
};

int SLang_init_slmath (void)
{
   unsigned char *int_types;

#if defined(__unix__)
   (void) SIGNAL (SIGFPE, math_floating_point_exception);
#endif

   int_types = _SLarith_Arith_Types;

   while (*int_types != SLANG_FLOAT_TYPE)
     {
	if (-1 == SLclass_add_math_op (*int_types, generic_math_op, double_math_op_result))
	  return -1;
	int_types++;
     }

   if ((-1 == SLclass_add_math_op (SLANG_FLOAT_TYPE, float_math_op, double_math_op_result))
       || (-1 == SLclass_add_math_op (SLANG_DOUBLE_TYPE, double_math_op, double_math_op_result))
#if SLANG_HAS_COMPLEX
       || (-1 == SLclass_add_math_op (SLANG_COMPLEX_TYPE, complex_math_op, complex_math_op_result))
#endif
       )
     return -1;

   if ((-1 == SLadd_math_unary_table (SLmath_Table, "__SLMATH__"))
       || (-1 == SLadd_intrin_fun_table (SLang_Math_Table, NULL))
       || (-1 == SLadd_dconstant_table (DConst_Table, NULL)))
     return -1;

   return 0;
}

