#pragma once

//Wrappers will contain any function that we cannot just passthrough
#include "Passthroughs.h"


/*
	Allows you to override what funciton LSASS calls.
Note: Use if you need to do extra checks which may change if MSV1_0 is called at all / what parameteres are passed to MSV1_0
Note2: If you are just going to do debug logging, or playing with the variables byval it is recommended you use one of the overridePassthoughsX functions
		and set a pre_x or post_x function pointer.
*/
void overrideFunctions (functionTables* funcTable);

/*
These 3 functions allow you to specify extra functionality before and after the call to MSV1_0.dll

The call order is as follows:
	LSASS -> funcTable->function() 
				-> pre_x->function()
				-> MSV1_0->function()
				-> post_x->function()

They are generally used for debug logging, or working with data given by MSV1_0.
NOTE: Generally you should NOT change the passed variables in the function
NOTE2: The return result of these functions is ignored

If you need more control then that (eg, you need to do extra checks and change variables passed to the MSV1_0 functions
then use the tables passed to overrideFunctions. This changes what LSASS calls so you can set your own functionality.
*/

void overridePassthroughsLSA(SECPKG_FUNCTION_TABLE* pre_passthroughTable, SECPKG_FUNCTION_TABLE* post_passthroughTable);
void overridePassthroughsSSP(LSA_SECPKG_FUNCTION_TABLE* pre_passthroughTable, LSA_SECPKG_FUNCTION_TABLE* post_passthroughTable);
void overridePassthroughsUSR(SECPKG_USER_FUNCTION_TABLE* pre_passthroughTable, SECPKG_USER_FUNCTION_TABLE* post_passthroughTable);

#ifdef WRAPPER
#include "..\MiTokenAP\NamedPipeClient.h"

extern NamedPipeClient NPC;

#endif