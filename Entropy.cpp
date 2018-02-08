#include "compact.h"
#include <memory.h>
#include <mmintrin.h>
#include <emmintrin.h>
#include <intrin.h>
#include "lagarith.h"
#include "reciprocal_table.cpp"

#include "log.h"
#include <sstream>
#include "tree.h"


#define readBit() { \
	bit = bitpos & *in;\
	bitpos >>=1;\
	if ( !bitpos ){\
		in++;\
		bitpos = 0x80;\
	}\
}

#define writeBit(x) { \
	if ( x) \
		*out|=bitpos;\
	bitpos>>=1;\
	if ( !bitpos){ \
		bitpos=0x80;\
		out++;\
		*out=0;\
	}\
}


/* Functions
=========================================================================================================================================
*/
void Countchar(const unsigned char * in, unsigned int length, unsigned int * table[256]) {
	const unsigned char * input = in;
	const unsigned char * const ending = in + length;
	int a = 0;

	do
	{
		a = (int)table[*input];
		a++;
		table[*input] = (unsigned int*)a;
		input++;

	} while (input < ending);

}

void CountcharRow(const unsigned char * in, unsigned int length, int * table[256]) {
	const unsigned char * input = in;
	unsigned char current_input = *input;
	const unsigned char * const ending = input + length;
	int a = 0;

	do
	{
		current_input = *input;
		char val = *input;

		int loop_val = val;
			int rlecount = 0;
			do
			{
				input++;
				rlecount++;
				val = *input;
			} while (input < ending && val == current_input);
		

		a = (int)table[current_input];

		a += rlecount - 2;
	
		table[current_input] = (int *)a;


	} while (input < ending);
}

unsigned int CompressClass::Charprob(const unsigned char * in, unsigned int length, unsigned char * out, unsigned char * prob_table) {
	int * table[256];
	unsigned char result[256];
	memset(table, 0, sizeof(table));
	memset(result, 0, sizeof(result));
	int count = 0;
	int prob = 30;

	CountcharRow(in, length, table);
	for (int i = 0; i < 256; i++) {
		if ((int)table[i] > 1) {
			result[count] = i;
			count++;
		}

	}
	*out = count;
	out++;
	for (int i = 0; i < count; i++) {
		*out = result[i];
		out++;
		prob_table[i] = result[i];
	}
	

	return count;

}

bool contains(unsigned char * in, const unsigned int length, const unsigned int element) {
	for (unsigned int i = 0; i < length; i++) {
		if (in[i] == element) {
			return true;
		}
	}
	return false;
}

/* Shannon Fano
=========================================================================================================================================
*/


unsigned int CompressClass::EncodeSF(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
	// Define input end
	const unsigned char * const ending = in + length;
	// save the out pointers start position
	unsigned char * const start = out;

	unsigned char bitpos = 0b10000000;

	unsigned int * table[256];			// Table to save the Counts
	unsigned char data[256];			// Table to save the used chars
	int freq[256];						// Table to save the frequencies for the used chars
	p_CodeTable codeTable[256];

	// init tables
	memset(table, 0, sizeof(table));
	memset(data, 0, sizeof(data));
	memset(freq, 0, sizeof(freq));
	memset(codeTable, 0, sizeof(codeTable));


	// Count the chars
	Countchar(in, length, table);

	// Remove the empty counts
	int count = 0;
	for (int i = 0; i < 256; i++) {
		if ((int)table[i] > 0) {
			data[count] = i;
			freq[count] = (int)table[i];
			count++;
		}

	}


	//Build Huffman tree
	p_TreeNodeArray nArray = createArray(0,0);
	p_TreeNode root = buildTreeShannon(freq, data, count,&nArray);
	cout << endl;

	int skip = 0;
	storeTree(root, out, &skip);
	out += skip;

	getCodes(root, codeTable);

	*out = 0;
	do {
		// getting input data
		int in_val = in[0];

		bool bit = 0;
		//unsigned int readpos = 0b10000000000000000000000000000000;
		unsigned int readpos = 0b10000000000000000000000000000000;
		unsigned int  shift = codeTable[in_val]->shift;
		unsigned int  code = codeTable[in_val]->code;

		for (unsigned int i = 0; i < shift; i++) {
			bit = code & readpos;
			readpos >>= 1;
			writeBit(bit);
		}

		in++;

	} while (in < ending);

	//deleteTree(root);
	//deleteTree(nArray);
	//delete [] data;

	return (unsigned int)(out - start) + 1;
}

void CompressClass::DecodeSF(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
	const unsigned char * const ending = out + length;
	bool bit = 0;
	unsigned char bitpos = 0x80;
	const unsigned char *  start = in;

	p_TreeNode root = createNode(0, 0);

	in++;

	int skip = 0;
	restoreTree(&root, in, &skip);
	in += skip;


	p_TreeNode currentNode;
	currentNode = root;

	do {
		bit = bitpos & *in;
		bitpos >>= 1;
		if (!bitpos) {
			in++;
			bitpos = 0x80;
		}

		if (bit)
			currentNode = currentNode->rightC;
		else
			currentNode = currentNode->leftC;

		if (currentNode->leaf) {
			*out = currentNode->data;
			out++;
			currentNode = root;
		}


	} while (out < ending);
}



/* Huffman
=========================================================================================================================================
*/


unsigned int CompressClass::EncodeHUF(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
	// Define input end
	const unsigned char * const ending = in + length;
	// save the out pointers start position
	unsigned char * const start = out;

	unsigned char bitpos = 0b10000000;

	unsigned int * table[256];			// Table to save the Counts
	unsigned char data[256];			// Table to save the used chars
	int freq[256];						// Table to save the frequencies for the used chars
	p_CodeTable codeTable[256];

	// init tables
	memset(table, 0, sizeof(table));
	memset(data, 0, sizeof(data));
	memset(freq, 0, sizeof(freq));
	memset(codeTable, 0, sizeof(codeTable));

	// Count the chars
	Countchar(in, length, table);

	// Remove the empty counts
	int count = 0;
	for (int i = 0; i < 256; i++) {
		if ((int)table[i] > 0) {
			data[count] = i;
			freq[count] = (int)table[i];
			count++;
		}

	}


	//Build Huffman tree
	p_TreeNode root = buildTree(freq, data, count);
	int skip = 0;
	storeTree(root, out, &skip);
	out += skip;

	getCodes(root, codeTable);

	*out = 0;
	do {
		// getting input data
		int in_val = *in;

		bool bit = 0;
		//unsigned int readpos = 0b10000000000000000000000000000000;
		unsigned int readpos = 0b10000000000000000000000000000000;
		unsigned int  shift = codeTable[in_val]->shift;
		unsigned int  code = codeTable[in_val]->code;

		for (unsigned int i = 0; i < shift; i++) {
			bit = code & readpos;
			readpos >>= 1;
			writeBit(bit);
		}

		in++;

	} while (in < ending);
	return (unsigned int)(out - start) + 1;
}

void CompressClass::DecodeHUF(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
	const unsigned char * const ending = out + length;
	bool bit = 0;
	unsigned char bitpos = 0x80;

	p_TreeNode root = createNode(0, 0);

	int skip = 0;
	restoreTree(&root, in, &skip);
	in += skip;


	p_CodeTable codeTable[256];
	memset(codeTable, 0, sizeof(codeTable));
	getCodes(root, codeTable);


	p_TreeNode currentNode;
	currentNode = root;
	int count = 0;
	do {

		readBit();
		if (bit)
			currentNode = currentNode->rightC;
		else
			currentNode = currentNode->leftC;

		if (currentNode->leaf) {
			*out = currentNode->data;
			out++;
			currentNode = root;

			count++;
		}
	} while (out < ending);


}

/* RLE
=========================================================================================================================================
*/

unsigned int CompressClass::EncodeRLE(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
	const unsigned char * const ending = in + length;
	unsigned char * const count = out;
	int current_val = *in;

	unsigned char prob_table[100];
	memset(prob_table, 0, sizeof(prob_table));

	int size = Charprob(in, length, out, prob_table);
	out += size + 1;

	do {
		// getting input data
		int val = *in;
		current_val = *in;


		bool exist = contains(prob_table, size, val);

		if (exist)
		{
			int loop_val = val;
			int rlecount = 0;
			do
			{
				in++;
				rlecount++;
				val = *in;

			} while (in < ending && val == current_val);

			out[0] = current_val;
			out[1] = rlecount;
			out += 2;
		}
		else {
			*out = val;
			out++;
			in++;
		}

	} while (in<ending);	
	return (unsigned int)(out - count);
}


void CompressClass::DecodeRLE(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	const unsigned char * const ending = out + length;
	
	int size = *in;
	in++;
	unsigned char prob_table[100];
	memset(prob_table, 0, sizeof(prob_table));
	for (int i = 0; i < size; i++) {
		prob_table[i] = *in;
		in++;
	}


	do {
		int val = *in;

		bool exist = contains(prob_table, size, val);

		if (exist)
		{
			in++;
			int count = *in;
			for (int i = 0; i < count; i++) {
				*out = val;
				out++;
				i += val;
			}
		}
		else {
			*out = val;
			out++;
		}
		in++;

	} while (out<ending);

}


/* LZ77
=========================================================================================================================================
*/

bool searchInBuffer(const unsigned char * lookahead, unsigned char  lookaheadlength, const unsigned char * buffer, unsigned char  bufferlength, unsigned char * offset, unsigned char * length, unsigned char * additional_char) {
	const unsigned char * const bufferEnding = buffer + bufferlength;
	const unsigned char * const lookaheadEnding = lookahead + lookaheadlength;
	const unsigned char *  count = buffer;
	length[0] == 0;
	do {

		if (*buffer == *lookahead)
		{
			*offset = (buffer - count);

			do {
				length[0]++;;
				buffer++;
				lookahead++;

			} while (*buffer == *lookahead && lookahead < lookaheadEnding && buffer < bufferEnding);

			*additional_char = *lookahead;

			return true;

		}

		buffer++;
	} while (buffer < bufferEnding);
	*additional_char = *lookahead;
	return false;

}

bool findBestMatch(const unsigned char * lookahead, unsigned char  lookaheadlength, const unsigned char * buffer, unsigned char  bufferlength, unsigned char * offset, unsigned char * length, unsigned char * additional_char) {
	unsigned char best_offset = 0;
	unsigned char best_length = 0;
	unsigned char best_additional = 0;

	const unsigned char * const bufferEnding = buffer + bufferlength;
	const unsigned char * const lookaheadEnding = lookahead + lookaheadlength;
	const unsigned char *  count = buffer;

	do {
		length[0] = 0;
		bool found = searchInBuffer(lookahead, lookaheadlength, buffer, bufferEnding - buffer, offset, length, additional_char);


		if (found) {
			if (best_length < *length)
			{
				best_length = *length;
				best_additional = *additional_char;
				buffer += *offset;
				best_offset = buffer - count;
			}
			buffer += *offset;

		}
		else
		{
			if (best_length) {
				*offset = best_offset;
				*length = best_length;
				*additional_char = best_additional;
				return true;
			}
			else {
				best_additional = *lookahead;
				return false;
			}

		}

		buffer++;

	} while (buffer < bufferEnding);

	if (best_length) {
		*offset = best_offset;
		*length = best_length;
		*additional_char = best_additional;
		return true;
	}
	else {
		best_additional = *lookahead;
		return false;
	}



}

unsigned int CompressClass::EncodeLZ77(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
	const unsigned char * const ending = in + length;
	unsigned char * const count = out;

	unsigned char max_buffer_size = 100;
	unsigned char max_lookahead_size = 50;
	*out = max_buffer_size;
	out++;



	unsigned char buffer_size = 1;
	const unsigned char *  buffer = in;
	unsigned char lookahead_size = max_lookahead_size;
	const unsigned char *  lookahead = in;

	unsigned char offset = 0;
	unsigned char Llength = 0;
	unsigned char additional = 0;

	out[0] = 0;
	out[1] = *in;
	lookahead++;
	out += 2;

	do {
		bool found = findBestMatch(lookahead, lookahead_size, buffer, buffer_size, &offset, &Llength, &additional);

		if (found)
		{
			out[0] = Llength;
			out[1] = offset;
			out[2] = additional;
			lookahead += Llength;
			out += 3;
		}
		else
		{
			out[0] = 0;
			out[1] = additional;
			out += 2;
		}

		lookahead++;

		int dif = lookahead - buffer;

		if (dif < max_buffer_size) {
			buffer = in;
			buffer_size = lookahead - buffer;
		}
		else if (dif > max_buffer_size) {
			buffer = lookahead - max_buffer_size;
			buffer_size = max_buffer_size;
		}



	} while (lookahead < ending);

	return (unsigned int)(out - count);
}

void CompressClass::DecodeLZ77(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	const unsigned char * const ending = out + length;
	const unsigned char * buffer = out;
	const unsigned char * start = out;
	unsigned char max_buffer_size = *in;
	in++;



	unsigned char offset = 0;
	unsigned char Llength = 0;
	unsigned char additional = 0;

	do {
		// getting input data
		unsigned char in_val = *in;
		in++;


		if (in_val != 0)
		{
			Llength = in_val;
			offset = *in;
			in++;

			for (unsigned int i = 0; i < Llength; i++)
			{
				*out = buffer[offset + i];
				out++;
			}

			*out = *in;
			in++;
			out++;

		}
		else
		{
			*out = *in;
			out++;
			in++;
		}

		//int dif = out - buffer;
		//if (dif != max_buffer_size) {
		//	if (dif < max_buffer_size)
		//		buffer = start;
		//	else if (dif > max_buffer_size)
		//		buffer = out - max_buffer_size;
		//}

		int dif = out - buffer;

		if (dif < max_buffer_size) {
			buffer = start;
		}
		else if (dif > max_buffer_size) {
			buffer = out - max_buffer_size;
		}


	} while (out < ending);
}


/* Template
=========================================================================================================================================
*/

unsigned int CompressClass::EncodeTemplate(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	const unsigned char * const ending = in + length;
	unsigned char * const count = out;

	do {
		// getting input data
		int in_val = *in;
		in++;

		//writing output data
		*out = in_val;
		out++;

	} while (in < ending);

	return (unsigned int)(out - count);
}

void CompressClass::DecodeTemplate(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	const unsigned char * const ending = out + length;

	do {
		// getting input data
		int in_val = *in;
		in++;

		//writing output data
		if (in_val < 235)
			in_val += 1;
		*out = in_val;
		out++;

	} while (out < ending);
}
