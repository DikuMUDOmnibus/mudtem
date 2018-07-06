#PROG
vnum 19005
descripcion Turista random~
code mob(ent,"forget");
mob(ent,"remember " + nombre(actor) );
mob(ent,"delay 3");
interpret(ent,"emote te mira con cara de curioso...");
~
#END

