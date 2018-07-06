_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing array functions ...");

static variable A = [0:23];

static variable B = transpose(A);
static variable dims;

(dims,,) = array_info (B);
if ((dims[0] != 1)
    or (dims[1] != 24))
  failed ("transpose ([0:23])");


reshape (A, [2,3,4]);

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


if ((A[0,0,0] != 0)
    or (A[0,0,1] != 1)
    or (neqs (A[0,0,[:]], [0:3]))
    or (neqs (A[0,1,[:]], [4:7]))
    or (neqs (A[0,2,[:]], [8:11]))
    or (neqs (A[1,0,[:]], [12:15]))
    or (neqs (A[1,1,[:]], [16:19]))
    or (neqs (A[1,2,[:]], [20:23]))) failed ("reshape");

B = transpose (A);

if ((B[0,0,0] != 0)
    or (B[1,0,0] != 1)
    or (neqs (B[[:],0,0], [0:3]))
    or (neqs (B[[:],1,0], [4:7]))
    or (neqs (B[[:],2,0], [8:11]))
    or (neqs (B[[:],0,1], [12:15]))
    or (neqs (B[[:],1,1], [16:19]))
    or (neqs (B[[:],2,1], [20:23]))) failed ("transpose int array");

% Test for memory leak
loop (100) B = transpose (B);
B = 0;

% Try on a string array
variable S = String_Type[length (A)];
foreach (A)
{
   variable i = ();
   S[i] = string (i);
}

variable T = @S;
reshape (S, [2,3,4]);

if ((S[0,0,0] != T[0])
    or (S[0,0,1] != T[1])
    or (neqs (S[0,0,*], T[[0:3]]))
    or (neqs (S[0,1,*], T[[4:7]]))
    or (neqs (S[0,2,*], T[[8:11]]))
    or (neqs (S[1,0,*], T[[12:15]]))
    or (neqs (S[1,1,*], T[[16:19]]))
    or (neqs (S[1,2,*], T[[20:23]]))) failed ("reshape string array");

S = transpose (S);

if ((S[0,0,0] != T[0])
    or (S[1,0,0] != T[1])
    or (neqs (S[*,0,0], T[[0:3]]))
    or (neqs (S[*,1,0], T[[4:7]]))
    or (neqs (S[*,2,0], T[[8:11]]))
    or (neqs (S[*,0,1], T[[12:15]]))
    or (neqs (S[*,1,1], T[[16:19]]))
    or (neqs (S[*,2,1], T[[20:23]]))) failed ("transpose string array");


S = ["", "1", "12", "123", "1234", "12345"];
S = array_map (Int_Type, &strlen, S);
if (neqs (S, [0:5])) failed ("array_map 1");

S = ["", "1", "12", "123", "1234", "12345"];
variable SS = S + S;
if (neqs (SS, array_map (String_Type, &strcat, S, S))) failed ("array_map 2");

SS = S + "--end";
if (neqs (SS, array_map (String_Type, &strcat, S, "--end"))) failed ("array_map 3");

S = [1:20:0.1];
if (neqs (sin(S), array_map (Double_Type, &sin, S))) failed ("array_map 3");

% Check indexing with negative subscripts 
S = [0:10];

if (S[-1] != 10) failed ("[-1]");
if (length (S[[-1:3]])) failed ("[-1:3]");
if (neqs(S[[-1:0:-1]], [10:0:-1])) failed ("[-1:0:-1]");
if (neqs(S[[0:-1]], S)) failed ("[0:-1]");
if (neqs(S[[3:-1]], [3:10])) failed ([3:-1]);
if (length (S[[0:-1:-1]])) failed ("[0:-1:-1]");   %  first to last by -1
if (neqs(S[[0:]], S)) failed ("[0:]");
if (neqs(S[[:-1]], S)) failed ("[:-1]");

S = Int_Type[0];
if (length (S) != 0) failed ("Int_Type[0]");
if (neqs (S, S[[0:-1]])) failed ("Int_Type[0][[0:-1]]");


S = bstring_to_array ("hello");
if ((length (S) != 5) 
    or (typeof (S) != Array_Type)) failed ("bstring_to_array");
if ("hello" != array_to_bstring (S)) failed ("array_to_bstring");

A = ['a':'z'];
foreach (A)
{
   $1 = ();
   if (A[$1 - 'a'] != $1)
     failed ("['a':'z']");
}

print ("Ok\n");

exit (0);

