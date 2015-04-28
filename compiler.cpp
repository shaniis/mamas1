#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <assert.h>
#include "compiler.h"
#include "lc3.h"

using namespace std;

const char *delimiters = " \t,\0";

string Compiler::NextToken(string& str)
{
	string token;
	size_t min_del_place;
	do
	{
		const char* delimiter = delimiters;
		min_del_place = str.size();

		while (*delimiter) {
			size_t del_place = str.find(*delimiter);
			if (del_place < min_del_place)
					min_del_place = del_place;
			delimiter++;
		}

		token = str.substr(0, min_del_place);
		str = min_del_place==str.size() ? "" : str.substr(min_del_place+1);
	} while (min_del_place <= 1 && str.size() > 0);

	return token;
}

unsigned short Compiler::Encode(signed short val, InsField field)
{
	unsigned short* uval = (unsigned short*)&val;
	const struct InsFieldBits *pIfb = insFieldBits;

	while (pIfb->field != NO_FIELD && pIfb->field != field)
		pIfb++;

	unsigned short res = (*uval & ((1U<<(pIfb->eBit-pIfb->sBit+1)) - 1)) << pIfb->sBit;
	return (*uval & ((1U<<(pIfb->eBit-pIfb->sBit+1)) - 1)) << pIfb->sBit;
}

unsigned short Compiler::ParseRegNum(const string& str)
{
	return atoi(str.substr(1, string::npos).data());
}

signed short Compiler::ParseImmediate(const string& str)
{
	return atoi(str.substr(1, string::npos).data());
}

const OpCodeName opNames[] = {
	{"ADD", ADD_OPCODE},
	{"AND", AND_OPCODE},
	{"BR", BR_OPCODE},
	{"BRZ", BR_OPCODE},
	{"BRN", BR_OPCODE},
	{"BRP", BR_OPCODE},
	{"JMP", JMP_OPCODE},
	{"JS", JS_OPCODE},
	{"LD", LD_OPCODE},
	{"LDI", LDI_OPCODE},
	{"LDR", LDR_OPCODE},
	{"LEA", LEA_OPCODE},
	{"NOT", NOT_OPCODE},
	{"RET", RET_OPCODE},
	{"RTI", RTI_OPCODE},
	{"ST", ST_OPCODE},
	{"STI", STI_OPCODE},
	{"STR", STR_OPCODE},
	{"TRAP", TRAP_OPCODE},
	{"RES", RES_OPCODE}
};

OpCode Compiler::GetOpCode(const string& opName)
{
	const OpCodeName *pOpCodeName = opNames;
	while (opName != pOpCodeName->name && pOpCodeName->opCode != RES_OPCODE)
		pOpCodeName++;
	assert(pOpCodeName->opCode != RES_OPCODE);
	return pOpCodeName->opCode;
}

void Compiler::SecondPhase()
{
	int pc = 0;
	for(vector<Phase1>::const_iterator cii=phase1.begin();
			cii!=phase1.end(); cii++) {
		const Phase1* p1 = cii.base();
		unsigned short op = p1->assembled;
		if (p1->label != "") {
			// Need to backpatch
			bool found = false;

			for (vector<Label>::const_iterator lii=labels.begin();
					lii!=labels.end(); lii++) {
				if (cii.base()->label == lii.base()->label) {
					signed short offset = lii.base()->pc - pc - 2;
					op |= Encode(offset, p1->offsetType);
					found = true;
					break;
				}
			}
			if (!found) {
				cout << "cannot find label " << cii.base()->label << endl;
			}

			assert(found);
		}
		lc3->WriteMem(pc, op);
		pc += 2;
	}
}

bool Compiler::Parse(const string &str, int &pc)
{
	string parsed = str;
	string rest, nocomment = str;
	nocomment = str.substr(0, str.find(';'));
	std::transform(nocomment.begin(), nocomment.end(), nocomment.begin(), ::toupper);

	string label;
	string token = NextToken(nocomment);
	if (token.size() > 0 && token.at(token.size()-1) == ':') {
		label = token.substr(0, token.size()-1);
		labels.push_back(Label(label, pc));

		token = NextToken(nocomment);
	}

	string op = token;
	string arg1 = NextToken(nocomment);
	string arg2 = NextToken(nocomment);
	string arg3 = NextToken(nocomment);
	string backpatchLabel = "";
	InsField offsetType = PCOFFSET9;

	if (op == "")
		return false;


	if (op == ".WORD") {
		signed short val = atoi(arg1.data());
		phase1.push_back(Phase1(*(unsigned short*)&val, PCOFFSET9, ""));
		return true;
	}

	OpCode opCode = GetOpCode(token);
	unsigned short opcode = 0;
	signed short fields[INS_FIELDS_NUM] = {0};

	fields[OPCODE] = opCode;

	switch (opCode)
	{
	case ADD_OPCODE:
	case AND_OPCODE:
		fields[DR] = ParseRegNum(arg1);
		fields[SR1] = ParseRegNum(arg2);
		if (arg3.at(0)=='#') {
			fields[IMM] = 1;
			fields[IMM5] = ParseImmediate(arg3);
		} else
			fields[SR2] = ParseRegNum(arg3);
		break;

	case BR_OPCODE:
		if (op.find('Z') != string::npos)
			fields[NZP] |= Z_FLAG;
		else if (op.find('P') != string::npos)
			fields[NZP] |= P_FLAG;
		else if (op.find('N') != string::npos)
			fields[NZP] |= N_FLAG;
		else
			fields[NZP] = Z_FLAG | P_FLAG | N_FLAG;
		
		backpatchLabel = arg1;
		break;

	case LD_OPCODE:
		fields[DR] = ParseRegNum(arg1);
		backpatchLabel = arg2;
		break;

	case ST_OPCODE:
		fields[SR] = ParseRegNum(arg1);
		backpatchLabel = arg2;
		break;

	default:
		cout << "This case is not handled" << endl;

	}

	for (int i=0; i<INS_FIELDS_NUM; i++)
		opcode |= this->Encode(fields[i], (InsField)i);

	phase1.push_back(Phase1(opcode, offsetType, backpatchLabel));

	return true;
}

Compiler::Compiler(const char* inFilename, LC3* aLc3)
{
	srcFilename = inFilename;
	lc3 = aLc3;
}

void Compiler::Compile()
{
	char line[256];
	srcFile.open(srcFilename.data());
	int pc = 0;
	while (!srcFile.eof()) {
		srcFile.getline(line, 256);
		if (Parse(string(line), pc))
			pc += 2;
	}
	SecondPhase();
	srcFile.close();
}
