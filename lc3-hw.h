/*
 * LC3-hw.h
 */

#ifndef LC3_HW_H_
#define LC3_HW_H_

typedef enum {NO, YES, MAYBE} ans;
typedef enum {MEM, ALU} MUX_MemToReg;
typedef enum {REG2, IM} MUX_ALUSrc;
typedef enum {ADD, AND} OP_ALUop;
typedef enum {NF, ZF, PF, ALL, NOBR} BrType;

struct Signals
{
	struct {
		unsigned short newPC;
		unsigned short ins;

	} IFtoID;

	struct {
		unsigned short newPC;
		unsigned short RegData1;
		unsigned short RegData2;
		short IMM;
		unsigned short RegDst;
		//unsigned short Rt;
		//unsigned short Rd;

		//control
		ans RegWrite; 	// 0- no, 1- yes
		MUX_MemToReg MemToReg; 	// 0- mem, 1- alu
		ans Branch;		// 0- no, 1- yes /// <-----------changed!!!
		BrType whichBr;
		ans MemWrite;	// 0- no, 1- yes
		ans MemRead;	// 0- no, 1- yes
		MUX_ALUSrc ALUSrc;		// 0- reg2, 1- imm
		OP_ALUop ALUop;		// 0- '+', 1- 'logic and'
		//int RegDst;	 	// 0- Rt, 1- Rd
		ans changeFlags;// 0- no, 1- yes.  change flags only for ADD, AND, LD

	} IDtoEXE;

	struct {
		unsigned short newPC;
		unsigned short ALU_Res;
		unsigned short RegData2;
		unsigned short RegDst;

		//control
		bool ALU_isZero;
		ans RegWrite; 	// 0- no, 1- yes
		MUX_MemToReg MemToReg; 	// 0- mem, 1- alu
		ans Branch;		// 0- no, 1- yes
		ans MemWrite;	// 0- no, 1- yes
		ans MemRead;	// 0- no, 1- yes

	} EXEtoMem;
};

#endif

