/*
 *	PKZIP compression routines
 */

#include	"pk.h"
#include "implode.h"
#include	"memmgr.h"
#include	"string.h"
#include	"fileio.h"
#include	"resource.h"
#include	"errmsg.h"

Handle	dcompBuffer;

// variables used by pk mem functions
static uchar far *pkReadPtr;
static uchar far *pkWritePtr;
static uint amountToRead;
static int pkFd;

static void far pascal pkWriteToMem(char far *buff, unsigned short far *size);
static unsigned far pascal pkReadFromDisk(char far *buff, unsigned short int far *size);
static void far pascal pkWriteToDisk(char far *buff, unsigned short far *size);
static unsigned far pascal pkReadFromMem(char far *buff, unsigned short int far *size);

void pkExplode(int fd, uchar far *dst, unsigned length)
{
	pkFd = fd;
	pkWritePtr = dst;
	amountToRead = length;

	//pkRead and pkWrite must use pkFd, pkWritePtr, and amountToRead
	if (explode(pkReadFromDisk, pkWriteToMem, (char far*) *dcompBuffer))
		Panic(E_EXPLODE);
}

void pkImplode(int fd, uchar far *src, unsigned length)
{
   uint type  = CMP_BINARY;		/* Use BINARY compression */
   uint dsize = 4096;				/* Use 4K dictionary     */
	Handle workBuf;

	pkFd = fd;
	pkReadPtr = src;
	amountToRead = length;

	workBuf = GetResHandle(35256U);	//35256 minimum work buffer for implode

	//pkRead and pkWrite must use pkFd, pkReadPtr, and amountToRead
	implode(pkReadFromMem, pkWriteToDisk, (char far*) *workBuf,
		(unsigned short far*) &type, (unsigned short far*) &dsize);

	DisposeHandle(workBuf);
}

uint pkImplode2Mem(uchar far *dst, uchar far *src, unsigned length)
{
   uint type  = CMP_BINARY;		/* Use BINARY compression */
   uint dsize = 1024;				/* Use 1K dictionary (fast compression for mem check) */
	Handle workBuf;

	pkWritePtr = dst;
	pkReadPtr = src;
	amountToRead = length;

	workBuf = GetResHandle(35256U);	//35256 minimum work buffer for implode

	//pkRead and pkWrite must use pkWritePtr, pkReadPtr, and amountToRead
	implode(pkReadFromMem, pkWriteToMem, (char far*) *workBuf,
		(unsigned short far*) &type, (unsigned short far*) &dsize);

	DisposeHandle(workBuf);

	return((uint)(pkWritePtr - dst));
}

static void far pascal pkWriteToMem(char far *buff, unsigned short far *size)
{
	// for output we will just be copying to the area allocated by the
	// outHandle.  However, in order to emulate the file position concept,
	// (since we can assume explode will write in chunks)
	// it is necessary to keep a global pointer that we can keep adjusting
	// with the size of the output data

	FarMemcpy(pkWritePtr, buff, *size);
	pkWritePtr += *size;

}

static unsigned far pascal pkReadFromDisk(char far *buff, unsigned short int far *size)
{
	uint theSize;


	if(amountToRead < *size)			// make sure we don't try to read more
		theSize = amountToRead;			// than we need to - we know the 
	else										// compressed length
		theSize = *size;

	amountToRead -= theSize;

	return ReadDos(pkFd, buff, theSize);	// a special routine in fileio.s for this purpose
}

static void far pascal pkWriteToDisk(char far *buff, unsigned short far *size)
{
	WriteDos(pkFd, buff, *size);
}

static unsigned far pascal pkReadFromMem(char far *buff, unsigned short int far *size)
{
	uint theSize;


	if(amountToRead < *size)			// make sure we don't try to read more
		theSize = amountToRead;			// than we need to - we know the 
	else										// compressed length
		theSize = *size;

	FarMemcpy(buff, pkReadPtr, theSize);
	pkReadPtr += theSize;
	amountToRead -= theSize;

	return(theSize);
}
