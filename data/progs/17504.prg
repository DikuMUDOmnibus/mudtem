#PROG
vnum 17504
descripcion Prog: Purga 7 llaves Poseidon.~
code if istarget $n

mob echo La esmeralda te expele.

mob force $n north

else

mob echo .

mob echo #BFelicitaciones#n $n, has traspasado el Ultimo Portal.

mob echo .

say ..Creo que ya no necesitaras mas esas pesadas llaves..

mob opurge char '$n' 17501

mob opurge char '$n' 17502

mob opurge char '$n' 17503

mob opurge char '$n' 17504

mob opurge char '$n' 17505

mob opurge char '$n' 17506

mob opurge char '$n' 17507

mob say ..Las tomare, no te preocupes $n seras bien recompensado..

endif

if carries $n 17538

mob echo .

Maldito..acaso no deseas rescatar a Athena!! Ya te di tu portal..

mob force $n north

else

mob oload 17538

mob echo .

mob echo Gran $n, heroe, toma el portal del Nibelungo para ti.

mob echo .. y continua tu camino a Athena, al norte.

mob remember $n

drop all

endif

~
#END

