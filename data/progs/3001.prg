#PROG
vnum 3001
descripcion GRALL baker~
code if (isimmort(actor))

{

mob(ent,"say Hola, maestro " + nombre(actor) + "!");

interpret(ent,"bow " + nombre(actor));

}

else

mob(ent,"say Hola! En que puedo servirte?");

~
#END

