#PROG
vnum 3718
descripcion MP Saludo Furey~
code act("$N te dice 'Saludos, $n'.", actor, NULL, ent, TO_CHAR);

act("$N te dice 'Puedo ayudarte a subir tus habilidades si lo deseas.", actor, NULL, ent, TO_CHAR);

act("$N te dice 'Solo escribe 'train <habilidad>'. Por ejemplo, 'train fuerza''.", actor, NULL, ent, TO_CHAR);

interpret(ent,"smile " + nombre(actor));

~
#END

