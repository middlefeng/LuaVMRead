


General Process
--------------------------------------------------------------------------------
1. All objects are created white.
2. A reachable object is marked as black, and added to the gray list because its
   refered objects might still remain white (or gray if they in turn have refered
   objects).
3. If all directly and indirectly referred object of a black object is marked black,
   the black object is removed out of the gray list.
4. A black object is not necessarily a reachable object, because it might be
   obsoleted after it is marked.
5. A current-white object is not necessarily unreachable unless GC is in "atomic"
   step.
6. A current-white object MUST be unreachable at "atomic" step because at that 
   point all reachable and some just-becoming-unreachable objects have all been marked.
7. At "atomic" step, all current-white objects are added to the sweep lists, and
   then they become other-white because the while flag is flipped. After "atomic" step,
   all "other-white" objects are guranteed to be unreachable until the next "pause"
   step.





Barriers
--------------------------------------------------------------------------------
A barriers is set when a object is added to another object as the latter's refered
object.

If the "refering" object is a table, then a backward barrier is usually set, making
the table back to the gray-list.

Otherwise, a forward barrier is set, mark the refered object as "black" (but possibly
in the gray-list) in order to avoid putting the refering object into gray-list (but
it might be already there).






Global List
================================================================================
finobj		Objects marked by a meta-table with "__gc" method.
tobefnz		Objects moved from finobj in the "atomic" step, all unreachable.
			They will be:
				1. Remarked at "pause" step; and/or
				2. Be inovked "__gc" upon, and then put to "allgc" list.
allgc		Objects except for those in "finobj" and "tobefnz".





Object Status
================================================================================
Current-white		Unreached yet.
Black				Currently used. Not neccessarily reachable.
Other-white			Unreachable (never used). __gc has been invoked on it.



GCObject *luaC_newobj (lua_State *L, int tt, size_t sz)
================================================================================
This function is simplified from what it was in 5.2. In 5.2, there was a "list" parameter which was
used for only UpVal (which then was a GCObject). UpVal in 5.2, although was GCObject, were not linked
to g->allgc, but instead g->uvhead.



bitmask(b)
================================================================================
Create a mask in which the "b"th bit is "1".


resetbits(x,m)
================================================================================
Set to zero all bits in "x" which are masked by "m".


resetbit(x,b)
================================================================================
Set to zero the "b"th bit of "x".


isdeadm(ow,m)
================================================================================
Ture if "m" has "other_than_current_white" marked.


linkgclist(o,p)
================================================================================
Link object "o" on to list "p". The *next* pointer is h->gclist.
This was named linktable in 5.2.


static GCObject *udata2finalize (global_State *g)
================================================================================
Move the head of "g->tobefnz" to "g->allgc".
Clear its "FINALIZEDBIT" bit. Then return that object (previous head of g->tobefnz).

Immediately after this function, the "__gc" of the returned object must be invoked
by invoking GCTM().

Before this function called, markbeingfnz() is called to mark all objects as black
or gray (and added to the gray-list). Then this function adding them to g->allgc
makes the object subject to normal GC rules (normally reclaimed, but could be
resurrected within "__gc").


static void separatetobefnz (global_State *g, int all)
================================================================================
Move un-reached ("all" is 0) or all ("all" is 1) objects in g->finobj to g->tobefnz.
"all" is set to 1 only at the time that a lua_State is closed.


static GCObject **sweeplist (lua_State *L, GCObject **p, lu_mem count)
================================================================================
Reclaim the dead object in "*p". Mark still alive objects as "current-white".

If "*p" is allgc, then the alive objects wait for the next marking phase.
If "*p" is finobj, they will be moved to tobefnz by separatetobefnz() later.
If "*p" is tobefnz, they will be moved to allgc after "__gc" called by udata2finalize().



static GCObject **sweeplist (lua_State *L, GCObject **p, lu_mem count)
================================================================================
sweeplist (L, p, count)
	otherwhite(L->l_G)			// ==> ow
	luaC_white(L->l_G)			// ==> white
	[iterate over lisp "p"]
		isdeadm(ow, (*p)->marked)
		[true]
			freeobj(L, curr)





static void reallymarkobject (global_State *g, GCObject *o)
================================================================================
1. Objects which do not refer to other colletable objects (userdata, string)
   are marked black.
2. Objects which refer to other collectable objects (table, closure, thread,
   prototype/func) are linked to a gray list, waiting for "propagation" of
   the marks.
3. Upval is marked black and its real value is marked according to the above
   rules.

	Userdata
	-----------------------------------------
	reallymarkobject(g, o)
		white2gray(o)							       // clear all white bits
		markobject(g, mt)
		markobject(g, o->env)
		gray2black(o)

	Table
	-----------------------------------------
	reallymarkobject(g, o)
		white2gray(o)							       // clear all white bits  
		linktable(gco2t(o), &g->gray);
		gray2black(o)



static void separatetobefnz (global_State *g, int all)
================================================================================
separatetobefnz(g, 0)
	findlast(&g->tobefnz)		// ==> lastnext
	[loop over g->finobj: curr]
		iswhite(curr)
			[move curr from g->finobj to the tail of g->tobefnz]


static void callallpendingfinalizers (lua_State *L, int propagateerrors)
================================================================================
callallpendingfinalizers(L, 1)
	[loop on "L->l_G->tobefnz"]
		GCTM(L, 1)
			udata2finalize(L->l_G)
				resetbit(gch(o)->marked, FINALIZEDBIT)
				issweepphase(L->L_G)
					[makewhite(L->l_G, o)]
			setgcovalue(L, &v, L->l_G->allgc)
			luaT_gettmbyobj(L, &v, TM_GC)		       // ==> tm
			[L->l_G->gcrunning = 0]
			luaD_pcall(L, dothecall, NULL, ...);
				luaD_rawrunprotected(L, dothecall, NULL)
					dothecall(L, NULL)
						luaD_call(L, L->top - 2, 0, 0)
							luaD_precall(L, L->top - 2, 0)
							[luaV_execute(L)]	      // only for Lua-function




void luaC_fullgc (lua_State *L, int isemergency) 
================================================================================
luaC_fullgc(L, isemergency)
	keepinvariant(L->l_G)
		[entersweep(L)]
			sweeptolive(L, &L->l_G->allgc, &n)
				[loop on L->l_G->allgc]
					sweeplist(L, &L->l_G->allgc, &n)
	luaC_runtilstate(L, bitmask(GCSpause))





markvalue(g,o)
================================================================================
markvalue(g,o)
	checkconsistency(o)
		iscollectable(o)
			rttype(o)
			righttt(o)
		valiswhite(o)
			iscollectable(o)
			iswhite(gcvalue(o))


traversethread(global_State *g, lua_State *th)
================================================================================
traversethread(g, th)
	[loop from th->stack to th->top: o]
		markobject(o)
	[in atomic step]
		[loop from th->top+1 to th->stack+th->stacksize]
			setnilvalue(o)


Steps
================================================================================

	Pause
	-----------------------------------------
	singlestep(L)					  			         // L->l_G ==> g
		restartcollection(g)
			[clean "gray", "grayagain"]
			[clean "weak", "allweak", "ephemeron"]
			markobject(g, g->mainthread)		         // mainthread ==> gray list
			markvalue(g, &g->l_registry)
			markmt(g)
				[for each in "g->mt"]
					markobject(g, [each])
			markbeingfnz(g)
				[for each in "g->tobefnz"]
					makewhite(g, [each])
					reallymarkobject(g, [each])

	Propregate with gray
	-----------------------------------------
	singlestep(L)					  			         // L->l_G ==> g
		propagatemark(g)
	
	
	Proporgate with no gray
	-----------------------------------------
	singlestep(L)						  	             // L->l_G ==> g
		atomic(L)
			markobject(g, L)
			markvalue(g, &g->l_registry)
			markmt(g)
			remarkupvals(g)
			propagateall(g)						        // assure no gray any more
			retraversegrays(g)	
				propagateall(g)					        // seems no use?
				propagatelist(g, grayagain)
				propagatelist(g, weak)
				propagatelist(g, ephemeron)
			convergeephemerons(g)
			clearkeys(g, g->ephemeron, NULL)
			clearkeys(g, g->allweak, NULL)
			clearvalues(g, g->weak, NULL)
			clearvalues(g, g->allweak, origall)
		entersweep(L)





