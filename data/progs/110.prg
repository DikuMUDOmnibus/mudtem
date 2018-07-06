#PROG
vnum 110
descripcion ~
code act("$n explota!", ent, NULL, actor, TO_ALL);
mob(ent,"damage " + nombre(actor) + " 20 30");
mob(ent,"opurge char " + nombre(actor) + " 110");
~
#END

