_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing string functions...");

variable s;

s = strcompress (" \t  \tA\n\ntest\t", " \t\n");
if (s != "A test") failed ("strcompress");

s = " \t hello world\n\t";
if ("hello world" != strtrim (s)) failed ("strtrim");
if ("hello world\n\t" != strtrim_beg (s)) failed ("strtrim_beg");
if (" \t hello world" != strtrim_end (s)) failed ("strtrim_beg");

if ("hello wor" != strtrim (s, " \t\nld")) failed ("strtrim with whitespace");

if ("abcdefg" != strcat ("a", "b", "c", "d", "e", "f", "g")) failed ("strcat");
if ("abcdefg" != strcat ("abcdefg")) failed ("strcat 2");

if ((strtok (s)[0] != "hello") 
    or (strtok(s)[1] != "world")
    or (strtok (s, "^a-z")[0] != "hello")
    or (strtok (s, "^a-z")[1] != "world")
    or (2 != length (strtok (s)))
    or (2 != length (strtok (s, "^a-z")))) failed ("strtok");

define test_create_delimited_string ()
{
   variable n = ();
   variable args = __pop_args (_NARGS - 3);
   variable delim = ();
   variable eresult = ();
   variable result;
   
   result = create_delimited_string (delim, __push_args (args), n);
   if (eresult != result)
     failed ("create_delimited_string: expected: %s, got: %s",
	     eresult, result);

   if (n)
     result = strjoin ([__push_args (args)], delim);
   else 
     result = strjoin (String_Type[0], delim);

   if (eresult != result)
     failed ("strjoin: expected: %s, got: %s",
	     eresult, result);
}

	
test_create_delimited_string ("aXXbXXcXXdXXe",
			      "XX",
			      "a", "b", "c", "d", "e",
			      5);


test_create_delimited_string ("", "", "", 1);
test_create_delimited_string ("a", ",", "a", 1);
test_create_delimited_string (",", ",", "", "", 2);
test_create_delimited_string (",,", ",", "", "", "", 3);
test_create_delimited_string ("", "XXX", 0);


print ("Ok\n");
exit (0);
