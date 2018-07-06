_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing NameSpace routines ...");

implements ("NSpace");
% From this point on, define and variable defaults to static 

define static_function ()
{
   "static_function";
}
variable static_variable = "static_variable";

public define public_function ()
{
   "public_function";
}
public variable public_variable = "public_variable";

private define private_function ()
{
   "private_function";
}
private variable private_variable = "private_variable";

!if (is_defined ("Global->public_function")) failed ("public_function");
!if (is_defined ("Global->public_variable")) failed ("public_variable");
!if (is_defined ("public_function")) failed ("public_function");
!if (is_defined ("public_variable")) failed ("public_variable");
!if (is_defined ("NSpace->static_function")) failed ("static_function");
!if (is_defined ("NSpace->static_variable")) failed ("static_variable");
if (is_defined ("NSpace->private_function")) failed ("private_function");
if (is_defined ("NSpace->private_variable")) failed ("private_variable");

if (static_variable != NSpace->static_variable) failed ("static_variable test");
if (public_variable != Global->public_variable) failed ("public_variable test");
if (private_variable != "private_variable") failed ("private_variable test");

print ("Ok\n");

exit (0);

