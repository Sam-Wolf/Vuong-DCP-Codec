#include "compact.h"
#include <iostream>
#include <queue>

#include "log.h"
using namespace std;

void getPermutation(unsigned char * in, unsigned char * out, unsigned int length, unsigned int start) {
	unsigned char * in_begin = in;
	unsigned char * in_end = in + length;
	in += start;

	unsigned int ending = length - start;
	for (unsigned int i = 0; i < ending; i++)
	{
		*out = *in;
		out++;
		in++;
	}
	in = in_begin;
	for (unsigned int i = 0; i < start; i++)
	{
		*out = *in;
		out++;
		in++;
	}


}

//bool comparePermutation(const unsigned char * in, unsigned int length, unsigned int p1, unsigned int p2) {
//	const unsigned char * in_p1 = in + p1;
//	const unsigned char * in_p2 = in + p2;
//	const unsigned char * in_ending = in + length;
//
//
//	unsigned int i = 0;
//	while (*in_p1 == *in_p2 && i < length)
//	{
//		in_p1++;
//		in_p2++;
//		i++;
//		if (in_p1 > in_ending)
//			in_p1 = in;
//		if (in_p2 > in_ending)
//			in_p2 = in;
//	}
//
//	return *in_p1 > *in_p2;
//}

bool comparePermutation(const unsigned char * in, unsigned int length, unsigned int p1, unsigned int p2) {

	unsigned int i = 0;
	while (in[p1] == in[p2] && i < length)
	{
		p1++;
		p2++;
		i++;
		if (p1 == length)
			p1 = 0;
		if (p2 == length)
			p2 = 0;
	}

	return in[p1] > in[p2];
}

unsigned int CompressClass::EncodeBW(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	const unsigned char * const ending = in + length;
	unsigned char * const start = out;
	out += 4;

	vector<unsigned int> que;

	for (unsigned int i = 0; i < length; i++) {
		que.push_back(i);
	}

	sort(que.begin(), que.end(), [in, length](unsigned int a, unsigned int b) {return comparePermutation(in, length, a, b); });

	for (unsigned int i = 0; i < length; i++)
	{
		unsigned int pos = que.back();
		if (pos)
			pos -= 1;
		else {
			pos = length - 1;
			*((unsigned int *)start) = i;
		}

		*out = in[pos];
		que.pop_back();
		out++;
	}


	return (unsigned int)(out - start);
}

void CompressClass::DecodeBW(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	unsigned char *  ending = out + length;
	const unsigned char * const start = out;
	unsigned int startpos = *((unsigned int *)in);
	in += 4;
	const unsigned char * const start_in = in;


	vector<pair<unsigned int, unsigned int>> que;

	for (unsigned int i = 0; i < length; i++) {
		pair<unsigned int, unsigned int> p(*in, i);
		que.push_back(p);
		in++;
	}
	sort(que.begin(), que.end());

	/*vector<pair<unsigned int, unsigned int>> positions;

	for (unsigned int i = 0; i < length; i++) {
		pair<unsigned int, unsigned int> p(que.back().second, length - 1 - i);
		positions.push_back(p);
		que.pop_back();
	}
	sort(positions.begin(), positions.end());*/

	unsigned int * posi = new unsigned int[length];
	posi[0] = 0;

	for (unsigned int i = 0; i < length; i++) {
		posi[que.back().second] = length - 1 - i;
		que.pop_back();
	}

	in = start_in;


	unsigned int pos = startpos;
	out = ending - 1;
	for (unsigned int i = 0; i < length; i++)
	{
		*out = *(in + pos);
		out--;
		pos = *(posi + pos);
	}

	delete posi;
	que.clear();
}