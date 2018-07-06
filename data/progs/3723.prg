#PROG
vnum 3723
descripcion ~
code mob(ent,"remember " + nombre(actor));

if (not hastarget(ent)) break;

variable blah = target(ent);

send_to_ent(blah,"fuck");

~
#END

