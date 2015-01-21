


Value vs. GCObject vs. GCUnion
---------------------
Values (or TValue, more strictly speaking) are what is associated with Lua variables.
TValues reside in stack. Values represent UpVals. TValues act as table fields.

GCObjects reside only in the heap. They are referred by Values.
GCUnion represents all subclasses of GCObject.



StkId --> TValue -->  lua_TValue  --> (contains) TValuefields


TValuefields
-----------------
Value is untagged (having no type information). TValue means "tagged" value, value augmented
with type tag (tt_).
-----------------
value	(Value)     ---> (union)  	gc	(GCObject)
tt_		(int)						p	(void)
									b	(int)
									f	(lua_CFunction)
									n	(ua_Number)



setnilvalue(obj)
==================
Assign *tt_* field as LUA_TNIL





GCUnion
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


TString
-------------------------------------------------
	GCObject*		next		|
	lu_byte			tt			|	Common header
	lu_byte			marked		|
-------------------------------------------------
	lu_byte			extra		|	
	unsigned int	hash		|	
	size_t			len 		|
	TString*		next		|
-------------------------------------------------






UpVal
-----------------
UpVal is no longer a GCObject like it was in 5.2.
UpVal is no longer linked to g->uvhead, but only linked to L->openupval.
-------------------------------------------------------------------------------------
	TValue*				v					|	linked to either in-stack value or to "value"
	lu_mem 				refcount			|
-------------------------------------------------------------------------------------
	TValue 		value 	|	UpVal*	next	|	linked to "L->openupval". In 5.2 it was to g->uvhead
						|	int 	touched	|
-------------------------------------------------------------------------------------




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



UData
-----------------

-------------------------------------------------
	GCObject*	next			|
	lu_byte		tt				|	Common header
	lu_byte		marked			|
-------------------------------------------------
	lu_byte		ttuv_			|	New in 5.3
	Table*		metatable		|
	size_t		len 			|
	Value 		user_			|	New in 5.3. Holding any Lua type. In 5.2, it was "env" which holds table type only.
								|	Note: "env" in 5.2 does not really mean env. Just an ordinary table.
-------------------------------------------------
	...			(real data)
-------------------------------------------------



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

