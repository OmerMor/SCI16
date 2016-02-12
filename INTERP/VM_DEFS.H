/* * * * *
* VM_DEFS.H
* COPYRIGHT 1989 BY DYNAMIX, INC
*
* Contains VM defines.
*
* MODIFICATION HISTORY
* NAME            DATE     WHAT
*
* Richard Rayl    08/23/88 Moved all definitions from VM_GLOBS.H
* M Edwards       10/17/88 Added FLIP defs, changed TRUE/FALSE defs,
*                          and moved key defs to vm_keys.h
* M Edwards       11/30/8  Added equates from grafix.inc.
* ME              03/07/89 Added def's for MCGA, AMIGAVM, ATARIST.
* ME              03/28/89 Moved uchar, ushort, ulong defs to vm_types.h.
* ME              03/29/89 Added conditional inclusion.
* ME              05/11/89 Added bitfield graphics mode defines.
* 
* * * * */


	#define MAXWIDTH  320
	#define MAXHEIGHT 200




	/* VM Sci defines	 */

	#define	VM_SCALABLE 0x0001
	#define	VM_AUTO_SCALE 0x0002
	#define	VM_SCALE_BASE 128
	#define	VM_SCALE_SHIFT 7


	



