

void luaF_close (lua_State *L, StkId level)
==========================================================================
Close all open upvals when a block is out of execution flow.
- All upvals of all closures are linked into "L->openupval".
- Upvals belonging to the about-to-exit block has the "v" field higher than "level".
- "level" is the base of a function call, or the nactvar of a block.
  ("nactvar" is the number of active locals outside a block.)

This function is invoked in two places:
- Returning from a function. (OP_RETURN)
- Exit a block in which some local variables are used as UpVals. Whether there being any
  variables in a block are used UpVals is determined by BlockCnt::upval and BlockCnt::prev. (OP_JMP)
  BockCnt::prev is used to determine if it is the topmost level in a chunck (i.e. loaded source code).
