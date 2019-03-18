#pragma once



/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

// MD5
class MD5
{
	public:
		MD5();
		explicit MD5 (const unsigned char*);

		void Init ();
		int Update (unsigned char *, ULONG64);
		void Final ();
		void Get (
			unsigned char* pMd5Hash
		);

		void GetLowerDWord (
			DWORD* pMd5Hash
		);


	protected:	
		void Transform (UINT4 [4], const unsigned char [64]);
		void Encode (unsigned char *, UINT4 *, unsigned int);
		void Decode (UINT4 *, const unsigned char *, unsigned int);
		void _Update (unsigned char *, unsigned int);

	protected:	
		UINT4 m_state[4];                                   /* state (ABCD) */
		UINT4 m_count[2];        /* number of bits, modulo 2^64 (lsb first) */
		unsigned char m_buffer[64];                         /* input buffer */

		unsigned char sum[16];

	public:
		virtual ~MD5();
};
