

lua_State
-----------------------------        ----------------- ----------------
|  GCObject*	next		|
|  lu_byte		tt			|         CommonHeader   
|  lu_byte		marked		|
-----------------------------        ----------------- ----------------
|  ...						|
|  unsigned short nCcalls	|		  with dual-meaning: 1. Nested C call, 2. Nested syntac structure.
|  ...						|
-----------------------------





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