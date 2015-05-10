#include <iostream>
#include "lc3.h"
#include "compiler.h"

using namespace std;
// an array of all the instruction Insfieldbit. the array cotnain InsFieldbit for each field in the architecture.
const struct InsFieldBits insFieldBits[] = {
	{SR, 9, 11, false},
	{SR1, 6, 8, false},
	{SR2, 0, 2, false},
	{IMM5, 0, 4, true},
	{IMM, 5, 5, false},
	{OFFSET6, 0, 5, true},
	{PCOFFSET9, 0, 8, true},
	{TRAPVECT8, 0, 7, false},
	{BASER, 6, 8, false},
	{DR, 9, 11, false},
	{OPCODE, 12, 15, false},
	{NZP, 9, 11, false},
	{NO_FIELD, 0, 0, false}
};

// Write to memory a value.
void LC3::WriteMem(unsigned short addr, unsigned short val)
{
	*(unsigned short*)(mem+addr) = val;
}

// Read from memory a value.
unsigned short LC3::ReadMem(unsigned short addr)
{
	return *(unsigned short*)(mem+addr);
}

LC3::LC3() : flags(0), pc(0)
{
	for (int i=0; i < REGS_NUM; i++)
		regs[i] = 0;
	memset(mem, 0, MEM_SIZE);
}
// prints the entire content of the memory . 
void LC3::DumpMem()
{
	for (int i=0; i<MEM_SIZE; i++)
		cout << hex << (int)mem[i] << endl;
}
/* decode an unsigned instruction field.
recieves an instruction and a type of unsigned instrction field.
returns the value of the that field in the instruction.
for example  in the instruction add R1,R1,R2 (0001001001000010b) calling the function with OPCODE field returns 1(0001b).   
*/
unsigned short LC3::decodeUnsignedField(unsigned short inst, InsField field) {
	const struct InsFieldBits fieldBits=insFieldBits[field];
	unsigned short numberOfBits=fieldBits.eBit - fieldBits.sBit +1;
	 return  (inst >> fieldBits.sBit) & ((1U<<numberOfBits) - 1);
	}

//same like decodeUnsignedField, except this fucntions decode signed fields , and use sign extension if it is neccessary. 
short LC3::decodeSignedField(unsigned short inst, InsField field) {
	const struct InsFieldBits fieldBits=insFieldBits[field];
	unsigned short numberOfBits=fieldBits.eBit - fieldBits.sBit +1;
	signed short res = (inst >> fieldBits.sBit) & ((1U<<numberOfBits) - 1);
	if (fieldBits.signExt) {
		// there are only 2 of signed field that you need to implement. 
		if(numberOfBits==9)
			return SignExtend<short,9>(res);
		else 
			return SignExtend<short,5>(res);
		}
	return res;
	}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		cout << "usage: lc3 [asm filename] [cycles]" << endl;
		return -1;
	}

	LC3 *lc3 = new LC3();
	Compiler* compiler = new Compiler(argv[1], lc3);
	//sstd::cout << argv[1] << std::endl;
	int cycles = atoi(argv[2]);
	if (cycles <= 0) {
		cout << "number of cycles must be great than zero" << endl;
		return -1;
	}
	
	compiler->Compile();
	lc3->Run(cycles);
	lc3->DumpMem();
	delete compiler;
	delete lc3;
	return 0;
}

