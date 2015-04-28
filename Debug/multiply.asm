	BR start
arg1:	.WORD	2
arg2:	.WORD	5
result:	.WORD	0
start:	LD	R1,arg1
	LD	R2,arg2
	AND	R3,R3,#0
loop:
	ADD	R3,R3,R1
	ADD	R2,R2,#-1
	BRp	loop
	ST	R3,result
done:	BR 	done
