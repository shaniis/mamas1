/*
 * LC3.h
 */

#ifndef LC3_H_
#define LC3_H_

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include "lc3-hw.h"

using namespace std;

typedef enum
{
	b0000 = 0x00,
	b0001 = 0x01,
	b0010 = 0x02,
	b0011 = 0x03,
	b0100 = 0x04,
	b0101 = 0x05,
	b0110 = 0x06,
	b0111 = 0x07,
	b1000 = 0x08,
	b1001 = 0x09,
	b1010 = 0x0A,
	b1011 = 0x0B,
	b1100 = 0x0C,
	b1101 = 0x0D,
	b1110 = 0x0E,
	b1111 = 0x0F
} T_BIN_NIBBLE;

//enum for each opcode type 
typedef enum {
	ADD_OPCODE 	= b0001,
	AND_OPCODE 	= b0101,
	BR_OPCODE	= b0000,
	JMP_OPCODE 	= b1100,
	JS_OPCODE	= b0100,
	LD_OPCODE	= b0010,
	LDI_OPCODE	= b1010,
	LDR_OPCODE	= b0110,
	LEA_OPCODE	= b1110,
	NOT_OPCODE	= b1001,
	RET_OPCODE	= b1100,
	RTI_OPCODE	= b1000,
	ST_OPCODE	= b0011,
	STI_OPCODE	= b1011,
	STR_OPCODE	= b0111,
	TRAP_OPCODE	= b1111,
	RES_OPCODE	= b1101
} OpCode;

// SRs are intentionally first.
// this enum type  represent each type of the instruction field. the names are identical to those in the lc3 architecture document. 
typedef enum {SR, SR1, SR2, IMM5, IMM, OFFSET6, PCOFFSET9,
	TRAPVECT8, BASER, DR, OPCODE, NZP,NO_FIELD, INS_FIELDS_NUM } InsField;

//enum for each flag type 
typedef enum {P_FLAG = 1, Z_FLAG = 2, N_FLAG = 4} Flags;

/* this struct contains the information for a field of an instruction.
* this sturct contains the name of the field, the lsb of the field(sBit) & the msb of the field ( eBit).
the struct contains a boolean indicating if the the sign of the field should be extended.
*/
struct InsFieldBits {
	InsField field;
	unsigned short sBit, eBit;
	bool signExt;
};

struct OpCodeName {
	const char* name;
	OpCode opCode;
};

extern const InsFieldBits insFieldBits[];
extern const OpCodeName opNames[];

//constant for the system. 
enum {MEM_SIZE=1<<9, REGS_NUM = 8, FLAGS_NUM=3,
	IMM_IND=1<<5, MAX_INS_FIELDS=3, SRS_NUM=3};

// Sign extend a number
template <typename T, unsigned B>
inline T SignExtend(const T x)
{
	struct {T x:B;} s;
	return s.x = x;
}

class LC3
{
	signed short regs[REGS_NUM];	// Registers
	unsigned short flags;			// Flags
	unsigned char mem[MEM_SIZE];	// Memory
	unsigned short pc;				// Program Counter

public:
	LC3();
	void DumpMem();
	void WriteMem(unsigned short addr, unsigned short val);
	unsigned short ReadMem(unsigned short addr);
	void Run(int steps);
	
	unsigned short decodeUnsignedField(unsigned short inst, InsField field);
	short decodeSignedField(unsigned short inst, InsField field);

	// For you to implement
	void Fetch(struct Signals &signalsRead, struct Signals &signalsWrite);
	void Decode(struct Signals &signalsRead, struct Signals &signalsWrite);
	void Exec(struct Signals &signalsRead, struct Signals &signalsWrite);
	void WbMem(struct Signals &signalsRead, struct Signals &signalsWrite);
	

};


#endif /* LC3_H_ */
