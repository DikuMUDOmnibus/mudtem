!if (is_defined ("__argv"))
{
   message ("You need a newer version of jed to run this script");
   quit_jed ();
}

if (__argc != 4)
{
   message ("Usage: jed -script fixtex.sl <filename>");
   quit_jed ();
}

variable file = __argv[3];
() = read_file (file);

% Patch up the >,< signs
bob ();
replace ("$<$", "<");
replace ("$>$", ">");

% Make the first chapter a preface
bob ();
if (bol_fsearch ("\\chapter{Preface}"))
{
   push_spot ();
   push_mark ();
   go_right (8); insert ("*");	       %  \chapter{ --> \chapter*{
   () = bol_fsearch ("\\chapter{");
   push_spot ();

   insert("\\tableofcontents\n");
   eol ();
   insert ("\n\\pagenumbering{arabic}");

   pop_spot ();
   narrow ();
   bob ();
   replace ("\\section{", "\\section*{");
   widen ();

   if (bol_bsearch ("\\tableofcontents"))
     delete_line ();
   
   pop_spot ();
   if (bol_bsearch ("\\maketitle"))
     insert ("\\pagenumbering{roman}\n");

}

save_buffer ();
quit_jed ();


