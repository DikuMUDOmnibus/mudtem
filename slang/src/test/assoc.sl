_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing Associative Arrays ...");

static define key_to_value (k)
{
   return "<<<" + k + ">>>";
}

static define value_to_key (v)
{
   strcompress (v, "<>");
}

static define add_to_x (x, k)
{
   x[k] = key_to_value(k);
}

static variable Default_Value = "****Default-Value****";

define setup (type)
{
   variable x = Assoc_Type [type, Default_Value];
   
   add_to_x (x, "foo");
   add_to_x (x, "bar");
   add_to_x (x, "silly");
   add_to_x (x, "cow");
   add_to_x (x, "dog");
   add_to_x (x, "chicken");

   return x;
}

static variable X;

% Test create/destuction of arrays
loop (20) X = setup (Any_Type);

loop (20) X = setup (String_Type);
   
static variable k, v;

foreach (X)
{
   (k, v) = ();
   if ((k != value_to_key(v)) or (v != key_to_value (k))
       or (X[k] != v))
     failed ("foreach");
}

foreach (X) using ("keys")
{
   k = ();
   if (X[k] != key_to_value (k))
     failed ("foreach using keys");
}

foreach (X) using ("keys", "values")
{
   (k, v) = ();
   if ((k != value_to_key(v)) or (v != key_to_value (k))
       or (X[k] != v))
     failed ("foreach using keys, values");
}

k = assoc_get_keys (X);
v = assoc_get_values (X);

static variable i;
_for (0, length(k)-1, 1)
{
   i = ();
   if (v[i] != X[k[i]])
     failed ("assoc_get_keys/values");
   assoc_delete_key (X, k[i]);
}

if (length (X) != 0)
  error ("assoc_delete_key failed");

if (X["*******************"] != Default_Value)
  failed ("default value");

print ("Ok\n");

exit (0);

