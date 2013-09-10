
StkId --> TValue -->  lua_TValue  --> (contains) TValuefields


TValuefields
-----------------
value	(Value)     ---> (union)  	gc	(GCObject)
tt_		(int)						p	(void)
									b	(int)
									f	(lua_CFunction)
									n	(ua_Number)



setnilvalue(obj)
==================
Assign *tt_* field as LUA_TNIL





GCObject
-----------------
"gclist" is the next node of the gray list if the object resides in one such list.

-------------------------------------------------
	GCObject*	next		|
	lu_byte		tt			|	Common header
	lu_byte		marked		|
-------------------------------------------------
	lu_byte		nupvalues	|	Closure header
	GCObject*	gclist		|
-------------------------------------------------




UpVal
-----------------

-------------------------------------------------------------------------------------
	GCObject*			next				|	linked to "L->openupval"	|
	lu_byte				tt					|								|	Common header
	lu_byte				marked				|								|
-------------------------------------------------------------------------------------
	TValue*				v					|	linked to either in-stack value or to "value"
-------------------------------------------------------------------------------------
	TValue 		value 	|	UpVal*	prev	|
						|	UpVal*	next	|	linked to "g->uphead"





Proto
-----------------
sizecode:		size of "code"
sizelineinfo:	size of "lineinfo"

is_vararg:	It has mere two possible values now, 0 or 1.
upvalues:	This array is for the _description_ of upvalues. For example, if an upval
			shall be on the stack (locals of enclosing function).



Closure
-----------------
upvals:		Reference to real upvals. Its initialization is made according to
			Proto.upvalues specification.





setgcovalue(L,obj,x)
==================
Set TValue "obj" pointing to collectable object "x", including make "obj" and "x"
marked as the same type ("tt_" field of the former and "tt" field of the latter).

