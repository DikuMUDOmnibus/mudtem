#PROG
vnum 22000
descripcion MP Arena Leon~
code if (ispc(actor))

{

act("$n ruge y se lanza sobre ti!", ent, NULL, actor, TO_VICT);

act("$n ruge y se lanza sobre $N!", ent, NULL, actor, TO_ROOM);

mob(ent,"kill " + nombre(actor));

}

~
#END

