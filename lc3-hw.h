/*
 * LC3-hw.h
 */

#ifndef LC3_HW_H_
#define LC3_HW_H_

//enum bool {NO, YES, MAYBE};
enum MUX_TOREG {MEM, ALU};
enum MUX_ALU {REG2, IM, OFFSET};
enum OP_ALU{ADD, AND};
enum BR_TYPE {NOBR=0, PF=1, ZF=2, NF=4, ALL=7};

struct Signals
{
	struct {
		bool dataHazardStall;
		unsigned short DHnewPC;
		unsigned short DHins;
	} DataHazard;

	struct {
		bool Branch;		// 0- no, 1- yes
		int ctrlHazardStallCycs;
	} toIF;

	struct {
		unsigned short newPC;
		unsigned short ins;
	} IFtoID;

	struct {
		unsigned short newPC;
		int Reg1Num;
		int Reg2Num;
		short RegData1;
		short RegData2;
		int RegDst; //number of destination register
		short IMM;

		//control
		bool RegWrite;
		MUX_TOREG MemToReg; // 0- mem, 1- alu
		bool Branch;
		BR_TYPE whichBr;
		bool MemWrite;
		bool MemRead;
		MUX_ALU ALUSrc;		// 0- reg2, 1- imm
		OP_ALU ALUop;		// 0- '+', 1- 'logic and'
		bool changeFlags;// 0- no, 1- yes.  change flags only for ADD, AND, LD

	} IDtoEXE;

	struct {
		int Reg1Num;
		int Reg2Num;
		unsigned short newPC;
		short ALU_Res;
		short RegData2;
		int RegDst;  //number of destination register

		//control
		bool ALU_isZero;
		bool RegWrite;
		MUX_TOREG MemToReg; // 0- mem, 1- alu
		bool Branch;
		BR_TYPE whichBr;
		bool MemWrite;
		bool MemRead;


	} EXEtoMem;
};

#endif

