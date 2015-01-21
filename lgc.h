

GCObject::marked
===========================================================================================================

	FINALIZEDBIT		In 5.2, it means the object is in "tobefnz" list.
						In 5.3 beta, it means the object is in either "finobj" or in "tobefnz" list.
						In 5.3 beta, after an object's "__gc" called and the object is moved back to "allgc" list,
						there will be no flag indicating the object was "finalized". In 5.2, such an object still
						has "FINALIZEDBIT" in its mark but that state is not used.

	SEPARATED			In 5.2, it means the objct is in either "finobj" or in "tobefnz" list.
						In 5.3 beta, its meaning is replaced by FINALIZEDBIT, and this flag is removed.
						There is no longer a flag indicating an object is in "tobefnz" (which was FINALIZEDBIT
						in 5.1). 
