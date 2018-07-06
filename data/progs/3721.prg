#PROG
vnum 3721
descripcion MP Saludo Novatos~
code if (level(actor) == 1)

{

mob(ent,"remember " + nombre(actor));

mob(ent,"delay 5");

}

~
#END

