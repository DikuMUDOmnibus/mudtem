\function{array_info}
\synopsis{Returns information about an array}
\usage{(Array_Type, Integer_Type, DataType_Type) array_info (Array_Type a)}
\description
  The \var{array_info} function returns information about the array \var{a}.
  It returns three values: an 1-d integer array array specifying the
  size of each dimension of \var{a}, the number of dimensions of
  \var{a}, and the data type of \var{a}.
\example
  The \var{array_info} function may be used to find the number of rows
  of an array:
#v+
    define num_rows (a)
    {
       variable dims, num_dims, data_type;

       (dims, num_dims, data_type) = array_info (a);
       return dims [0];
    }
#v-
  For 1-d arrays, this information is more easily obtained from the
  \var{length} function.
\seealso{typeof, reshape, length, _reshape}
\done

\function{array_map}
\synopsis{Apply a function to each element of an array}
\usage{Array_Type array_map (type, func, arg0, ...)}
#v+
    DataType_Type type;
    Ref_Type func;
#v-
\description
  The \var{array_map} function may be used to apply a function to each
  element of an array and returns the result as an array of a
  specified type.  The \var{type} parameter indicates what kind of
  array should be returned and generally corresponds to the return
  type of the function.  The \var{arg0} parameter should be an array
  and is used to determine the dimensions of the resulting array.  If
  any subsequent arguments correspond to an array of the same size,
  then those array elements will be passed in parallel with the first
  arrays arguments.
\example
  The first example illustrates how to apply the \var{strlen} function
  to an array of strings:
#v+
     S = ["", "Train", "Subway", "Car"];
     L = array_map (Integer_Type, &strlen, S);
#v-
  This is equivalent to:
#v+
     S = ["", "Train", "Subway", "Car"];
     L = Integer_Type [length (S)];
     for (i = 0; i < length (S); i++) L[i] = strlen (S[i]);
#v-
  
  Now consider an example involving the \var{strcat} function:
#v+
     files = ["slang", "slstring", "slarray"];

     exts = ".c";
     cfiles = array_map (String_Type, &strcat, files, exts);
     % ==> cfiles = ["slang.c slstring.c slarray.c"];

     exts =  [".a",".b",".c"];
     xfiles = array_map (String_Type, &strcat, files, exts);
     % ==> xfiles = ["slang.a", "slstring.b", "slarray.c"];
#v-
\notes
  Many mathemetical functions already work transparantly on arrays.
  For example, the following two statements produce identical results:
#v+
     B = sin (A);
     B = array_map (Double_Type, &sin, A);
#v-
\seealso{array_info, strlen, strcat, sin}
\done

\function{array_sort}
\synopsis{Sort an array}
\usage{Array_Type array_sort (Array_Type a [, String_Type or Ref_Type f])}
\description
  \var{array_sort} sorts the array \var{a} into ascending order and
  returns an integer array that represents the result of the sort. If
  the optional second parameter \var{f} is present, the function
  specified by \var{f} will be used to compare elements of \var{a};
  otherwise, a built-in sorting function will be used.  

  If \var{f} is present, then it must be either a string representing
  the name of the comparison function, or a reference to the function.
  The sort function represented by \var{f} must be a \slang
  user-defined function that takes two arguments.  The function must
  return an integer that is less than zero if the first parameter is
  considered to be less than the second, zero if they are equal, and a
  value greater than zero if the first is greater than the second.

  If the comparision function is not specified, then a built-in comparison
  function appropriate for the data type will be used.  For example,
  if \var{a} is an array of character strings, then the sort will be
  preformed using \var{strcmp}.

  The integer array returned by this function is simply an index that
  indicates the order of the sorted array.  The input array \var{a} is
  not changed.
\example
  An array of strings may be sorted using the \var{strcmp} function
  since it fits the specification for the sorting function described
  above:
#v+
     variable A = String_Type [3];
     A[0] = "gamma"; A[1] = "alpha"; A[2] = "beta";

     variable I = array_sort (A, &strcmp);
#v-
  Alternatively, one may use
#v+
     variable I = array_sort (A);     
#v-
  to use the built-in comparison function.

  After the \var{array_sort} has executed, the variable \var{I} will
  have the values \exmp{[2, 0, 1]}.  This array can be used to
  re-shuffle the elements of \var{A} into the sorted order via the
  array index expression \exmp{A = A[I]}.
\seealso{strcmp}
\done

\function{init_char_array}
\synopsis{Initialize an array of characters}
\usage{init_char_array (Array_Type a, String_Type s)}
\description
  The \var{init_char_array} function may be used to initialize a
  character array \var{a} by setting the elements of the array
  \var{a} to the corresponding characters of the string \var{s}.
\example
  The statements
#v+
     variable a = Char_Type [10];
     init_char_array (a, "HelloWorld");
#v-
   creates an character array and initializes its elements to the
   characters in the string \exmp{"HelloWorld"}.
\notes
   The character array must be large enough to hold all the characters
   of the initialization string.
\seealso{bstring_to_array, strlen, strcat}
\done

\function{length}
\synopsis{Get the length of an object}
\usage{Integer_Type length (obj)}
\description
  The \var{length} function may be used to get information about the
  length of an object.  For simple scalar data-types, it returns \1.
  For arrays, it returns the total number of elements of the array.
\notes
  If \var{obj} is a string, \var{length} returns \1 because a
  \var{String_Type} object is considered to be a scalar.  To get the
  number of characters in a string, use the \var{strlen} function.
\seealso{array_info, typeof, strlen}
\done

\function{reshape}
\synopsis{Reshape an array}
\usage{reshape (Array_Type A, Array_Type I)}
\description
  The \var{reshape} function changes the size of \var{A} to have the size
  specified by the 1-d integer array \var{I}.  The elements of \var{I}
  specify the new dimensions of \var{A} and must be consistent with
  the number of elements \var{A}.
\example
  If \var{A} is a \var{100} element 1-d array, it can be changed to a
  2-d \var{20} by \var{5} array via
#v+
      reshape (A, [20, 5]);
#v-
  However, \exmp{reshape(A, [11,5])} will result in an error because
  the the \exmp{[11,5]} array specifies \exmp{55} elements.
\notes
  Since \var{reshape} modifies the shape of an array, and arrays are
  treated as references, then all references to the array will
  reference the new shape.  If this effect is unwanted, then use the 
  \var{_reshape} function instead.
\seealso{_reshape, array_info}
\done

\function{_reshape}
\synopsis{Copy an array to a new shape}
\usage{Array_Type _reshape (Array_Type A, Array_Type I)}
\description
  The \var{_reshape} function creates a copy of an array \var{A},
  reshapes it to the form specified by \var{I} and returns the result.
  The elements of \var{I} specify the new dimensions of the copy of
  \var{A} and must be consistent with the number of elements \var{A}.
\example
  If \var{A} is a \var{100} element 1-d array, a new array 2-d array of 
  size \var{20} by \var{5} may be created from the elements of \var{A}
  by
#v+
      A = _reshape (A, [20, 5]);
#v-
  In this example, the original array was no longer needed.  Hence, it
  is preferable to make use of the \var{__tmp} operator to avoid the
  creation of a new array, i.e.,
#v+
      A = _reshape (__tmp(A), [20,5]);
#v-
\notes
  The \var{reshape} function performs a similar function to
  \var{_reshape}.  In fact, the \var{_reshape} function could have been
  implemented via:
#v+
     define _reshape (a, i)
     {
        a = @a;     % Make a new copy
        reshape (a, i);
        return a;
     }
#v-
\seealso{reshape, array_info}
\done

\function{transpose}
\synopsis{Transpose a 2d array}
\usage{Array_Type transpose (Array_Type a)}
\description
  The \var{transpose} function returns the transpose of a specified
  array.  By definition, the transpose of an array, say one with
  elements \exmp{a[i,j,...k]} is an array whose elements are
  \exmp{a[k,...,j,i]}.
\seealso{_reshape, reshape, array_info}
\done

\function{where}
\synopsis{Get indices where an integer array is non-zero}
\usage{Array_Type where (Array_Type a)}
\description
  The \var{where} function examines an integer array \var{a} and
  returns a 2-d integer array whose rows are the indices of \var{a}
  where the corresponding element of \var{a} is non-zero.
\example
  Consider the following:
#v+
    variable X = [0.0:10.0:0.01];
    variable A = sin (X);
    variable I = where (A < 0.0);
    A[I] = cos (X) [I];
#v-
  Here the variable \var{X} has been assigned an array of doubles
  whose elements range from \exmp{0.0} through \exmp{10.0} in
  increments of \var{0.01}.  The second statement assigns \var{A} to
  an array whose elements are the \var{sin} of the elements of \var{X}.
  The third statement uses the where function to get the indices of
  the elements of \var{A} that are less than \var{0.0}.  Finally, the
  last statement substitutes into \var{A} the \var{cos} of the
  elements of \var{X} at the positions of \var{A} where the
  corresponding \var{sin} is less than \var{0}.  The end result is
  that the elements of \var{A} are a mixture of sines and cosines.
\seealso{array_info, sin, cos}
\done

