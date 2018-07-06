#PROG
vnum 17505
descripcion MP: Recompensa de Athena~
code if istarget $n

mob say ... $n NO te habia visto ya por aqui ?

mob force $n down

else

mob remember $n

kiss $n

mob say Ahhh.. al fin un valiente vino por mi.

mob say Gracias $n, jamas te olvidare. Toma esta pequenia recompensa..

mob echo ..Athena te da #B200 PUNTOS DE QUEST!!!!#n

kiss $n

mob say Ahora..correre a la libertad...

endif

~
#END

