/*
 * asm.h
 *
 *  Created on: Mar 20, 2011
 *      Author: namit
 */

#ifndef COMPILER_H_
#define COMPILER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <assert.h>
#include "lc3.h"

using namespace std;

// Compiler
struct Phase1
{
	Phase1(unsigned short anAssembled) : assembled(anAssembled),
			label("") {}
	Phase1(unsigned short anAssembled, InsField anOffsetType,
			string aLabel) : assembled(anAssembled),
			offsetType(anOffsetType), label(aLabel) {}
	unsigned short assembled;
	InsField offsetType;
	string label;
};

struct Label
{
	Label(string aLabel, unsigned short aPc) : label(aLabel), pc(aPc) {}
	string label;
	unsigned short pc;
};

struct Symbol
{
	string name;
	unsigned short pc;
};

class LC3;

class Compiler {
public:
	Compiler() {}
	Compiler(const char* inFilename, LC3* lc3);
	void Compile();

	vector<Phase1> phase1;
	vector<Label> labels;
	bool Parse(const string &str, int& pc);
	unsigned short Encode(signed short val, InsField field);
	signed short ParseImmediate(const string& str);
	unsigned short ParseRegNum(const string& str);

private:
	OpCode GetOpCode(const string& opName);
	string NextToken(string& str);
	void SecondPhase();
	LC3* lc3;

	ifstream srcFile;
	string srcFilename;
	string codeFilename;
	vector<Symbol> symbols;
};

#endif /* ASM_H_ */
