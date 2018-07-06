#PROG
vnum 3065
descripcion Janitor drop prog~
code mob(ent,"say Basura! eso son ustedes...BASURA!");

interpret(ent,"growl " + nombre(actor));

if (rand(50))

mob(ent,"kill " + nombre(actor));

~
#END

