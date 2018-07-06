#PROG
vnum 17342
descripcion MP Melvin~
code if (not isfollow(ent))

{

mob(ent,"say Puedo acompanarte, y explicarte mis teoremas al reves?");

mob(ent,"follow " + nombre(actor));

}

~
#END

