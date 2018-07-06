#PROG
vnum 23099
descripcion ~
code if (race(actor) != race_lookup("troll"))

{

mob(ent,"yell #F#B!!! LIRHTIM#n");

mob(ent,"kill " + nombre(actor));

}

~
#END

