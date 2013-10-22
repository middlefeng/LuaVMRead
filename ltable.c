


hashpow2(t,n)
==========================================================================
n:	hash code, int
t:	table
Return the hashed slot corresponding to "n" in table "t".




hashmod(t,n)
==========================================================================
This method differs from "hashpow2()" in that the underlying algorithm of
moduling is the ordinary "C" moduling rather than a bit-manipulating one optimized
for 2-base-powered size of "t".

"n" is actually moduled by size of t minues 1, rather than size of t.





const TValue *luaH_getstr (Table *t, TString *key)
==========================================================================
Get value if "key" is a short-string.




static Node *mainposition (const Table *t, const TValue *key)
==========================================================================
Get the main hash-ed position for "key".




static Node *hashnum (const Table *t, lua_Number n)
==========================================================================
Return the main hash-ed position for "n".




luai_hashnum(i,n)
==========================================================================
Compute the hashcode of "n" and put it to "i".


