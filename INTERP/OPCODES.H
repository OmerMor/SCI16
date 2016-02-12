/*******************************************************************************
 *
 *						OPCODES
 *
 *
 * The opcodes for SPM are partially bit-mapped.  The byte is mapped as
 *
 * 	txxx xxxb
 *
 * where
 * 	t = 0	->	Arithmetic, stack, etc. operations
 * 	t = 1	->	Load/store operations
 * 
 * 	b = 0	->	Following address/value is a word.
 * 	b = 1	->	Following address/value is a byte.
 *
 *
 * The load/store operations are further bit-mapped:
 *
 * 	1boo idvv
 *
 * where
 * 	oo = 0	->	Load
 * 	oo = 1	->	Store
 * 	oo = 2	->	Increment, then load
 * 	oo = 3	->	Decrement, then load
 *
 * 	i = 0	->	Load/store from address as-is.
 * 	i = 1	->	Load/store indexed.  Index is in A.
 *
 *	d = 0	->	Load to accumulator
 *	d = 1	->	Load to stack
 *
 * 	vv = 0	->	Global
 * 	vv = 1	->	Local
 * 	vv = 2	->	Tmp
 * 	vv = 3	->	Parameter (load only -- specifies a different stack
 * 				frame than auto)
 *
 * Load operations leave the requested value in the A.  Store operations
 * store the A in memory unless they are indexed, in which case they store
 * the top value on the stack, popping the stack.
 ******************************************************************************/



#define	OP_LDST		0x80		/* load/store if set */
#define	OP_BYTE		0x01		/* byte operation if set, word otw */

#define	OP_TYPE		0x60		/* mask for operation type */
#define	OP_LOAD		0x00		/* load */
#define	OP_STORE		0x20		/* store */
#define	OP_INC		0x40		/* increment operation */
#define	OP_DEC		0x60		/* decrement operation */

#define	OP_INDEX		0x10		/* indexed op if set, non-indexed otw */

#define	OP_STACK		0x08		/* load to stack if set */

#define	OP_VAR		0x06		/* mask for var type */
#define	OP_GLOBAL		0x00		/* global var */
#define	OP_LOCAL		0x02		/* local var */
#define	OP_TMP		0x04		/* temporary var (on the stack) */
#define	OP_PARM		0x06		/* parameter (different stack frame than tmp) */


/* Opcodes other than load/store.
 */
#define	OP_bnot		0x00
#define	OP_add		0x02
#define	OP_sub		0x04
#define	OP_mul		0x06
#define	OP_div		0x08
#define	OP_mod		0x0a
#define	OP_shr		0x0c
#define	OP_shl		0x0e
#define	OP_xor		0x10
#define	OP_and		0x12
#define	OP_or		0x14

#define	OP_neg		0x16
#define	OP_not		0x18

#define	OP_eq		0x1a
#define	OP_ne		0x1c
#define	OP_gt		0x1e
#define	OP_ge		0x20
#define	OP_lt		0x22
#define	OP_le		0x24

#define	OP_ugt		0x26
#define	OP_uge		0x28
#define	OP_ult		0x2a
#define	OP_ule		0x2c

#define	OP_bt		0x2e
#define	OP_bnt		0x30
#define	OP_jmp		0x32

#define	OP_loadi		0x34
#define	OP_push		0x36
#define	OP_pushi		0x38
#define	OP_toss		0x3a
#define	OP_dup		0x3c
#define	OP_link		0x3e

#define	OP_call		0x40
#define	OP_callk		0x42
#define	OP_callb		0x44
#define	OP_calle		0x46

#define	OP_return		0x48

#define	OP_send		0x4a
/*
#define	OP_sendk		0x4c
#define	OP_sendb		0x4e
*/
#define	OP_class		0x50
/*
#define	OP_objID		0x52
*/
#define	OP_self		0x54
#define	OP_super		0x56
#define	OP_rest		0x58
#define	OP_lea		0x5a
#define	OP_selfID		0x5c
/*
#define	OP_superc		0x5e
*/
#define	OP_pprev		0x60

#define	OP_pToa		0x62
#define	OP_aTop		0x64
#define	OP_pTos		0x66
#define	OP_sTop		0x68
#define	OP_ipToa		0x6a
#define	OP_dpToa		0x6c
#define	OP_ipTos		0x6e
#define	OP_dpTos		0x70

#define	OP_lofsa		0x72
#define	OP_lofss		0x74

