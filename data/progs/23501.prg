#PROG
vnum 23501
descripcion Da la bienvenida a los player a la entrada del area~
code mob(ent,"tell "+ nombre(actor) + " Bienvenido al Gran Area de Sailor Moon!!!");
if (isevil(actor))
mob(ent,"tell "+ nombre(actor) + " la maldad no es muy bien recibida por aqui, ten mucho cuidado!!!");
else
mob(ent,"tell "+ nombre(actor) + " te recomiendo que vayas a donde la familia Blackmoon, pero ten mucho cuidado.");
~
#END

