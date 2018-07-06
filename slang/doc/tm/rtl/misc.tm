\function{__get_reference}
\synopsis{Get a reference to a global object}
\usage{Ref_Type __get_reference (String_Type nm)}
\description
   This function returns a reference to a global variable or function
   whose name is specified by \var{nm}.  If no such object exists, it
   returns \var{NULL}, otherwise it returns a reference.
\example
    For example, consider the function:
#v+
    define runhooks (hook)
    {
       variable f;
       f = __get_reference (hook);
       if (f != NULL)
         @f ();
    }
#v-
    This function could be called from another \slang function to
    allow customization of that function, e.g., if the function
    represents a mode, the hook could be called to setup keybindings
    for the mode.
\seealso{is_defined, typeof, eval, autoload, __is_initialized}
\done

\function{getenv}
\synopsis{Get the value of an environment variable}
\usage{String_Type getenv(String_Type var)}
\description
   The \var{getenv} function returns a string that represents the
   value of an environment variable \var{var}.  It will return
   \var{NULL} if there is no environment variable whose name is given
   by \var{var}.
\example
#v+
    if (NULL != getenv ("USE_COLOR"))
      {
        set_color ("normal", "white", "blue");
        set_color ("status", "black", "gray");
        USE_ANSI_COLORS = 1;
      }
#v-
\seealso{putenv, strlen, is_defined}
\done

\function{implements}
\synopsis{Name a private namespace}
\usage{implements (String_Type name);}
\description
  The \var{implements} function may be used to name the private
  namespace associated with the current compilation unit.  Doing so
  will enable access to the members of the namespace from outside the
  unit.  The name of the global namespace is \exmp{Global}.
\example
  Suppose that some file \exmp{t.sl} contains:
#v+
     implements ("Ts_Private");
     static define message (x)
     {
        Global->vmessage ("Ts_Private message: %s", x);
     }
     message ("hello");
#v-
  will produce \exmp{"Ts_Private message: hello"}.  This \var{message}
  function may be accessed from outside via:
#v+
    Ts_Private->message ("hi");
#v-
\notes
  Since \var{message} is an intrinsic function, it is global and may
  not be redefined in the global namespace.
\done

\function{putenv}
\synopsis{Add or change an environment variable}
\usage{putenv (String_Type s)}
\description
    This functions adds string \var{s} to the environment.  Typically,
    \var{s} should of the form \var{"name=value"}.  The function
    signals a \slang error upon failure.
\notes
    This function is not available on all systems.
\seealso{getenv, sprintf}
\done

