


void *luaM_realloc_ (lua_State *L, void *block, size_t osize, size_t nsize)
--------------------------------------------------------------------------------------------
Augment L->l_G->frealloc with GC.
It increases L->l_G->GCdebt by (nsize - osize).



luaM_reallocv(L, b, on, n, e)
--------------------------------------------------------------------------------------------
on:	old number of elements.
n:	new number of elements.
e:	size of a single element. "n * e" pass to luaM_realloc_(...) as "nsize".




luaM_reallocvector(L, v, oldn, n, t)
--------------------------------------------------------------------------------------------
Cast result to type "t" and use type "t" for element size.




void *luaM_growaux_ (lua_State *L, void *block, int *size, size_t size_elems,
                     int limit, const char *what)
--------------------------------------------------------------------------------------------
Try to double the size of block, or to "limit" if the doubled-size is larger than limit.
size		-	numbr of elements
size_elems	-	size in bytes of a single element



luaM_growvector(L, v, nelems, size, t, limit, e)
--------------------------------------------------------------------------------------------
Try to double the size of v if nelems + 1 > size.
size	-	original and new size
t		-	type of elements
e		-	error message will be "too many [e] (limit is [limit])"


luaM_growvector(L,
				funcstate.f->upvalues,
				funcstat.nups,
				f->sizeupvalues,
				Upvaldesc, MAXUPVAL, "upvalues");