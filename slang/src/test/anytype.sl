_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing Any_Type ...");

% Tests go here....

variable A = Any_Type[10];
if ((_typeof(A)) != Any_Type) failed ("_typeof");

static define eqs (a, b)
{
   variable len;
   len = length (a);
   if (len != length (b))
     return 0;
   
   len == length (where (a == b));
}

static define neqs (a, b)
{
   not (eqs (a, b));
}

static define check (a, i, value)
{
   a[i] = value;
   if (typeof (a[i]) != Any_Type)
     failed ("check typeof");
   % Because value can be an array, use neqs
   if (neqs(@a[i], value))
     failed ("a[i] = value for %S, computed: %S", value, @a[i]);
}

check (A, 0, "hello");
check (A, 0, 14);
check (A, 0, 2.3);
check (A, 0, &A);
check (A, 0, [1:10]);
check (A, 0, 1+2i);
check (A, 0, String_Type);

print ("Ok\n");

exit (0);

