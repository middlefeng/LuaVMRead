
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




Table
-----------------

-------------------------------------------------
	GCObject*	next			|
	lu_byte		tt				|	Common header
	lu_byte		marked			|
-------------------------------------------------
	lu_byte		flags			|	Presence of meta-method. "1" means absent. All "1" at initialized.
	lu_byte		lsizenode		|	Log(sizenode), so there is no "sizenode" field
	...							|
	Node*		node 			|	Hashed key-value
	...							|






Proto
-----------------

-------------------------------------------------
	GCObject*	next			|
	lu_byte		tt				|	Common header
	lu_byte		marked			|
-------------------------------------------------
	...							|
--------------------------------------------------------------------------------------------------
	LocVar*		locvars			|	TString* varname	|	debug and parsing information for local-var,
								|	int 	 startpc	|   used for var-seach in parsing. indexed by 
								|	int 	 endpc		|	"Dyndata::actvar.arr".
--------------------------------------------------------------------------------------------------
	...							|
	int 		sizecode		|	size of "code"
	int 		sizelineinfo	|	size of "lineinfo"
	...							|
	lu_byte		is_vararg		|	it has mere two possible values now, 0 or 1.
	lu_byte		upvalues		|	for the _description_ of upvalues. For example, if an upval shall be
								|	on the stack (locals of enclosing function).



Closure
-----------------
upvals:		Reference to real upvals. Its initialization is made according to
			Proto.upvalues specification.





setgcovalue(L,obj,x)
==================
Set TValue "obj" pointing to collectable object "x", including make "obj" and "x"
marked as the same type ("tt_" field of the former and "tt" field of the latter).

