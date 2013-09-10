


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
|  ...						|
|  int 			firstlocal	|		first local index in "dyd".
|  short		nlocvars	|		number of local vars (current-level)
|  lu_byte		nactvar		|		number of currently active local vars (all-levels)
|  lu_byte		nups		|		number of currently parsed up-values
|  ...						|
-----------------------------





expdesc
-----------------------------
|  expkind		k			|
|  int			u.info		|
|  int			t			|
|  int			e			|
-----------------------------



static int singlevaraux (FuncState *fs, TString *n, expdesc *var, int base)
==========================================================================
base:	indicating if this is the local-level (as 1) or upper-level (as 0).
		if a var is found at an upper-level, mark BlockCnt.upval.
		BlockCnt.upval makes leaveblock() inserts a JMP to close upvals. 



static int searchvar (FuncState *fs, TString *n)
==========================================================================



static LocVar *getlocvar (FuncState *fs, int i)
==========================================================================





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
VRELOCABLE:		The target register (usually R(A)) is not yet determined (set as 0).
				The instruction is already put into Proto.
VVARARG:		The target reg could be ether determined or not:
					1. If determined, it is a multi-value which is NOT adjusted.
					2. If undetermined, it will be either:
						2.a) assigned to a free reg for a multi-assignement.
						2.b) adjust to one value and the target reg left pent, type changed
							 to VRELOCABLE.
VCALL:			The reg location of the returned value is already determined by OP_CALL,
				and will be transfer into  "VNONRELOC" with no further post-fixing.
VINDEXED:		The instruction is NOT in Proto.



Methods Modifying "e->k"
==========================================================================
1. The "..exp2.." functions make sure an expression is set in a register, with VNONRELOC as result.
2. The "..discharge.." functions processes only some types of expression, leaving others handled by
   other "..discharge.." or "..exp2.." functions.

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
	* VKNUM			=>		VNONRELOC
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
	* VCALL			=>		VNONRELOC
	* VVARARG		=>		VRELOCABLE

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


statlis				=>		(statement)*


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


ifstat				=>		(test_then_block)+ ["else" block]


test_then_block		=>		("if" | "elseif") expr "then"
							(gotostat | statlist)


gotostat			=>		"goto" label  |  "break"


funcstat			=>		funcname body


exprstat			=>		suffixedexp (assignment)?


assignment			=>		"," suffixedexp assignment			|
							"=" explist


explist				=>		expr ("," expr)*


expr 				=>		subexpr


subexpr				=>		(UNOP subexpr | simpleexp) (BIOP subexpr)*


simpleexp			=>		NUMBER  |  STRING  |  NIL  |  TRUE  |  FASLE  |  DOTS  |
							constructor		   |
							"function" body	   |
							suffixedexp


suffixedexp			=>		primaryexp (fieldsel | yindex | (":" NAME funcargs) | funcargs)


primaryexp			=>		"(" expr ")" | NAME




UNOP:	"#", "-", "not"






