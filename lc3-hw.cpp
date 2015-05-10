#include "lc3.h"
#include <bitset>

#define STALL_CYCLES 2

/////// For you to implement

//you cannot change the signature of this function.
void LC3::Run(int steps)
{
   struct Signals signalsRead, signalsWrite;
   //set the content of the struct to 0's. 
   memset(&signalsRead, 0, sizeof(signalsRead));
   for (int i=0; i<steps; i++) {
	   //each iteration is equal to one clock cycle.
	   memset(&signalsWrite, 0, sizeof(signalsWrite));
	   WbMem(signalsRead, signalsWrite);
	   Exec(signalsRead, signalsWrite);
	   Decode(signalsRead, signalsWrite);
	   Fetch(signalsRead, signalsWrite);
	   memcpy(&signalsRead, &signalsWrite, sizeof(struct Signals));
	   //std::cout << "end of cycle " << i+1 << ":";
	   //for (int j = 0; j < REGS_NUM; j++) std::cout << " r" << j << " = " << regs[j];
	   //std::cout << std::endl;
   }
}

// you may change the signatures of this fucntions according to your needs. 
void LC3::Fetch(struct Signals &signalsRead, struct Signals &signalsWrite)
{
	if(signalsRead.DataHazard.dataHazardStall){
		memcpy(&signalsWrite.IFtoID, &signalsRead.IFtoID, sizeof(signalsRead.IFtoID));
		return;
	}

	if(signalsRead.toIF.Branch && signalsRead.toIF.ctrlHazardStallCycs){
		signalsWrite.IFtoID.ins = 0;
		signalsWrite.toIF.Branch = (--signalsRead.toIF.ctrlHazardStallCycs ? true : false);
		signalsWrite.toIF.ctrlHazardStallCycs = signalsRead.toIF.ctrlHazardStallCycs;
		return;
	}

	signalsWrite.IFtoID.ins = ReadMem(pc);
	unsigned short op = decodeUnsignedField(signalsWrite.IFtoID.ins, OPCODE);
	if(op == BR_OPCODE){
		signalsWrite.toIF.Branch = true;
		signalsWrite.toIF.ctrlHazardStallCycs = STALL_CYCLES;
	}
	else {
		signalsWrite.toIF.Branch = false;
		signalsWrite.toIF.ctrlHazardStallCycs = 0;
	}
	pc += 2;
	signalsWrite.IFtoID.newPC = pc;
}

void LC3::Decode(struct Signals &signalsRead, struct Signals &signalsWrite)
{
	unsigned short newPC = (signalsRead.DataHazard.dataHazardStall ? signalsRead.DataHazard.DHnewPC : signalsRead.IFtoID.newPC);
	unsigned short ins = (signalsRead.DataHazard.dataHazardStall ? signalsRead.DataHazard.DHins : signalsRead.IFtoID.ins);

	unsigned short op = decodeUnsignedField(ins, OPCODE);
	unsigned short reg1Num, reg2Num;

	signalsWrite.IDtoEXE.newPC = newPC;
	switch (op){
	case ADD_OPCODE:
	case AND_OPCODE:
		reg1Num = decodeUnsignedField(ins, SR1);
		if( (signalsRead.IDtoEXE.RegWrite && reg1Num == signalsRead.IDtoEXE.RegDst)
				|| (signalsRead.EXEtoMem.RegWrite && reg1Num == signalsRead.EXEtoMem.RegDst) ){
			signalsWrite.DataHazard.dataHazardStall = true; //<-------------------------------
			signalsWrite.DataHazard.DHins = ins;
			signalsWrite.DataHazard.DHnewPC = newPC;
			memset(&signalsWrite.IDtoEXE, 0, sizeof(signalsWrite.IDtoEXE)); //<---------------------------------
			return;
		}
		signalsWrite.IDtoEXE.RegData1 = regs[reg1Num];
		//signalsWrite.IDtoEXE.RegData2;
		signalsWrite.IDtoEXE.RegDst = decodeUnsignedField(ins, DR);
		//signalsWrite.IDtoEXE.IMM;
		signalsWrite.IDtoEXE.RegWrite = true;
		signalsWrite.IDtoEXE.MemToReg = ALU;
		signalsWrite.IDtoEXE.Branch = false;
		signalsWrite.IDtoEXE.whichBr = NOBR;
		signalsWrite.IDtoEXE.MemWrite = false;
		signalsWrite.IDtoEXE.MemRead = false;
		//signalsWrite.IDtoEXE.ALUSrc;
		signalsWrite.IDtoEXE.ALUop = (op == ADD_OPCODE ? ADD : AND);
		signalsWrite.IDtoEXE.changeFlags = true;
		if(decodeUnsignedField(ins, IMM)){ // bit no.5 is et (1)
			signalsWrite.IDtoEXE.RegData2 = 0;
			signalsWrite.IDtoEXE.IMM = decodeSignedField(ins, IMM5);
			signalsWrite.IDtoEXE.ALUSrc = IM;
		}
		else { // bit 5 is 0
			reg2Num = decodeUnsignedField(ins, SR2);
			if( (signalsRead.IDtoEXE.RegWrite && reg2Num == signalsRead.IDtoEXE.RegDst)
					|| (signalsRead.EXEtoMem.RegWrite && reg2Num == signalsRead.EXEtoMem.RegDst) ){
				signalsWrite.DataHazard.dataHazardStall = true;
				signalsWrite.DataHazard.DHins = ins;
				signalsWrite.DataHazard.DHnewPC = newPC;
				memset(&signalsWrite.IDtoEXE, 0, sizeof(signalsWrite.IDtoEXE));
				return;
			}
			signalsWrite.IDtoEXE.RegData2 = regs[reg2Num];
			signalsWrite.IDtoEXE.IMM = 0;
			signalsWrite.IDtoEXE.ALUSrc = REG2;
		}
		break;

	case BR_OPCODE:
		signalsWrite.IDtoEXE.RegData1 = 0;
		signalsWrite.IDtoEXE.RegData2 = 0;
		signalsWrite.IDtoEXE.RegDst = 0;
		signalsWrite.IDtoEXE.IMM = decodeSignedField(ins, PCOFFSET9);
		signalsWrite.IDtoEXE.RegWrite = false;
		//signalsWrite.IDtoEXE.MemToReg;
		signalsWrite.IDtoEXE.Branch = true;
		signalsWrite.IDtoEXE.whichBr = static_cast<BR_TYPE>(decodeUnsignedField(ins, NZP));
		//std::cout << "****Decode: whichBR = " << static_cast<BR_TYPE>(decodeUnsignedField(ins, NZP)) << std::endl;
		signalsWrite.IDtoEXE.MemWrite = false;
		signalsWrite.IDtoEXE.MemRead = false;
		signalsWrite.IDtoEXE.ALUSrc = OFFSET; //should do IMM
		signalsWrite.IDtoEXE.ALUop = ADD;
		signalsWrite.IDtoEXE.changeFlags = false;
		break;

	case LD_OPCODE:
		signalsWrite.IDtoEXE.RegData1 = 0;
		signalsWrite.IDtoEXE.RegData2 = 0;
		signalsWrite.IDtoEXE.RegDst = decodeUnsignedField(ins, DR);
		signalsWrite.IDtoEXE.IMM = decodeSignedField(ins, PCOFFSET9);
		signalsWrite.IDtoEXE.RegWrite = true;
		signalsWrite.IDtoEXE.MemToReg = MEM;
		signalsWrite.IDtoEXE.Branch = false;
		signalsWrite.IDtoEXE.whichBr = NOBR;
		signalsWrite.IDtoEXE.MemWrite = false;
		signalsWrite.IDtoEXE.MemRead = true;
		signalsWrite.IDtoEXE.ALUSrc = OFFSET; //should do IMM
		signalsWrite.IDtoEXE.ALUop = ADD;
		signalsWrite.IDtoEXE.changeFlags = true;
		break;

	case ST_OPCODE:
		signalsWrite.IDtoEXE.RegData1 = 0;
		reg2Num = decodeUnsignedField(ins, SR);
		if( (signalsRead.IDtoEXE.RegWrite && reg2Num == signalsRead.IDtoEXE.RegDst)
				|| (signalsRead.EXEtoMem.RegWrite && reg2Num == signalsRead.EXEtoMem.RegDst) ){
			signalsWrite.DataHazard.dataHazardStall = true; //<-------------------------------
			signalsWrite.DataHazard.DHins = ins;
			signalsWrite.DataHazard.DHnewPC = newPC;
			memset(&signalsWrite.IDtoEXE, 0, sizeof(signalsWrite.IDtoEXE)); //<---------------------------------
			return;
		}
		signalsWrite.IDtoEXE.RegData2 = regs[reg2Num];
		signalsWrite.IDtoEXE.RegDst = 0;
		signalsWrite.IDtoEXE.IMM = decodeSignedField(ins, PCOFFSET9);
		signalsWrite.IDtoEXE.RegWrite = false;
		//signalsWrite.IDtoEXE.MemToReg;
		signalsWrite.IDtoEXE.Branch = false;
		signalsWrite.IDtoEXE.whichBr = NOBR;
		signalsWrite.IDtoEXE.MemWrite = true;
		signalsWrite.IDtoEXE.MemRead = false;
		signalsWrite.IDtoEXE.ALUSrc = OFFSET; //should do IMM
		signalsWrite.IDtoEXE.ALUop = ADD;
		signalsWrite.IDtoEXE.changeFlags = false;
		break;
	}
}

void LC3::Exec(struct Signals &readSignals, struct Signals &writeSignals)
{
	switch (readSignals.IDtoEXE.ALUSrc) {
	case REG2:
		if (readSignals.IDtoEXE.ALUop == ADD) // '+'
			writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 + readSignals.IDtoEXE.RegData2;
		else writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 & readSignals.IDtoEXE.RegData2;
		break;
	case IM:
		if (readSignals.IDtoEXE.ALUop == ADD) // '+'
			writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 + readSignals.IDtoEXE.IMM;
		else writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.RegData1 & readSignals.IDtoEXE.IMM;
		break;
	case OFFSET: writeSignals.EXEtoMem.ALU_Res = readSignals.IDtoEXE.newPC + readSignals.IDtoEXE.IMM;
	}

	writeSignals.EXEtoMem.ALU_Res == 0 ?  writeSignals.EXEtoMem.ALU_isZero = 0 : writeSignals.EXEtoMem.ALU_isZero = 1;
    writeSignals.EXEtoMem.newPC = readSignals.IDtoEXE.newPC + readSignals.IDtoEXE.IMM; //check if multiply is needed

    // update flags
    if (readSignals.IDtoEXE.changeFlags) {
    	//std::cout << "******EXE: ALU_Res = "<< writeSignals.EXEtoMem.ALU_Res;
    	switch (writeSignals.EXEtoMem.ALU_Res > 0) {
    	case 0: flags = (writeSignals.EXEtoMem.ALU_Res == 0 ? Z_FLAG : N_FLAG); break;
    	case 1: flags = P_FLAG; break;
    	}
    	//std::cout << " flags (after change) = " << flags <<std::endl;
    }

    //Update EXEtoMem
    writeSignals.EXEtoMem.RegDst = readSignals.IDtoEXE.RegDst;
	writeSignals.EXEtoMem.RegWrite = readSignals.IDtoEXE.RegWrite;
	writeSignals.EXEtoMem.MemToReg = readSignals.IDtoEXE.MemToReg;
	writeSignals.EXEtoMem.MemWrite = readSignals.IDtoEXE.MemWrite;
	writeSignals.EXEtoMem.MemRead = readSignals.IDtoEXE.MemRead;
	writeSignals.EXEtoMem.Branch = readSignals.IDtoEXE.Branch;
	writeSignals.EXEtoMem.whichBr = readSignals.IDtoEXE.whichBr;
	writeSignals.EXEtoMem.RegData2 = readSignals.IDtoEXE.RegData2;
}


void LC3::WbMem(struct Signals &signalsRead, struct Signals &signalsWrite)
{
	if (signalsRead.EXEtoMem.Branch){  //BRANCH
		//std::cout << "********WbMem: in Branch, flags = " << flags << " whichBR = " << signalsRead.EXEtoMem.whichBr << std::endl ;
		if ((signalsRead.EXEtoMem.whichBr == NF && flags == N_FLAG) || (signalsRead.EXEtoMem.whichBr == ZF && flags == Z_FLAG) ||
			(signalsRead.EXEtoMem.whichBr == PF && flags == P_FLAG) || (signalsRead.EXEtoMem.whichBr == ALL)){
			//std::cout << "********WbMem: in cond!\n";
			pc = signalsRead.EXEtoMem.newPC;
		}
	}

	if (signalsRead.EXEtoMem.RegWrite){
		if (signalsRead.EXEtoMem.MemToReg == MEM){  //LD
			regs[signalsRead.EXEtoMem.RegDst] = mem[signalsRead.EXEtoMem.ALU_Res];
			// Update flags
			switch (mem[signalsRead.EXEtoMem.ALU_Res] > 0) {
			case 0: flags = P_FLAG; break;
			case 1: flags = N_FLAG; break;
			default: flags = Z_FLAG;
			}
		}
		else { // ADD or AND
			regs[signalsRead.EXEtoMem.RegDst] = signalsRead.EXEtoMem.ALU_Res;
		}
	}

	if (signalsRead.EXEtoMem.MemWrite) //SW
		mem[signalsRead.EXEtoMem.ALU_Res] = signalsRead.EXEtoMem.RegData2;
}
