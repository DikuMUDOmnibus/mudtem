#PROG
vnum 23503
descripcion Comienza a pelear con el peleador, si no le da la pasada al player~
code mob(ent,"tell " + nombre(actor) + " Pon mucha atencion, te demostrare quien es realmente Mr. Moon");
if (level(actor)<44)
mob(ent,"kill peleador");
else
mob(ent,"tell " + nombre(actor) + "Eres mas poderoso que yo, prueba tu...");
~
#END

