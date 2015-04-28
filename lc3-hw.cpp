#include "lc3.h"
#include <bitset>

/////// For you to implement

//you cannot change the signature of this function.
void LC3::Run(int steps)
{
   struct Signals signalsRead, signalsWrite;
   //set the content of the struct to 0's. 
   memset(&signalsRead, 0, sizeof(signalsRead));
   memset(&signalsWrite, 0, sizeof(signalsWrite));
   std::cout << "pc = " << pc << std::endl;
	for (int i=0; i<steps; i++) {
	//each iteration is equal to one clock cycle.
		WbMem(signalsRead, signalsWrite);
		Exec(signalsRead, signalsWrite);
		Decode(signalsRead, signalsWrite);
		Fetch(signalsWrite);
		memcpy(&signalsRead, &signalsWrite, sizeof(struct Signals));
	}
}

// you may change the signatures of this fucntions according to your needs. 
void LC3::Fetch(struct Signals &signalsWrite)
{
	signalsWrite.IFtoID.ins = ReadMem(pc);;
	pc += 2;
	signalsWrite.IFtoID.newPC = pc;
}

void LC3::Decode(struct Signals &signalsRead, struct Signals &signalsWrite)
{
	unsigned short op = decodeUnsignedField(signalsRead.IFtoID.ins, OPCODE);
	unsigned short reg1Num;

	switch (op){

	case ADD_OPCODE:
	case AND_OPCODE:
		signalsWrite.IDtoEXE.Branch = NO;
		signalsWrite.IDtoEXE.whichBr = NOBR;
		signalsWrite.IDtoEXE.ALUop = (op == ADD_OPCODE ? ADD : AND);
		signalsWrite.IDtoEXE.MemRead = NO;
		signalsWrite.IDtoEXE.MemWrite = NO;
		signalsWrite.IDtoEXE.MemToReg = ALU;
		signalsWrite.IDtoEXE.RegWrite = YES;
		signalsWrite.IDtoEXE.changeFlags = YES;
		signalsWrite.IDtoEXE.newPC = signalsRead.IFtoID.newPC;
		reg1Num = decodeUnsignedField(signalsRead.IFtoID.ins, SR1);
		signalsWrite.IDtoEXE.RegData1 = regs[reg1Num];
		signalsWrite.IDtoEXE.RegDst = decodeUnsignedField(signalsRead.IFtoID.ins, DR);

		if(decodeUnsignedField(signalsRead.IFtoID.ins, IMM)){ // bit 5 is 1
			signalsWrite.IDtoEXE.IMM = decodeSignedField(signalsRead.IFtoID.ins, IMM5);
			signalsWrite.IDtoEXE.ALUSrc = IM;
		}
		else { // bit 5 is 0
			unsigned short reg2Num = decodeUnsignedField(signalsRead.IFtoID.ins, SR2);
			signalsWrite.IDtoEXE.RegData2 = regs[reg2Num];
			signalsWrite.IDtoEXE.ALUSrc = REG2;
		}
		break;

	case BR_OPCODE:
		break;
	/*case JMP_OPCODE:
		break;
	case JS_OPCODE:
		break;*/
	case LD_OPCODE:
		break;
	/*case LDI_OPCODE:
		break;
	case LDR_OPCODE:
		break;
	case LEA_OPCODE:
		break;
	case NOT_OPCODE:
		break;
	case RET_OPCODE:
		break;
	case RTI_OPCODE:
		break;*/
	case ST_OPCODE:
		break;
	/*case STI_OPCODE:
		break;
	case STR_OPCODE:
		break;
	case TRAP_OPCODE:
		break;
	case RES_OPCODE:
		break;*/
	}
}

void LC3::Exec(struct Signals &readSignals, struct Signals &writeSignals)
{
	if (readSignals.IDtoEXE.ALUSrc == 0) {
		if (readSignals.IDtoEXE.ALUop == 1) {
			writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 + readSignals.IDtoEXE.RegData2;
		}
		else writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 & readSignals.IDtoEXE.RegData2; // איך עושים and לוגי???
	}
	else if (readSignals.IDtoEXE.ALUop == 1) {
		writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 + readSignals.IDtoEXE.IMM;
    }
	else writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 & readSignals.IDtoEXE.IMM; // איך עושים and לוגי???

	writeSignals.EXEtoMem.RegDst = readSignals.IDtoEXE.RegDst ;
	readSignals.EXEtoMem.ALU_Res == 0 ? writeSignals.EXEtoMem.ALU_isZero = 1 : writeSignals.EXEtoMem.ALU_isZero = 0;
	writeSignals.EXEtoMem.newPC = readSignals.IDtoEXE.newPC + readSignals.IDtoEXE.IMM * 4;
}


void LC3::WbMem(struct Signals &signalsRead, struct Signals &signalsWrite)
{
}

