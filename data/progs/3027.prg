#PROG
vnum 3027
descripcion MP guild warrior~
code if (class(actor) == class_lookup("warrior"))

{

if (level(actor) > 30)

{

interpret(ent,"bow " + nombre(actor));

mob(ent,"say Bienvenido, hermano guerrero!");

}

else

interpret(ent,"smile " + nombre(actor));

}

else

interpret(ent,"glare " + nombre(actor));

~
#END

