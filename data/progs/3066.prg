#PROG
vnum 3066
descripcion Cityguard greet pr~
code if (isgood(actor))

{

if (level(actor) > 18)

{

act("$n te saluda.", ent, NULL, actor, TO_VICT);

act("$n saluda a $N.", ent, NULL, actor, TO_ROOM);

}

}

else

if (rand(30))

{

interpret(ent,"growl " + nombre(actor));

mob(ent,"say La maldad no es bienvenida aqui!");

}

~
#END

