/* Setting this to 1 enables automatic support for associative arrays.
 * If this is set to 0, an application must explicitly enable associative
 * array support via SLang_init_slassoc.
 */
#define SLANG_HAS_ASSOC_ARRAYS	1

#define SLANG_HAS_COMPLEX	1
#define SLANG_HAS_FLOAT		1

#define _SLANG_OPTIMIZE_FOR_SPEED	1
#define _SLANG_USE_INLINE_CODE	1

/* This is experimental.  It adds extra information for tracking down
 * errors.
 */
#define _SLANG_HAS_DEBUG_CODE	1

/* Setting this to one will map 8 bit vtxxx terminals to 7 bit.  Terminals
 * such as the vt320 can be set up to output the two-character escape sequence
 * encoded as 'ESC [' as single character.  Setting this variable to 1 will
 * insert code to map such characters to the 7 bit equivalent.
 * This affects just input characters in the range 128-160 on non PC
 * systems.
 */
#if defined(VMS) || defined(AMIGA)
# define _SLANG_MAP_VTXXX_8BIT	1
#else
# define _SLANG_MAP_VTXXX_8BIT	0
#endif

/* Add support for color terminals that cannot do background color erases
 * Such terminals are poorly designed and are slowly disappearing but they
 * are still quite common.  For example, screen is one of them!
 * 
 * This is experimental.  In particular, it is not known to work if 
 * KANJI suupport is enabled.
 */
#if !defined(IBMPC_SYSTEM)
# define SLTT_HAS_NON_BCE_SUPPORT	1
#else
# define SLTT_HAS_NON_BCE_SUPPORT	0
#endif

/* Set this to 1 to enable Kanji support.  See above comment. */
#define SLANG_HAS_KANJI_SUPPORT 0

