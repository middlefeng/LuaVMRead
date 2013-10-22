

Basic Types
-----------------------------------------------------------
The enum of "basic types" is: LUA_T...
A "basic type" enum occupies only the right-most forth bits.
The fifth right-most bit represent the "sub-type":
	- short/long string
	- c/Lua closure
	...
The seventh right-most bit represent if the type is collectable.


	/*
	** tags for Tagged Values have the following use of bits:
	** bits 0-3: actual tag (a LUA_T* value)
	** bits 4-5: variant bits
	** bit 6: whether value is collectable
	*/

