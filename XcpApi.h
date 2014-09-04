#ifndef XCPAPI_H
#define XCPAPI_H

//	The value "X" is used instead of "T" because "T" was used for a text message.  Eventually this has to be renamed.
#define d_chAPIe_TaskDownloading					'x'
#define d_szAPIe_TaskDownloading_tsI				"x" _tsI
#define d_chAPIe_TaskSending						'X'
#define d_szAPIe_TaskSending_tsI					"X" _tsI

	#define d_chAPIa_TaskDataSizeTotal							's'			// Total size of the task's data
	#define d_szAPIa_TaskDataSizeTotal_i						" s='$i'"
	#define d_chAPIa_TaskDataOffset								'O'
	#define d_szAPIa_TaskDataOffset_i							" O='$i'"
	#define d_chAPIa_TaskDataBinary								'b'			// Chunk of data
	#define d_szAPIa_TaskDataBinary_Bii							" b='{o|}'"


#define d_szXCPe_MessageTextComposingStarted			"<a n='c'/>"
#define d_szXCPe_MessageTextComposingPaused				"<a n='c'>p</a>"

#endif // XCPAPI_H
