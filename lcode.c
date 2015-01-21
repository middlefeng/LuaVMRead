

General
-------------------------------------------------------
- luaK_code... returns the "pc" of the instruction just being generated.
- if a "luaK_..." function accepts an "expdesc" as parameter, it usually does not put
  instruction into Proto::code, at least not putting the last instruction.

- A "test-mode" instruction means one of the following three:
	- OP_EQ,
	- OP_LT,
	- OP_LE，
	- OP_TEST,
	- OP_TESTSET.
- A "test-mode" instruction is followed by a JMP instruction.
- The first three, OP_EQ, OP_LT, OP_LE, is used for generating a comp-instruction, 
  luaK_posfix() ==> codecomp() generates it as "jmp-if-true" (the same as "go-if-false").
  That means the "true-block"/"then-block" is located last.
- In "goiftrue()", the "test-bit" shall be inverted to invert the "jmp-if-true" behavior
  in order to have the "then-block" located first (so if-then, if-else-then must use goifture()).



Global Tables
-------------------------------------------------------
OpCode:			Enum type which is the index of the following two arraies.
luaP_opnames:	All OP names in "char*".
luaP_opmodes:	OP mode of each instruction, indexed as the same order to "luaP_opnames".




void luaK_setoneret (FuncState *fs, expdesc *e)
==========================================================================
Set the expression of a "VVARARG" or "VCALL".
1. For VVARARG, set its "B" as 2, meaning adjusting to 1 value.
2. For VCALL, set "e->k" to "NONRELOC" and "e->u.info" to the original "A".



static int addk (FuncState *fs, TValue *key, TValue *v)
==========================================================================
Add "v" to field "k" of the "struct Proto".
Before really creating an entry in "k", it check and reuse an eixsting one whenever possible.
It check by search in "fs->h", which is a compile-time hash-index of "k".

fs->f->k grows by double if there is no enough room, and all new items are set as "nil".
The number of actually used slots is stored in fs->nk.

Returns the location in "f->k" of the newly-added constant.



static int luaK_code (FuncState *fs, Instruction i)
==========================================================================
Put Instruction "i" into "fs->f->code".
Put the line number into "fs->f->lineinfo".
Increase fs->pc. Note "pc" is not part of fs->f (Proto) but of f (FuncState).




void luaK_exp2val (FuncState *fs, expdesc *e)
==========================================================================
luaK_exp2val (fs, e)
	hasjumps(e)
	[true]
		luaK_exp2anyreg(fs, e)
	[false]
		luaK_dischargevars(fs, e)



static int jumponcond (FuncState *fs, expdesc *e, int cond)
==========================================================================
	
	If "e" is a RELOC "NOT"
	--------------------------------------
	TEST 		RB(ie), !cond
	JMP 		0,		"jpc"

	Otherwise
	--------------------------------------
	TESTSET 	-1, "Rdischarge(e)", cond
	JMP 		0,	"jpc"



static int condjump (FuncState *fs, OpCode op, int A, int B, int C)
==========================================================================
"op"	A, B, C
JMP 	0, "jpc"



void luaK_goiftrue (FuncState *fs, expdesc *e)
==========================================================================
For "VJMP"
-------------------------------------------------------
e->t:	JMP 	0,	"jpc"		// pre-generated, not by this function






int luaK_jump (FuncState *fs)
==========================================================================
Generate an "OP_JMP" instruction.
Put the current "fs->jpc" to the target of the instruction.
Clean "fs->jpc".
Return the instruction's "pc".




static void luaK_concat (FuncState *fs, int *l1, int l2)
==========================================================================
Add instruction at index "l2" to a chain of jump instructions, which is called a patch-list.
-------------------------------------------------------
l1:		header to a patch-list (i.e. a sequence of unpatched JMP instructions, more strictly speaking,
		the "pc" indices of those instructions). NO_JUMP means the list is empty.
l2:		a "pc" index.
-------------------------------------------------------
1. Noop, when l2 == NO_JUMP.
2. *l1 = l2, when l1 == NO_JUMP. Mean to initialize the list.
3. if "*l1" has a non-"NO_JUMP" value, trace to the end of the patch-list and set "l2" as the target of
   the tail instruction, meaning add l2 to the end of the patch-list.



void luaK_patchtohere (FuncState *fs, int list)
==========================================================================
The function is used to patch a list of JMPs (i.e. a patch-list) to the location of the next instruction.

It stores the "list" to fs->jpc. When Lua compiler generate the next instruction, the fs->jpc will be
cleared to NO_JUMP, and the "list" will be:
	1. Patched to the location of next instruction if it is not a JMP, or
	2. Be augmented with the newly-generated JMP instruction, and the list will be usually put back to
	   fs->jpc later on, or be handled differently in some cases.

Effectively the same to:
luaK_concat(fs, &fs->jpc, list)

With a minor side-effect that modifying "fs->lasttarget".



void luaK_dischargevars (FuncState *fs, expdesc *e)
==========================================================================
Gaurantee there is one value avaialbe for "e".
1. If the value already exists in a register (VCALL, VLOCAL), then no new instruction
   spat out, no instruction patched. The pre-determined reg is put into "e->u.info".
2. If the value is not allocated with a register (VUPVAL, VINDEXED, VVARARG), this function
     1) Spit out a "GET..." instruction for VUPVAL/VINDEXED, left the target reg pent.
     2) Patch the VVARARG instruct to assign only one value (e.g. Bx=2), left the target reg pent.
----------------------------------------------------
1. VNONRELOC:	in "e->u.info" register.
2. VRELOCABLE:	"e->u.info" point to an instruction, whose "A" parameter is pending.
3. Other.



void luaK_setreturns (FuncState *fs, expdesc *e, int nresults)
==========================================================================
Adjust a multi-value.
1. For VCALL, patch Cx as "nresults+1".
2. For VVARARG, patch Bx as "nresults+1", patch Ax (destination reg) to a free reg slot.



static void exp2reg (FuncState *fs, expdesc *e, int reg)
==========================================================================

For e->k == VJMP and NOT following a "TESTSET"
----------------------------------------------------------
			JMP 			0, "e->t"
	e->f:	LOAD_BOOL		"reg", 0, 1
	e->t:	LOAD_BOOL		"reg", 1, 0

For anything else, just delegate to discharge2reg().

For e->k == VJMP
----------------------------------------------------------
exp2reg (fs, e, reg)




int luaK_exp2RK (FuncState *fs, expdesc *e)
==========================================================================
Make result of "e" in a register or in a constant entry (prepared for instruction accepting
RK parameters).



static void discharge2reg (FuncState *fs, expdesc *e, int reg)
==========================================================================
Make the result of "e" into register "reg".
Except for "VVOID" and "VJMP".



static void dischargejpc (FuncState *fs)
==========================================================================
Travserse the "fs->jpc" list. Modify all "TESTSET" instructions involved in the list to
"TEST" instructions. Patch all insructions' sBx to "fs->pc".
-------------------------------------------------------
patchlistaux(fs, fs->jpc, fs->pc, NO_REG, fs->pc)




static int code_label (FuncState *fs, int A, int b, int jump)
==========================================================================
Emit the last two instructions for a logical expression, one for the case the yielded
value is TRUE, the other for FALSE.

All sub-expressions in the same logical expression, which yield non-bollean values,
jumps to the instruction following the last instruction generated by this function.
The location of that instruction (the one following the last) is called "vtarget".



static void patchlistaux (FuncState *fs, int list, int vtarget, int reg, int dtarget)
==========================================================================
Traverse the patch-list pointed by "list".
	1. For all instructions which do NOT follow a "TESTSET", set their sBx to "dtarget".
	2. For all which follow "TESTSET"s, set the "TESTSET"'s' "register A" to "reg" and their
	   sBx to "vtarget".

Note:
	1. If "reg" is NO_REG, then "vtarget" should be the same to "dtarget".
	2. "vtarget" means "value" target. It is used by a relation-expr whose value is not
	   necessarily boolean, but could be any type. When the expr code jumps to "vtarget",
	   "register A" is the value of the expr which is set by the "TESTSET" instruction.
	3. Param "vtarget" and "reg" are used only in exp2reg().





static int patchtestreg (FuncState *fs, int node, int reg)
==========================================================================
Patch the "register A" (i.e. the destination register) of a "TESTSET" instruction.
The "TESTSET" would be either the one prior to "node", or the "node".
1. Do nothing and returns 0 if the "node"-1 or "node" instruction is not "TESTSET".
2. If "reg" is neither "NO_REG" nor equal to "register B" of the instruction, set
   it as "register A" of the instruction.
3. Otherwise, change the instruction to a "TEST" instruction, set the old
   "register B" as "register A". (For a TEST, "register B" is ignored.)



static int need_value (FuncState *fs, int list)
==========================================================================
The name of the function means the patch-list specified by "list" does not set a value
in a register by its own. Therefore it *needs* a value from somewhere else.

Not setting a value means not all instructions are TESTSET in "list".

This function is used by exp2reg() only. When an expr with JMP needs a value, exp2reg()
will generate two LOADBOOL instructions to generate the final boolean value.



static int getjump (FuncState *fs, int pc)
==========================================================================
The instruction at index "pc" must be of sBx form.
Return the target absolute index of the jump, or "NO_JUMP" when the offset is "NO_JUMP".
The absolute index of the jump is computed according to the relative jump offset obtained
from the instrcution, and the "pc" of the instruction.




static Instruction *getjumpcontrol (FuncState *fs, int pc)
==========================================================================
Test if the instruction proceeding "pc" is a "test instruction" (see "testTMode(m)" for details).
If so, return the "test instruction", otherwise return the instruction at index "pc".




static void fixjump (FuncState *fs, int pc, int dest)
==========================================================================
Write the sBx part of the instruction at index "pc" to have it jump to "dest".



testTMode(m)
==========================================================================
If the OP_ "m" is a "test"-mode instruction.
A "test"-mode instruction followed by a jump instruction.
Test mode includes:
- OP_EQ
- OP_LT
- OP_LE
- OP_TEST
- OP_TESTSET




assignment


