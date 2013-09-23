


static void correctstack (lua_State *L, TValue *oldstack)
==========================================================================
Set all "abstract" address in the "oldstack" to having the same relative address
to the current "L->stack".

