


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


linktable(h,p)
================================================================================
Link table "h" on to list "p". The *next* pointer is h->gclist.


static GCObject *udata2finalize (global_State *g)
================================================================================
Move the head of "g->tobefnz" to "g->allgc".
Clear its "SEPARATED" bit.



static void separatetobefnz (lua_State *L, int all)
================================================================================
Move un-reached or all objects in L->l_G->finobj to L->l_G->tobefnz.



static GCObject **sweeplist (lua_State *L, GCObject **p, lu_mem count)
================================================================================
sweeplist (L, p, count)
	otherwhite(L->l_G)
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





void luaC_fullgc (lua_State *L, int isemergency) 
================================================================================
luaC_fullgc(L, isemergency)
	callallpendingfinalizers(L, 1)
		resetoldbit(L->l_G->tobefnz)			       // remove old
		GCTM(L, 1)
			udata2finalize(L->l_G)
				resetbit(gch(o)->marked, SEPARATED)
				keepinvariantout(L->l_G)
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
	entersweep(L)
		sweeptolive(L, &L->l_G->finobj, &n)
		sweeptolive(L, &L->l_G->allgc, &n)
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





