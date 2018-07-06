_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing structures ...");

variable S = struct 
{
   a, b, c
};

S.a = "a";
S.b = "b";
S.c = "c";

variable T = @S;

if (S.a != T.a) failed ("Unable to copy via @S");
if (S.b != T.b) failed ("Unable to copy via @S");
if (S.c != T.c) failed ("Unable to copy via @S");

T.a = "XXX";
if (T.a == S.a) failed ("Unable to copy via @S");

set_struct_fields (T, 1, 2, "three");
if ((T.c != "three") or (T.a != 1) or (T.b != 2))
  failed ("set_struct_fields");

T.a++;
T.a += 3;
T.a -= 20;
if (T.a != -15) 
  failed ("structure arithmetic");

T.c = S;
S.a = T;

if (T != T.c.a)  
  failed ("Unable to create a circular list");

print ("Ok\n");

exit (0);

