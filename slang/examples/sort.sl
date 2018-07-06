#! /usr/bin/env slsh

% This program presents the solution to a problem posed by 
% Tom Christiansen <tchrist@mox.perl.com>.  The problem reads:
%
%    Sort an input file that consists of lines like this
%
%        var1=23 other=14 ditto=23 fred=2
%
%    such that each output line is sorted WRT to the number.  Order
%    of output lines does not change.  Resolve collisions using the
%    variable name.   e.g.
%
%        fred=2 other=14 ditto=23 var1=23 
%
%    Lines may be up to several kilobytes in length and contain
%    zillions of variables.
%---------------------------------------------------------------------------
%
% The solution presented below works by breaking up the line into an
% array of alternating keywords and values with the keywords as the even
% elements and the values as the odd.

static variable Keys, Values;
static define sort_fun (i, j)
{
   variable s, a, b;

   s = Values[i] - Values[j];
   if (s) return s;
   return strcmp (Keys[i], Keys[j]);
}


define main ()
{
   variable line, len, i;
   while (-1 != fgets (&line, stdin))
     {
	line = strtok (line, " \t\n=");
	len = length(line)/2;
	if (len == 0)
	  continue;

	% Even elements are keys, odd are values
	Keys = line[[0::2]];
	Values = array_map(Int_Type, &integer, line[[1::2]]);
	
	foreach (array_sort ([0:len-1], &sort_fun))
	  {
	     i = ();
	     () = fprintf (stdout, "%s=%d ", Keys[i], Values[i]);
	  }
	() = fputs ("\n", stdout);
     }
}

main ();
