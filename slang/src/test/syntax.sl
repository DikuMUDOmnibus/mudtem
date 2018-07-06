_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing syntax ...");

static define static_xxx ()
{
   return "xxx";
}

private define private_yyy ()
{
   return "yyy";
}

public define public_zzz ()
{
   return "zzz";
}

if (is_defined ("static_xxx") or "xxx" != static_xxx ())
  failed ("static_xxx");
if (is_defined ("private_yyy") or "yyy" != private_yyy ())
  failed ("private_yyy");
if (not is_defined ("public_zzz") or "zzz" != public_zzz ())
  failed ("public_xxx");

variable XXX = 1;
static define xxx ()
{
   variable XXX = 2;
   if (XXX != 2) failed ("local variable XXX");
}

xxx ();
if (XXX != 1) failed ("global variable XXX");
if (1)
{
   if (orelse
	{0}
	{0}
	{0}
	{0}
       ) 
     failed ("orelse");
}


!if (orelse
     {0}
     {0}
     {0}
     {1}) failed ("not orelse");




	

print ("Ok\n");

exit (0);

