_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing Arithmetic ...");

define check_typeof (expr, type)
{
   if (typeof (expr) != type)
     failed ("typeof " + string (type) + " found " + string (typeof(expr)));
}

define check_bool (i)
{
   check_typeof (i == i, Char_Type);
}

define check_result (i, j, k)
{
   if (k != i + j)
     failed (sprintf("%S + %S != %S", typeof (i), typeof(j), typeof(k)));
}

check_typeof('a', UChar_Type);
check_typeof(1h, Short_Type);
check_typeof(1hu, UShort_Type);
check_typeof(0x20hu, UShort_Type);
check_typeof(1, Integer_Type);
check_typeof(0x20, Integer_Type);
check_typeof(1u, UInteger_Type);
check_typeof(1LU, ULong_Type);
check_typeof(1f, Float_Type);
check_typeof(1e10f, Float_Type);
check_typeof(.1e10f, Float_Type);
check_typeof(.1e10, Double_Type);

check_typeof(~'a', UChar_Type);
check_typeof(~1h, Short_Type);
check_typeof(~1hu, UShort_Type);
check_typeof(~0x20hu, UShort_Type);
check_typeof(~1, Integer_Type);
check_typeof(~0x20, Integer_Type);
check_typeof(~1u, UInteger_Type);
check_typeof(~1LU, ULong_Type);

check_typeof ('a' + 'b', Integer_Type);
check_typeof (1h + 'b', Integer_Type);

if (Integer_Type == Short_Type) check_typeof (1hu + 'b', UInteger_Type);
else check_typeof (1hu + 'b', Integer_Type);

check_typeof (1u + 1, UInteger_Type);

if (Integer_Type == Long_Type) check_typeof (1u + 1L, ULong_Type);
else  check_typeof (1u + 1L, Long_Type);

check_typeof (1u + 1UL, ULong_Type);
check_typeof (1u + 1.0f, Float_Type);
check_typeof (1u + 1.0, Double_Type);

check_typeof ('c' * 1i, Complex_Type);
check_typeof (1h * 1i, Complex_Type);
check_typeof (1.0 * 1i, Complex_Type);
check_typeof (1i * 1i, Complex_Type);

check_bool ('a');
check_bool (1h);
check_bool (1hu);
check_bool (1);
check_bool (1u);
check_bool (1L);
check_bool (1LU);
check_bool (1f);
check_bool (1.0);
check_bool (1.0i);

check_typeof (Real(1), Double_Type);
check_typeof (Real('a'), Double_Type);
check_typeof (Real(1L), Double_Type);
check_typeof (Real(1f), Float_Type);
check_typeof (Real(1.0), Double_Type);

check_result (1, 1, 2);
check_result (1, 0x31, 50);
check_result (1, '1', 50);
check_result (1L, '1', 50L);
check_result (1L, 1h, 2L);
check_result (1, 1h, 2);
check_result (1h, '1', 50);
check_result (1u, 3, 4);
check_result (1UL, '\x3', 4UL);

print ("Ok\n");
exit (0);
