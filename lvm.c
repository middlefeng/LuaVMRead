

lua_State
---------------------------------        ----------------- ----------------
|  GCObject*		next		|
|  lu_byte			tt			|         CommonHeader   
|  lu_byte			marked		|
---------------------------------        ----------------- ----------------
|  lu_byte			status		|		  Store the status of a stack-shrunk event, e.g. LUA_YIELD, and/or error code
|								|		  of an "unprotected" call (i.e. errorJmp == NULL).
---------------------------------        ----------------- ----------------
|  StkId			top			|
|  global_State*	l_G			|
|  CallInfo*		ci 			|
|  Instruction*		oldpc		|		  (debug info) for trace
|  StkId			stack_last	|		  the first slot of EXTRA_STACK (room beyond maximally-allowed "top" but under "stacksize")
|  StkId			stack 		|
|  int 				stacksize	|
|  unsigned short	nny			|
|  unsigned short	nCcalls		|		  with dual-meaning: 1. Nested C call, 2. Nested syntac structure.
|  ...							|
|  lua_longjmp*		errorJmp	|		  store the location and error-code of longjmp, except coroutine yeild.
|  CallInfo			base_ci		|		  first level CallInfo
---------------------------------



global_State
---------------------------------        ----------------- ----------------
|  lua_Alloc		freealloc	|		  memory allocator with its
|  void*			ud 			|		  user-data
---------------------------------        ----------------- ---------------- ----------------		----------------
|  lu_mem			totalbytes	|		  invariant: totalbytes + GCdebt = sizeof(LG) + "allocated"	|
|  l_mem 			GCdebt		|		  just-allocated bytes										|	Seems not real purpose except
|  lu_mem			GCmemtrav	|																	|	guranteeing some invariants.
|  lu_mem			GCestimate	|		  used as the "debt" of the next step						|
---------------------------------        ----------------- ---------------- ----------------		----------------
|  GCObject**		hash		|
|  lu_int32			nuse		|		  stringtable strt
|  int 				size 		|
---------------------------------        ----------------- ----------------
|  TValue			l_registry	|
|  unsigned int 	seed		|
---------------------------------        ----------------- ----------------
|  lu_byte			currentwhite|
|  lu_byte			gcstate 	|		  GC incremental steps
|  lu_byte 			gckind		|
|  lu_byte			gcrunning	|
|  int 				sweepstrgc	|
|  ...							|





-------------------
|                 |
|  Local          |
|                 |
-------------------
|                 |
|  Copied Fixed   |          ci->p->numparams
|                 |
-------------------   <----  ci->u.l.base             Current
|                 |                                   Stake Frame (Callee)
|                 |
|                 |
|  Var Args       |
|                 |
|                 |
-------------------                                 ------------------------------
|                 |
|  Fixed Params   |          ci->p->numparams
|                 |                                   Previous
-------------------                                   Stack Frame (Caller)
|  Function       |
-------------------   <----  ci->func
|                 |
|                 |


ci->u.l.base is the "top" of the previous function call frame. In Lua, instructions specify var-length register
parameters by a number to indicate registers indexed from that number to the "stack top".

OP_VARARGS re-set the "stack top" if it accept "all" var-args so a following var-args function-call will get
right number of parameters from the "func" register to the "stack top".