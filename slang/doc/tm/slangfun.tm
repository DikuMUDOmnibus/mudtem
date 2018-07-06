#c -*- mode: textmac; mode: fold; eval: .0 =TAB -*-
#%{{{Macros 

#i linuxdoc.tm

#d slang \bf{S-lang}
#d kw#1 \tt{$1}
#d exmp#1 \tt{$1}
#d var#1 \tt{$1}
#d ldots ...
#d chapter#1 <chapt>$1<p>
#d preface <preface>
#d tag#1 <tag>$1</tag>

#d function#1 \section{<bf>$1</bf>\label{$1}}<descrip>
#d variable#1 \section{<bf>$1</bf>\label{$1}}<descrip>
#cd function#1 <p><bf>$1</bf>\label{$1}<p><descrip>
#d synopsis#1 <tag> Synopsis </tag> $1
#d keywords#1 <tag> Keywords </tag> $1
#d usage#1 <tag> Usage </tag> <tt>$1</tt>
#d description <tag> Description </tag>
#d example <tag> Example </tag>
#d notes <tag> Notes </tag>
#d seealso#1 <tag> See Also </tag> <tt>$1</tt>
#d r#1 \ref{$1}{$1}
#d done </descrip><p>
#d -1 <tt>-1</tt>
#d 0 <tt>0</tt>
#d 1 <tt>1</tt>
#d NULL <tt>NULL</tt>
#d documentstyle book
#d sect1 \chapter
#d sect2 \section
#d sect3 \subsection
#d sect4 \subsubsection


#d user-manual \bf{A Guide to the S-Lang Language}


#%}}}

\linuxdoc
\begin{\documentstyle}

\title S-Lang Run-Time Library Reference: Version 1.4.0
\author John E. Davis, \tt{davis@space.mit.edu}
\date \__today__

\toc

\sect1{Array Functions}
#i rtl/array.tm

\sect1{Associative Array Functions}
#i rtl/assoc.tm

\sect1{Functions that Operate on Strings}
#i rtl/strops.tm

\sect1{Functions that Manipulate Structures}
#i rtl/struct.tm

\sect1{Informational Functions}
#i rtl/info.tm

\sect1{Mathematical Functions}
#i rtl/math.tm

\sect1{Message and Error Functions}
#i rtl/message.tm

\sect1{Time and Date Functions}
#i rtl/time.tm

\sect1{Data-Type Conversion Functions}
#i rtl/type.tm

\sect1{Stdio File I/O Functions}
#i rtl/stdio.tm

\sect1{Low-level POSIX I/O functions}
#i rtl/posio.tm

\sect1{Directory Functions}
#i rtl/dir.tm

\sect1{Functions that parse pathnames}
#i rtl/ospath.tm

\sect1{System Call Functions}
#i rtl/posix.tm

\sect1{Eval Functions}
#i rtl/eval.tm

\sect1{Module Functions}
#i rtl/import.tm

\sect1{Debugging Functions}
#i rtl/debug.tm

\sect1{Stack Functions}
#i rtl/stack.tm

\sect1{Miscellaneous Functions}
#i rtl/misc.tm

\end{\documentstyle}
