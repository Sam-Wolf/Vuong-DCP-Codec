// Modified by Samuel Wolf

#pragma once
//Added by Samuel Wolf - Begin
	typedef unsigned int(__stdcall *f_Encode)(const unsigned char * __restrict, unsigned char * __restrict, const unsigned int);
	typedef void(__stdcall *f_Decode)(const unsigned char * __restrict, unsigned char * __restrict, const unsigned int);
	typedef char *(__stdcall *f_Name)();
	typedef unsigned long(__stdcall *f_DWORD)();
	typedef bool(__stdcall *f_Buffer)();
//Added by Samuel Wolf - End

class CompressClass {

public:
	unsigned int prob_ranges[257];		// Byte probablity range table
	unsigned int scale;				// Used to replace some multiply/divides with binary shifts,
									// (1<<scale) is equal to the cumulative probabilty of all bytes

	unsigned char * buffer;			// buffer to perform RLE when encoding

	CompressClass();
	~CompressClass();

	bool InitCompressBuffers(const unsigned int length);
	void FreeCompressBuffers();


	//Added by Samuel Wolf - Begin
	f_Encode Encode1;
	f_Decode Decode1;
	f_Encode Encode2;
	f_Decode Decode2;
	f_Encode Encode3;
	f_Decode Decode3;
	unsigned char * comp_settings;

	unsigned int CompactLAGS(unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length);
	void UncompactLAGS(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length);
	//Added by Samuel Wolf - End

	unsigned int Compact( unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length);
	void Uncompact( const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length);
	unsigned int Calcprob(const unsigned char * in, unsigned int length, unsigned char * out=0);
	void Scaleprob(const unsigned int length);
	unsigned int Readprob(const unsigned char * in);
	unsigned int EncodeLAGS(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length);
	void Decode_And_DeRLE( const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length, unsigned int level);
};

void ObsoleteUncompact( const unsigned char * in, unsigned char * out, int length, unsigned char * buffer);
