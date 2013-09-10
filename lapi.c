

lua_settop(lua_State *L, int idx)



lua_pcallk
=====================================
lua_pcallk(L, ..., errfunc, ...)
	[CallS c]
	index2addr(L, errfunc)
	savestack(L, o)
	luaD_pcall(L, f_call, &c, ...)
		luaD_rawrunprotected(L, f_call, &c)
			_setjmp(...)
			f_call(L, &c)
				luaD_call(L, c->func, c->nresult, 0)
					luaD_precall(L, c->func, c->nresult)     // Note 1
					luaV_execute(...)                        //
	adjustresults(...)




Note 1: A normal call purely in byte-code invoke "luaD_precall()" and
        re-enter the exec-loop by a "goto".



lua_pcallk (with continuation)
=====================================
lua_pcallk(L, ..., errfunc, ctx, k)
	[CallS c]
		index2addr(L, errfunc)
		savestack(L, o)



luaD_precall (lua_State *L, StkId func, int nresults)
=====================================
(C Closure) (return 1 for C-closure)
-------------------------
luaD_precall(L, func, nresult)
	savestack(L, func)						// ==> funcr
	clCvalue(func)
	next_ci(L)
	restorestack(L, funcr)					// ==> ci->func, same to "func ==> ci->func"
	luaC_checkGC(L)
	(*f)(...)								// ==> n
	luaD_poscall(L, L->top - n)
		L->ci->func ==> res
		[Loop from "L->top - n" to "L->top"]
			setobjs2s(L, ...)
		[Loop]
			setnilvalue(...)