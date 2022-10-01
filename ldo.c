


static void correctstack (lua_State *L, TValue *oldstack)
==========================================================================
Set all "abstract" address in the "oldstack" to having the same relative address
to the current "L->stack".



int luaD_growstack (lua_State *L, int n, int raiseerror)
==========================================================================



int luaD_reallocstack (lua_State *L, int newsize, int raiseerror)
==========================================================================
luaD_reallocstack(L, newsize, raiseerror)
	luaM_reallocvector(L, L->stack, L->stacksize, newsize, StackValue)		// ==> newstack
		luaM_realloc_(L, L->stack,  cast_sizet(L->stacksize) * sizeof(StackValue), cast_sizet(newsize) * sizeof(StackValue))
			firsttry(G(L), L->stack, cast_sizet(L->stacksize) * sizeof(StackValue), cast_sizet(newsize) * sizeof(StackValue))
				G(L)->frealloc(G(L)->ud, L->stack, cast_sizet(L->stacksize) * sizeof(StackValue), cast_sizet(newsize) * sizeof(StackValue))
			tryagain(L, L->stack, cast_sizet(L->stacksize) * sizeof(StackValue), cast_sizet(newsize) * sizeof(StackValue))
				luaC_fullgc(L, 1)
			...
	[for lim = lim = L->stacksize; lim < newsize]
		setnilvalue((newstack + lim)->val)
	correctstack(L, L->stack, newstack)
	L->stack = newstack
  	L->stacksize = newsize
  	L->stack_last = L->stack + newsize - EXTRA_STACK

