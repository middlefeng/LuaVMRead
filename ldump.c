
static void DumpFunction(const Proto* f, DumpState* D)
------------------------------------------------------------------------------------------
Nested function are dumped in DumpConstants().




static void DumpConstants(const Proto* f, DumpState* D)
------------------------------------------------------------------------------------------
Dumping format:
--------------------------------
|	f->sizek					|
--------------------------------
|	(constants)					|
|	...							|
--------------------------------
|	f->sizep (nested funcs)		|
--------------------------------
|	f->p[0]						|
|	f->p[1]						|
|	...							|
--------------------------------