//	Lagarith v1.3.27, copyright 2011 by Ben Greenwood.
//	http://lags.leetcode.net/codec.html
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either version 2
//	of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef _COMPACT_CPP
#define _COMPACT_CPP

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <math.h>
#include "lagarith.h"
#include "fibonacci.h"
#include "zerorle.h"

#include "log.h"
#include <sstream>

// scale the byte probablities so the cumulative
// probabilty is equal to a power of 2
void CompressClass::Scaleprob(unsigned int length){
	assert(length > 0);
	assert(length < 0x80000000);

	unsigned int temp = 1;
	while ( temp < length )
		temp<<=1;

	int a;

	if ( temp != length ){
		double factor = ((int)temp)/((double)(int)length);
		double temp_array[256];

		
		for ( a = 0; a < 256; a++ ){
			temp_array[a] = ((int)prob_ranges[a+1])*factor;
		}
		for ( a = 0; a < 256; a++ ){
			prob_ranges[a+1]=(int)temp_array[a];
		}
		unsigned int newlen=0;
		for ( a = 1; a < 257; a++ ){
			newlen+=prob_ranges[a];
		}

		newlen = temp-newlen;
		assert(newlen < 0x80000000);

		if ( newlen & 0x80000000 ){ // should never happen
			prob_ranges[1]+=newlen;
			newlen=0;
		}

		a =0;
		unsigned int b=0;
		while ( newlen  ){
			if ( prob_ranges[b+1] ){
				prob_ranges[b+1]++;
				newlen--;
			}
			b++;
			b&=0x7f;
		}
	}

	for ( a =0; temp; a++)
		temp>>=1;
	scale =a-1;
	for ( a = 1; a < 257; a++ ){
		prob_ranges[a]+=prob_ranges[a-1];
	}
}

// read the byte frequency header
unsigned int CompressClass::Readprob(const unsigned char * in){

	unsigned int length=0;
	unsigned int skip;

	//prob_ranges[0]=0;
	memset(prob_ranges,0,sizeof(prob_ranges));

	skip = FibonacciDecode(in,&prob_ranges[1]);
	if ( !skip )
		return 0;

	for (unsigned int a =1;a< 257; a++){
		length+=prob_ranges[a];
	}

	if ( !length )
		return 0;

	Scaleprob(length);
	return skip;
}

// Determine the frequency of each byte in a byte stream; the frequencies are then scaled
// so the total is a power of 2. This allows binary shifts to be used instead of some
// multiply and divides in the compression/decompression routines.
// If out is set, the freqencies are also written and the output size returned.
unsigned int CompressClass::Calcprob(const unsigned char * const in, unsigned int length, unsigned char * out){

	unsigned int table2[256];
	memset(prob_ranges,0,257*sizeof(unsigned int));
	memset(table2,0,sizeof(table2));
	unsigned int a=0;
	for (a=0; a < (length&(~1)); a+=2 ){
		prob_ranges[in[a]+1]++;
		table2[in[a+1]]++;
	}
	if ( a < length ){
		prob_ranges[in[a]+1]++;
	}
	for ( a=0;a<256;a++){
		prob_ranges[a+1]+=table2[a];
	}
 
	// Clamp prob_ranges total to 1<<19 to ensure the range coder has enough precision to work with;
	// Larger totals reduce compression due to the reduced precision of the range variable, and
	// totals larger than 1<<22 can crash the range coder. Lower clamp values reduce the decoder speed
	// slightly on affected video.
	const int clamp_size = 1<<19;
	if ( out && length > clamp_size ){
		double factor = clamp_size;
		factor/=length;
		double temp[256];
		for ( int a=0;a<256;a++){
			temp[a]=((int)prob_ranges[a+1])*factor;
		}
		unsigned int total=0;
		for ( int a=0;a<256;a++){
			int scaled_val=(int)temp[a];
			if ( scaled_val== 0 && prob_ranges[a+1] ){
				scaled_val++;
			}
			total+=scaled_val;
			prob_ranges[a+1]=scaled_val;
		}
		int adjust = total<clamp_size?1:-1;
		int a=0;
		while ( total != clamp_size){
			if ( prob_ranges[a+1]>1){
				prob_ranges[a+1]+=adjust;
				total+=adjust;
			}
			a++;
			a&=255;
		}
		length = clamp_size;
	}

	unsigned int size=0;
	if ( out != NULL ){
		size = FibonacciEncode(&prob_ranges[1],out);
	}

	Scaleprob(length);

	return size;
}

// This function encapsilates compressing a byte stream.
// It applies a modified run length encoding if it saves enough bytes, 
// writes the frequency header, and range compresses the byte stream.

// !!!Warning!!! in[length] through in[length+3] will be thrashed
// in[length+8] must be readable

//Changed by Samuel Wolf
unsigned int CompressClass::Compact( unsigned char * in, unsigned char * out, const unsigned int length){

	unsigned int bytes_used=0;

	unsigned int skip = 0;
	skip += Encode(in, out + skip, length);
	bytes_used = skip;

	//if (comp_variant == 0) {
	//	out[0] = 0;
	//	unsigned int skip = Calcprob(in, length, out + 1);
	//	skip += EncodeLAGS(in, out + skip + 1, length) + 1;
	//	bytes_used = skip;
	//}

	//if (comp_variant == 1) {
	//	out[0] = 1;
	//	unsigned int skip = 1;
	//	skip += EncodeTemplate(in, out + skip, length) ;
	//	bytes_used = skip;

	//}

	//if (comp_variant == 2) {
	//	out[0] = 2;
	//	unsigned int skip = 1;
	//	skip += EncodeRLE(in, out + skip, length);
	//	bytes_used = skip;


	//}
	//
	//if (comp_variant == 3) {
	//	out[0] = 3;
	//	unsigned int skip = 1;
	//	skip += Encode(in, out + skip, length);
	//	bytes_used = skip;



	//}

	//if (comp_variant == 4) {
	//	unsigned int skip = 1;
	//	out[0] = 4;
	//	skip += EncodeSF(in, out + skip + 1, length);
	//	bytes_used = skip;
	//	
	//}

	//if (comp_variant == 5) {
	//	unsigned int skip = 1;
	//	out[0] = comp_variant;
	//	skip += EncodeLZ77(in, out + skip, length);
	//	bytes_used = skip;
	//	if (bytes_used > length)
	//	{
	//		Log("TO MUCH BYTES\n");
	//		/*out[0] = 1;
	//		skip = 1;
	//		skip += EncodeTemplate(in, out + skip, length);
	//		bytes_used = skip;*/
	//	}
	//}

	//if (comp_variant == 6) {
	//	out[0] = 6;
	//	unsigned int skip = 5;
	//	unsigned char * buffer = new unsigned char[length * 4];
	//	memset(buffer, 0, sizeof(buffer));
	//	//unsigned char  buffer[120000];
	//	
	//	unsigned int size = EncodeRLE(in, buffer, length);
	//	skip += EncodeHUF(buffer, out + skip, size);
	//	bytes_used = skip;

	//	out[1] = size >> 32;
	//	out[2] = size >> 16;
	//	out[3] = size >> 8;
	//	out[4] = size;

	//	delete buffer;
	//}

	//if (comp_variant == 7) {
	//	out[0] = 7;
	//	unsigned int skip = 5;
	//	unsigned char * buffer = new unsigned char[length * 4];
	//	memset(buffer, 0, sizeof(buffer));
	//	//unsigned char  buffer[120000];

	//	unsigned int size = EncodeHUF(in, buffer, length);
	//	skip += EncodeRLE(buffer, out + skip, size);
	//	bytes_used = skip;

	//	out[1] = size >> 32;
	//	out[2] = size >> 16;
	//	out[3] = size >> 8;
	//	out[4] = size;

	//	delete buffer;
	//}

	//if (comp_variant == 8) {
	//	out[0] = 8;
	//	unsigned int skip = 5;
	//	unsigned char * buffer = new unsigned char[length * 4];
	//	memset(buffer, 0, sizeof(buffer));
	//	//unsigned char  buffer[120000];

	//	unsigned int size = EncodeBW(in, buffer, length);
	//	skip += EncodeRLE(buffer, out + skip, size);
	//	bytes_used = skip;

	//	out[1] = size >> 32;
	//	out[2] = size >> 16;
	//	out[3] = size >> 8;
	//	out[4] = size;

	//	delete buffer;

	//}


	return bytes_used;
}

// this function encapsulates decompressing a byte stream
// Changed by Samuell Wolf
void CompressClass::Uncompact( const unsigned char * in, unsigned char * out, const unsigned int length){

	unsigned int skip = 0;
	Decode(in + skip, out, length);
	

	//int variant = in[0];
	//if (variant == 0) {
	//	int skip = *(int*)(in + 1);

	//	// Adobe Premeire Pro tends to zero out the data when multi-threading is enabled...
	//	// This will help avoid errors
	//	if (!skip)
	//		return;
	//	skip = Readprob(in + 1);
	//	if (!skip)
	//		return;
	//	Decode_And_DeRLE(in + skip + 1, out, length, 0);
	//}

	//if (variant == 1) {
	//	unsigned int skip = 1;
	//	DecodeTemplate(in + skip, out, length);
	//	
	//}

	//if (variant == 2) {
	//	unsigned int skip = 1;
	//	DecodeRLE(in + skip, out, length);
	//}

	//if (variant == 3) {
	//	unsigned int skip = 1;
	//	Decode(in + skip, out, length);
	//}

	//if (variant == 4) {
	//	unsigned int skip = 1;
	//	DecodeSF(in + skip, out, length);

	//}

	//if (variant == 5) {
	//	unsigned int skip = 1;
	//	DecodeLZ77(in + skip, out, length);

	//}

	//if (variant == 6) {
	//	unsigned int skip = 5;
	//	unsigned int size = (in[1] << 32) + (in[2] << 16) + (in[3] << 8) + in[4];
	//	unsigned char * buffer = new unsigned char[size * 3];
	//	memset(buffer, 0, size * 3);
	//	DecodeHUF(in + skip, buffer, size);
	//	DecodeRLE(buffer, out, length);

	//	delete buffer;
	//}

	//if (variant == 7) {
	//	unsigned int skip = 5;
	//	unsigned int size = (in[1] << 32) + (in[2] << 16) + (in[3] << 8) + in[4];
	//	unsigned char * buffer = new unsigned char[size * 3];
	//	memset(buffer, 0, size * 3);
	//	DecodeRLE(in + skip, buffer, size);
	//	DecodeHUF(buffer, out, length);

	//	delete buffer;
	//}

	//if (variant == 8) {
	//	unsigned int skip = 5;
	//	unsigned int size = (in[1] << 32) + (in[2] << 16) + (in[3] << 8) + in[4];
	//	unsigned char * buffer = new unsigned char[size * 3];
	//	memset(buffer, 0, size * 3);
	//	DecodeRLE(in + skip, buffer, size);
	//	DecodeBW(buffer, out, length);

	//	delete buffer;

	//}
	
}

// initalized the buffers used by RLE and range coding routines
bool CompressClass::InitCompressBuffers(const unsigned int length){
	// buffer must be large enough to hold all 3 RLE levels at their worst case
	buffer = (unsigned char *)lag_aligned_malloc(buffer,length*3/2+length*5/4+32,8,"Compress::temp");
	if ( !buffer ){
		FreeCompressBuffers();
		return false;
	}
	return true;
}

// free the buffers used by RLE and range coding routines
void CompressClass::FreeCompressBuffers(){	
	lag_aligned_free( buffer,"Compress::buffer");
}

CompressClass::CompressClass(){
	buffer=NULL;
}

CompressClass::~CompressClass(){
	FreeCompressBuffers();
}

//MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);

#endif