


l_noret luaG_runerror (lua_State *L, const char *fmt, ...)
==========================================================================
luaG_runerror(L, fmt, ...)
	[va_list argp]
	va_start(argp, fmt)
	luaO_pushvfstring(L, fmt, argp)
		strchr(fmt, '%')
		luaD_checkstack(L, 2)