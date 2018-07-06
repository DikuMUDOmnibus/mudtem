#PROG
vnum 3722
descripcion MP Saludo novatos 2~
code if (not hastarget(ent))

{

mob(ent,"say no tengo target :(");

break;

}

mob(ent,"say Bienvenido, " + nombre(target(ent)) + "! La entrada a la escuela es hacia el norte.");

interpret(ent,"smile");

mob(ent,"forget");

~
#END

