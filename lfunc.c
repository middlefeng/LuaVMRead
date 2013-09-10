

void luaF_close (lua_State *L, StkId level)
==========================================================================
Close all open upvals when a block is out of execution flow.
- All upvals of all closures are linked into "L->openupval".
- Upvals belonging to the about-to-exit block has the "v" field higher than "level".
- "level" is the base of a function call, or the nactvar of a block.
  ("nactvar" is the number of active locals outside a block.)
