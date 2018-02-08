
#pragma once
//Added by Samuel Wolf
	typedef unsigned int(__stdcall *f_Encode)(const unsigned char * __restrict, unsigned char * __restrict, const unsigned int);
	typedef void(__stdcall *f_Decode)(const unsigned char * __restrict, unsigned char * __restrict, const unsigned int);
	typedef char *(__stdcall *f_Name)();
	typedef unsigned long(__stdcall *f_DWORD)();
	typedef bool(__stdcall *f_Buffer)();

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

	//Added by Samuel Wolf
	f_Encode Encode;
	f_Decode Decode;

	unsigned int Compact( unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length);
	void Uncompact( const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length);
	unsigned int Calcprob(const unsigned char * in, unsigned int length, unsigned char * out=0);
	void Scaleprob(const unsigned int length);
	unsigned int Readprob(const unsigned char * in);
	unsigned int EncodeLAGS( const unsigned char * in, unsigned char * out, const unsigned int length);
	void Decode_And_DeRLE( const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length, unsigned int level);

	unsigned int EncodeTemplate(const unsigned char * in, unsigned char * out, const unsigned int length);
	void DecodeTemplate(const unsigned char * in, unsigned char * out, const unsigned int length);

	unsigned int EncodeRLE(const unsigned char * in, unsigned char * out, const unsigned int length);
	void DecodeRLE(const unsigned char * in, unsigned char * out, const unsigned int length);
	unsigned int Charprob(const unsigned char * in, unsigned int length, unsigned char * out, unsigned char * prob_table);

	unsigned int EncodeHUF(const unsigned char * in, unsigned char * out, const unsigned int length);
	void DecodeHUF(const unsigned char * in, unsigned char * out, const unsigned int length);

	unsigned int EncodeSF(const unsigned char * in, unsigned char * out, const unsigned int length);
	void DecodeSF(const unsigned char * in, unsigned char * out, const unsigned int length);
	
	unsigned int EncodeLZ77(const unsigned char * in, unsigned char * out, const unsigned int length);
	void DecodeLZ77(const unsigned char * in, unsigned char * out, const unsigned int length);

	unsigned int EncodeBW(const unsigned char * in, unsigned char * out, const unsigned int length);
	void DecodeBW(const unsigned char * in, unsigned char * out, const unsigned int length);


};

void ObsoleteUncompact( const unsigned char * in, unsigned char * out, int length, unsigned char * buffer);
