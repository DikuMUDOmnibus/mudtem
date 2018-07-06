#PROG
vnum 108
descripcion ~
code act("$n te dice 'Aqui tienes una sorpresita!'", ent, NULL, actor, TO_VICT);
mob(ent,"oload 110");
interpret(ent,"give regalo " + nombre(actor));
~
#END

