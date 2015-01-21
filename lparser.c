

General Rules
--------------------------------------------------------------
Arraies are usually stored in continuous memory block. The block is pre-allocated
and grow on-the-fly by doubling its size.
	- The actually used item number is of name "n...". For Proto::code, the actually
	  generated code instruction number is FuncState::pc.
	- the allocated memory size is of name "size...",
	- both in terms of the number of items (not in bytes).



LClosure
-----------------------------        ----------------- ----------------
|  GCObject*	next		|
|  lu_byte		tt			|         CommonHeader   
|  lu_byte		marked		|
-----------------------------        -------           ClosureHeader
|  lu_byte		nupvalues	|
|  GCObject*	gclist		|
-----------------------------        ----------------- -----------------
|  Proto*		p			|
-----------------------------
|  UpVal*		upvalue		|
|  ...						|
|  ...						|
-----------------------------





FuncState
-----------------------------
|  Proto*		f			|
|  Table*		h			|		hash-table of "f->k", for resuing const whenever possible
|  FuncState*	prev		|
|  LexState*	ls			|
|  BlockCnt*	bl			|		chain of nesting blocks
|  int 			pc 			|		count of "f->code" and "f->lineinfo".
|  int 			lasttarget	|
|  int 			jpc			|		list of JMP instructions pending to the next "pc"
|  ...						|
|  int 			nk			|		number of already-used slot in "f->k".
|  ...						|
|  int 			firstlocal	|		first local index in "dyd".
|  short		nlocvars	|		number of all local vars within the current function, used items in "f->locvars".
----------------------------------------------------------
|  lu_byte		nactvar		|		number of currently active local vars, "nactvar <= nlocalvars" because some blocks
|							|		already exited. its maximal value is the number of regs allocated to the current
|							|		function with a name. note some statments allocate temp "var" without name
----------------------------------------------------------
|  lu_byte		nups		|		number of currently parsed up-values
|  ...						|
-----------------------------



Dyndata
-----------------------------
|  Vardesc*		arr			|		array of (short index)s, indices to Proto::locvars
|							|		this array itself is indexed by "reg" number. note that items in Proto::locvars	might
|							|		reuse the same "reg" numbers as this array grows and shrinks
|  int 			n 			|
|  int 			size		|
-----------------------------
|  Labellist* 	gt 			|
|  Labellist* 	label		|
-----------------------------



expdesc
-----------------------------
|  expkind		k			|
|  int			u.info		|
|  int			t			|
|  int			f			|
-----------------------------


static int explist (LexState *ls, expdesc *v)
==========================================================================
Generate code as follows:
Put comma-separated expression into adajcent "reg"s, except for the last one.

Note: The general rule is functions other than "lua_K..." does not generate
	  code into Proto::code. This method handles more than one expressions but
	  accepts and resues single "v", so it generates actual code to Proto::code
	  and only leaves the code for the last expression in the list "open".



static void assignment (LexState *ls, struct LHS_assign *lh, int nvars)
==========================================================================
An invocation to "assignment()" handles a suffixing portion of an "assignment-stat",
which is a type of "exprstat" (although this is not a type defined in syntax).

For example:
a, b, c, d, e = f, g, h, i

Then an assignment could be one of the follows:
- , b, c, d, e = f, g, h, i
- , c, d, e = f, g, h, i
- , d, e = f, g, h, i
- , e = f, g, h, i
- = f, g, h, i

But not the follows:
- a, b, c, d, e = f, g, h, i 		// this is a whole "exprstat"
- f, g, h, i 						// this is the tailing "explist", must contain the "="

For a multi-assignment, "assignment()" recursively invokes as many level as the
number of left-hand vars.



static void singlevar (LexState *ls, expdesc *var)
==========================================================================
Initialize "var" for a single NAME. The resulted "var" content is:
	1. VLOCAL.
	2. VUPVAL, or
	3. VINDEXED, for a global var.




static int singlevaraux (FuncState *fs, TString *n, expdesc *var, int base)
==========================================================================
base:	indicating if this is the local-level (as 1) or upper-level (as 0).
		if a var is found at an upper-level, mark BlockCnt.upval.
		BlockCnt.upval makes leaveblock() inserts a JMP to close upvals.

		along the path from a FuncState where an upvalue is found (call level n)
		to the current "fs", call level 0, a new item is added to FunState::f->upvalues
		from level n-1 to 0. 

return:	VVOID if the name "n" is not found. A global var.
		VLOCAL if found at the current level. Only meaningful when "base" is 0.
			When "base" is greater than 0, the found var is marked as "used-as-
			upval", and the return value is ignored by the lower level (inner-
			nested) searching.
		VUPVAL if found at the upper level.



static int searchupvalue (FuncState *fs, TString *name)
==========================================================================
Return the index of "fs->upvalues" of the upvalue, which has the given name "name".
Return -1 if no upvalue of the name found.



static int searchvar (FuncState *fs, TString *n)
==========================================================================
Search a local var with the given name "n" within the given function-level
"fs". Return the "reg" index the local var residing.



static LocVar *getlocvar (FuncState *fs, int i)
==========================================================================
return information of the var: name, start/end pc of valid region

i:		"reg" number of the local var
fs:		represent the current function level



static int newupvalue (FuncState *fs, TString *name, expdesc *v)
==========================================================================
Add a new slot into "fs->upvalues".
Note: "v->k" should be either
		1. VLOCAL, meaning an upvalue at the immediate outter block, or
		2. VUPVAL, meaning an upvalue at the indirectly outter block, for
		   which a slot is added to "fs->upvalues" for each "fs" along the
		   nesting-path.
Note: used only in two places:
		1. when create a chunk, creating the "_ENV".
		2. in singlevaraux().



static void codestring (LexState *ls, expdesc *e, TString *s)
==========================================================================
Add a string constant "s" to "ls->fs->f->k".
Set the index to "e->u.info", the value to "e->k".

e:	out parameter, set as a constant with content of "s".
s:	the content of the new string constant.




static TString *str_checkname (LexState *ls)
==========================================================================
Assure the current token is a NAME. Return the string of the NAME and advance
the Lexer state, for one token.




lua_load (lua_State *L, lua_Reader reader, void *data,
						 const char *chunkname, const char *mode)
==========================================================================
lua_load (L, reader, data, chunkname, mode)
	luaZ_init(L, &z, reader, data)
	luaD_protectedparser(L, &z, chunkname, mode)
		[SParser p]
		luaZ_initbuffer(L, &p.buff)
		luaD_pcall(L, f_parser, &p, ...)
			luaD_rawrunprotected(L, f_parser, &p)
				_setjmp(...)
				f_parser(L, &p)
					luaY_parser(L, p->z, &p->buff, &p->dyd, chunkname, ...)
						[LexState lexstate]
						[FuncState funcstate]
						luaF_newLclosure(L, 1)									// ==> cl
							luaC_newobj(L, LUA_TLCL, ...)
						setclLvalue(L, L->top, cl)
						luaF_newproto(L)										// ==> cl.l.p
						luaS_new(L, chunkname)									// ==> cl.l.p.source
						luaX_setinput(L, &lexstate, p->z, funcstate.f->source, ...)
							luaS_new(L, LUA_ENV)								// ==> lexstate.envn
							luaS_fix(lexstate->envn)
							luaZ_resizebuffer(L, &p->buff, LUA_MINBUFFER)
						mainfunc(&lexstate, &funcstate)
							[BlockCnt bl]
							[expdesc v]
							open_func(&lexstate, &funcstate, &bl)
								luaH_new(L)										// ==> funcstate.h
								enterblock(&funcstate, bl, 0);
							init_exp(&v, VLOCAL, 0)
							newupvalue(&funcstate, lexstate.envn, &v)
								checklimit(&funcstate, funcstate.nups + 1, MAXUPVAL, "upvalues")
								luaM_growvector(L, funcstate.f->upvalues, funcstat.nups, f->sizeupvalues,
								                Upvaldesc, MAXUPVAL, "upvalues");
								luaC_objbarrier(L, funcstate.f, chunkname)
							luaX_next(&lexstate)
							statlist(&lexstate)
							close_func(&lexstate)




static void statlist (LexState *ls)
==========================================================================
statlist(ls)
	block_follow(ls, 1)




static void ifstat (LexState *ls, int line)
==========================================================================
GOTO state
------------------------------------------
ifstat(ls, line)
	[escapelist]
	test_then_block(ls, &escapelist)
		[expdesc v]
		expr(ls, &v)
		checknext(ls, TK_THEN)
		luaK_goiffalse(ls->fs, &v)
			luaK_dischargevars(ls->fs, &v)
				freereg(fs, u.ind.idx)




Expressions Apperaing In (Only):
------------------------------------------
1. Rvalue of assignment statement.
2. Rvalue of local statement.
3. Cond of "if", "while", "repeat".



Expression Type
------------------------------------------
VNIL		|	Instuction has not been emitted.
VTRUE		|
VFALSE		|
VK 			|
VKFLT 		|
VKINT 		|
VJMP:			The expression is a comparison expression (i.e. EQ, LT, LE, NE, GE, GT).
VRELOCABLE:		The target register (usually R(A)) is not yet determined (set as 0).
				The instruction is already put into Proto.
				e->u.info stores the location of the instruction.
VNONRELOC:		The instruction has been emitted to FuncState::f->code.
				e->u.info stores the register storing the result value. The instruction is
				complete and must not be modified (hence the type NONRELOC).
VVARARG:		The target reg could be ether determined or not:
					1. If determined, it is a multi-value which is NOT adjusted.
					2. If undetermined, it will be either:
						2.a) assigned to a free reg for a multi-assignement.
						2.b) adjust to one value and the target reg left pent, type changed
							 to VRELOCABLE.
VCALL:			The reg location of the returned value is already determined by OP_CALL,
				and will be transfer into  "VNONRELOC" with no further post-fixing.
VINDEXED:		The instruction of evaluate the table and the key have already been in Proto.
				The instruction of evaluate the value is NOT in Proto.



Methods Modifying "e->k"
==========================================================================
1. The "..exp2.." functions make sure an expression is set in a register, with VNONRELOC as result.
2. The "..discharge.." functions process only some types of expression, leaving others handled by
   other "..discharge.." or "..exp2.." functions.
4. No "..dischare.." function processes VJMP expressions. It's left to "..exp2.." functions.
5. The functions with prefix "luaK_" are interfacing with the parser.
6. The functions with prefix "luaK_" do not specify which reg the value should be put. They use
   "2anyreg" or "2nextreg", or "2value" (meaing either to reg, or have a constant result).
7. The functions with "2reg" do not have "luaK_" prefix because the register allocation is done
   completely inside the code-generator.
==========================================================================

	void luaK_exp2nextreg (FuncState *fs, expdesc *e)
	---------------------------------------------------
	Set "e" to the next free register, even when "e" is VNONRELOC (in such case generating
	an "OP_MOV").

	int luaK_exp2anyreg (FuncState *fs, expdesc *e)
	---------------------------------------------------
	Assure "e" to be in a register. If "e" is VNONRELOC, doing nothing (??).

	void exp2reg (FuncState *fs, expdesc *e, int reg)
	---------------------------------------------------
	The cover function which gurantees the "e" to be VNONRELOC. Its main purpose
	is handling "VJMP". All other types are delegated to "discharge2reg()".
	---------------------------------------------------
	* VJMP			=>		VNONRELOC
	* (discharge2reg)

	static void discharge2reg (FuncState *fs, expdesc *e, int reg)
	---------------------------------------------------
	With a specified "reg", this function makes most of "e" to "VNONRELOC".
	"VJMP" and "VVOID" are left unchanged. "VJMP" will be handled by "exp2reg()".
	---------------------------------------------------
	* VNIL			=>		VNONRELOC
	* VFALSE		=>		VNONRELOC
	* VTRUE			=>		VNONRELOC
	* VK 			=>		VNONRELOC
	* VKFLT			=>		VNONRELOC
	* VKINT			=>		VNONRELOC
	* VRELOCABLE	=>		VNONRELOC
	* VNONRELOC		=>		VNONRELOC	|	insert an "OP_MOVE".
	* (luaK_dischargevars)

	void luaK_dischargevars (FuncState *fs, expdesc *e)
	---------------------------------------------------
	* VLOCAL 		=>		VNONRELOC
	* VUPVAL		=>		VRELOCABLE
	* VINDEXED		=>		VRELOCABLE
	* VCALL			=>		VNONRELOC	|	(luaK_setoneret), pre-determined reg location
	* VVARARG		=>		VRELOCABLE	|	(luaK_setoneret), only adjust-to-one VVARARG could be relocable.

	void luaK_setoneret (FuncState *fs, expdesc *e)
	---------------------------------------------------
	* VCALL			=>		VNONRELOC	|	e->u.info is the reg A of the OP_CALL.
	* VVARARG		=>		VRELOCABLE	|	set reg B to 2 (meaning copying one value through OP_VARARG)

	int luaK_exp2RK (FuncState *fs, expdesc *e)
	---------------------------------------------------
	* VTRUE			=>		VK 			|
	* VFALSE		=>		VK 			|
	* VNIL			=>		VK 			|	=> (luaK_dischargevars)
	* VKNUM			=>		VK 			|
	* VK 			=>		VK 			|

	void luaK_self (FuncState *fs, expdesc *e, expdesc *key)
	---------------------------------------------------
	*				=>		VNONRELOC


Expression Evaluation Dependencies
==========================================================================

	luaK_exp2RK				=>		luaK_exp2val  (luaK_exp2anyreg)?

	luaK_exp2val			=>		luaK_exp2anyreg		|
									luaK_dischargevars

	luaK_exp2anyreg			=>		luaK_dischargevars  (exp2reg | luaK_exp2nextreg)?

	luaK_exp2nextreg		=>		luaK_dischargevars  exp2reg

	luaK_dischargevars		=>		luaK_setoneret

	exp2reg 				=> 		discharge2reg

	discharge2reg			=>		luaK_dischargevars


static void freeexp (FuncState *fs, expdesc *e)
==========================================================================
"e" should be a non-relocatable expression whose register is the last allocated
one.



static void freereg (FuncState *fs, int reg)
==========================================================================
Register "reg" should be a constant, or a register outside the current block
(less than fs->nactvar), or the last allocated register.

For the first two cases, the function is no-op. For the second, it will release
the register slot (by fs->freereg--).



void luaK_dischargevars (FuncState *fs, expdesc *e)
==========================================================================
Change the "e" to either VNONRELOC or VRELOCABLE. (Exception, not handle VJMP.)



void luaK_setoneret (FuncState *fs, expdesc *e)
==========================================================================
Invoked when the result of "e" is adjusted to only one value.
"e" is a list, either the return value of a function (VCALL) or a VVARARG.
	1. It calls getcode() to retrive the instruction.
		- For e->k being VCALL, it should be a OP_CALL.
		- For e->k being VVARARG, it should be a OP_VARARG.
	2. For VCALL, set e->u.info as the reg A of the OP_CALL, and change type
	   to VNONRELOC.
	   For VVARARG, set reg B of OP_VARARG as 2 (meaing copying one value),
	   leave reg A zero and set the type VRELOCABLE.



static void discharge2reg (FuncState *fs, expdesc *e, int reg)
==========================================================================
Make sure the result of "e" get into register "reg". (Excpetion, not handle VJMP)

If there is already some code generated in Proto::code (see the above section "Expression Type"),
the "luaK_dischargevars()" will make sure the code is patched that only register "A" is left.

Then this function handle one of the following cases:
	1. If "e" is constant, the function emit constant-loading instruction:
		OP_LOADNIL, OP_LOADBOOL, OP_LOADK/OP_LOADKX,
		or add new item into Proto::k.
	2. If "e" is VRELOCABLE, patch the reg "A" in the instruction as "reg".
	3. If "e" is VNONRELOC, emit OP_MOVE to copy the value to reg "reg".

------------------------------------------------------------------------------------------------
VUPVAL			| 	OP_GETUPVAL reg, e->u.info, 0			|	R(reg) := UpValue[e->u.info]
------------------------------------------------------------------------------------------------
VINDEXED		|	OP_GETTABUP 0, e->u.ind.t, e->u.ind.ind |
------------------------------------------------------------------------------------------------
VNIL			|	OP_LOADNIL reg, 1 						|	R(reg) := nil
VFALSE			|	OP_LOADBOOL reg, 0						|	R(reg) := FALSE
VTRUE			|	OP_LOADBOOL reg, 1						|	R(reg) := TRUE
VK (normal)		|	OP_LOADK reg, e->u.info 				|	R(reg) := e->u.info
------------------------------------------------------------------------------------------------
VK (ex_large)	|	OP_LOADKX reg, 0 						|	R(reg) := e->u.info
				|	OP_EXTRAARG, e->u.info 					|
------------------------------------------------------------------------------------------------
VRELOCABLE		|	(no new instruction)					|
	

	discharge2reg(fs, e, reg)				// e->k == VUPVAL
		luaK_dischargevars(fs, e)
			luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0)
			e->k <== VRELOCABLE
		getcode(fs, e)						// ==> pc
		SETARG_A(*pc, reg)

	discharge2reg(fs, e, reg)				// e->k == VINDEXED
		luaK_dischargevars(fs, e)
			freereg(fs, e->u.ind.idx)


	discharge2reg(fs, e, reg)				// e->k == VRELOCABLE
		getcode(fs, e)						// ==> pc
		SETARG_A(*pc, reg)
	


Methods Creating "expdesc"
==========================================================================

	static void singlevar (LexState *ls, expdesc *var)
	---------------------------------------------------
		Only as a "string" constant for indexing "_ENV".

	static void fieldsel (LexState *ls, expdesc *v)
	---------------------------------------------------
		Only as a "string" constant for a table.

	static void recfield (LexState *ls, struct ConsControl *cc)
	---------------------------------------------------
		key:	as "string" constant
		val:	value expression to assign to a table field "key"

	static void funcargs (LexState *ls, expdesc *f, int line)
	---------------------------------------------------
		As the argument list expression.
		Should be either "VCALL" or "VVARARG".

	static void suffixedexp (LexState *ls, expdesc *v)
	---------------------------------------------------
		If following a ":", treated as a "string" constant.
		If encompassed by a "[]", treated as a normal expression.

	static BinOpr subexpr (LexState *ls, expdesc *v, int limit)
	---------------------------------------------------
		For the inner recursively-invoked "subexpr()".

	static void assignment (LexState *ls, struct LHS_assign *lh, int nvars)
	---------------------------------------------------
		For the "rvalue" exprlist.

	static int cond (LexState *ls)
	---------------------------------------------------

static int exp1 (LexState *ls)
static void forlist (LexState *ls, TString *indexname)
static void test_then_block (LexState *ls, int *escapelist)
static void localfunc (LexState *ls)
static void localstat (LexState *ls)
static void funcstat (LexState *ls, int line)
static void retstat (LexState *ls)
static void mainfunc (LexState *ls, FuncState *fs)

static void constructor (LexState *ls, expdesc *t)							// through ConsControl
static void assignment (LexState *ls, struct LHS_assign *lh, int nvars)		// through LHS_assign
static void exprstat (LexState *ls)											// through LHS_assign


mainfunc			=>		statlist


statlist			=>		(statement)*


statement			=>		";"									|
							ifstat								|
							whilestat							|
							"do" block "end"					|
							forstat								|
							repeatstat							|
							funcstat							|
							"local" (localfunc | localstat)		|
							"::" labelstat						|
							"return" retstat					|
							"goto" gotostat						|
							exprstat


ifstat				=>		(test_then_block)+ ["else" block] "end"


test_then_block		=>		("if" | "elseif") expr "then"
							(gotostat | statlist)


whilestat			=>		"while" expr "do" block "end"


forstat				=>		"for" NAME (fornum | forlist) "end"


fornum				=> 		"=" expr "," expr ("," expr)? forbody


forlist				=>		("," NAME)+ "in" explist forbody


forbody				=>		"do" block


gotostat			=>		"goto" label  |  "break"


funcstat			=>		funcname body


block 				=>		statlist

		Note:	"block"s are "statlist"s which must be surrounded by "begin"/"end" pair.
				"block"'s sematic is repsonsible for storing block-level of the internal
				parser state.


exprstat			=>		suffixedexp (assignment)?


assignment			=>		"," suffixedexp assignment			|
							"=" explist

		Note:	"assignment" by its own is not a completed statement. it should always be a
				part of a "exprstat". An "exprstat" becomes an "assignment-stat" when its sufixing
				part is "assignment".

		Note:	"suffixedexp" involved in "assignment", including the one in the precending
				"exprstat" must all be "NAME". This is checked in "assignment()" when a "="
				is encountered.


explist				=>		expr ("," expr)*


expr 				=>		subexpr


subexpr				=>		(UNOP subexpr | simpleexp) (BIOP subexpr)*


simpleexp			=>		NUMBER  |  STRING  |  NIL  |  TRUE  |  FASLE  |  DOTS  |
							constructor		   |
							"function" body	   |
							suffixedexp


suffixedexp			=>		primaryexp (fieldsel | yindex | (":" NAME funcargs) | funcargs)?

		Note:	A "suffixed" expr consists of a "primaryexp" and optionally its suffixed part.

		Note:	Function invocations are always "suffixedexp". The call instruction is generated
				from within "funcargs" according to the "expdesc" filled by the "primaryexp" (and
				"suffixedexp" if it is an object-oriented method).


primaryexp			=>		"(" expr ")" | NAME




UNOP:	"#", "-", "not"






