	ADD	R3,R3,#3
	ADD R2,R3,#-1
	ADD R1,R2,#1
	ADD R4,R2,R3
	ADD R1,R3,R4
	ADD R2,R3,R1
	ST	R3,result
done:	BR 	done
result:	.word	0

