#ifndef HFD_H
#define HFD_H

#ifdef	LINT_ARGS

	extern uint		HFDecode(word, Handle); /* HUFFMAN decompressor */

#else

	extern uint		HFDecode(); /* HUFFMAN decompressor */

#endif
#endif
