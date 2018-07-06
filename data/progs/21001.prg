#PROG
vnum 21001
descripcion ~
code if (race(actor) == race_lookup("orc"))

{

mob(ent,"yell Muere asqueroso orco!!!!");

mob(ent,"kill " + nombre(actor));

}

~
#END

