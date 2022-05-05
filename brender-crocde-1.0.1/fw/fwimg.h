/*
 * Table of exported names
 */
static char * namePointers_BRCORE1[] = {
	"_BrAbort",
	"_BrAddHead",
	"_BrAddTail",
	"_BrDevAdd",
	"_BrDevAddStatic",
	"_BrDevBegin",
	"_BrDevBeginVar",
	"_BrDevCheckAdd",
	"_BrDevContainedCount",
	"_BrDevContainedFind",
	"_BrDevContainedFindMany",
	"_BrDevCount",
	"_BrDevFind",
	"_BrDevFindMany",
	"_BrDevLastBeginQuery",
	"_BrDevLastBeginSet",
	"_BrDevRemove",
	"_BrFailure",
	"_BrFatal",
	"_BrFileAdvance",
	"_BrFileAttributes",
	"_BrFileClose",
	"_BrFileEof",
	"_BrFileGetChar",
	"_BrFileGetLine",
	"_BrFileOpenRead",
	"_BrFileOpenWrite",
	"_BrFilePrintf",
	"_BrFilePutChar",
	"_BrFilePutLine",
	"_BrFileRead",
	"_BrFileWrite",
	"_BrGetEnv",
	"_BrImageAdd",
	"_BrImageDereference",
	"_BrImageLookupName",
	"_BrImageLookupOrdinal",
	"_BrImageReference",
	"_BrImageRemove",
	"_BrInsert",
	"_BrIsAlpha",
	"_BrIsDigit",
	"_BrIsPrint",
	"_BrIsSpace",
	"_BrLexerAllocate",
	"_BrLexerCommentSet",
	"_BrLexerDumpSet",
	"_BrLexerErrorSet",
	"_BrLexerFree",
	"_BrLexerPop",
	"_BrLexerPosition",
	"_BrLexerPushFile",
	"_BrLexerPushString",
	"_BrLexerTokenError",
	"_BrLogPrintf",
	"_BrMemAllocate",
	"_BrMemCalloc",
	"_BrMemCmp",
	"_BrMemCpy",
	"_BrMemFree",
	"_BrMemInquire",
	"_BrMemSet",
	"_BrMemStrDup",
	"_BrNamePatternMatch",
	"_BrNewList",
	"_BrObjectContainerFree",
	"_BrObjectListAllocate",
	"_BrParseFixed",
	"_BrParseFloat",
	"_BrParseInteger",
	"_BrParseMatrixFixed",
	"_BrParseMatrixFloat",
	"_BrParseVectorFixed",
	"_BrParseVectorFloat",
	"_BrPrimitiveLibraryFind",
	"_BrQsort",
	"_BrRegistryAdd",
	"_BrRegistryAddMany",
	"_BrRegistryAddStatic",
	"_BrRegistryClear",
	"_BrRegistryCount",
	"_BrRegistryEnum",
	"_BrRegistryFind",
	"_BrRegistryFindMany",
	"_BrRegistryNew",
	"_BrRegistryNewStatic",
	"_BrRegistryRemove",
	"_BrRegistryRemoveMany",
	"_BrRegistryRemoveStatic",
	"_BrRemHead",
	"_BrRemTail",
	"_BrRemove",
	"_BrRendererFacilityFind",
	"_BrResAdd",
	"_BrResAllocate",
	"_BrResCheck",
	"_BrResChildEnum",
	"_BrResClass",
	"_BrResDump",
	"_BrResFree",
	"_BrResFreeNoCallback",
	"_BrResIsChild",
	"_BrResRemove",
	"_BrResSize",
	"_BrResSizeTotal",
	"_BrResStrDup",
	"_BrSScanf",
	"_BrScratchAllocate",
	"_BrScratchFlush",
	"_BrScratchFree",
	"_BrScratchInquire",
	"_BrScratchString",
	"_BrScratchStringSize",
	"_BrSimpleAddHead",
	"_BrSimpleInsert",
	"_BrSimpleNewList",
	"_BrSimpleRemHead",
	"_BrSimpleRemove",
	"_BrSprintf",
	"_BrSprintfN",
	"_BrStrCat",
	"_BrStrCmp",
	"_BrStrCpy",
	"_BrStrICmp",
	"_BrStrLen",
	"_BrStrNCmp",
	"_BrStrNCpy",
	"_BrStrNICmp",
	"_BrStrRChr",
	"_BrStrToD",
	"_BrStrToF",
	"_BrStrToL",
	"_BrStrToUL",
	"_BrStringToTokenValue",
	"_BrSwap16",
	"_BrSwap32",
	"_BrSwapBlock",
	"_BrSwapFloat",
	"_BrTVTemplateAllocate",
	"_BrTVTemplateFree",
	"_BrTokenCount",
	"_BrTokenCreate",
	"_BrTokenFind",
	"_BrTokenFindMany",
	"_BrTokenIdentifier",
	"_BrTokenType",
	"_BrTokenValueCompare",
	"_BrTokenValueComparePartial",
	"_BrTokenValueDump",
	"_BrTokenValueQuery",
	"_BrTokenValueQueryAll",
	"_BrTokenValueQueryAllSize",
	"_BrTokenValueQueryMany",
	"_BrTokenValueQueryManySize",
	"_BrTokenValueSet",
	"_BrTokenValueSetMany",
	"_BrVSScanf",
	"_BrVSprintf",
	"_BrVSprintfN",
	"_BrWarning",
	"__BrAssert",
	"__BrExceptionBegin",
	"__BrExceptionEnd",
	"__BrExceptionResource",
	"__BrExceptionThrow",
	"__BrExceptionValueFetch",
	"__BrUAssert",
	"__EPI",
	"__M_br_object_container_addFront",
	"__M_br_object_container_count",
	"__M_br_object_container_find",
	"__M_br_object_container_findMany",
	"__M_br_object_container_remove",
	"__M_br_object_container_removeFront",
	"__M_br_object_container_tokensMatch",
	"__M_br_object_container_tokensMatchBegin",
	"__M_br_object_container_tokensMatchEnd",
	"__M_br_object_query",
	"__M_br_object_queryAll",
	"__M_br_object_queryAllSize",
	"__M_br_object_queryBuffer",
	"__M_br_object_queryMany",
	"__M_br_object_queryManySize",
	"__PRO",
};

/*
 * Table of ordinals
 */
static br_uint_16 nameOrdinals_BRCORE1[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	48,
	49,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	65,
	66,
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	75,
	76,
	77,
	78,
	79,
	80,
	81,
	82,
	83,
	84,
	85,
	86,
	87,
	88,
	89,
	90,
	91,
	92,
	93,
	94,
	95,
	96,
	97,
	98,
	99,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121,
	122,
	123,
	124,
	125,
	126,
	127,
	128,
	129,
	130,
	131,
	132,
	133,
	134,
	135,
	136,
	137,
	138,
	139,
	140,
	141,
	142,
	143,
	144,
	145,
	146,
	147,
	148,
	149,
	150,
	151,
	152,
	153,
	154,
	155,
	156,
	157,
	158,
	159,
	160,
	161,
	162,
	163,
	164,
	165,
	166,
	167,
	168,
	169,
	170,
	171,
	172,
	173,
	174,
	175,
	176,
	177,
	178,
	179,
	180,
	181,
	182,
	183,
};

/*
 * Table of ordinals
 */
static void * functionPointers_BRCORE1[] = {
	BrAbort                                                         , /* 0 */
	BrAddHead                                                       , /* 1 */
	BrAddTail                                                       , /* 2 */
	BrDevAdd                                                        , /* 3 */
	BrDevAddStatic                                                  , /* 4 */
	BrDevBegin                                                      , /* 5 */
	BrDevBeginVar                                                   , /* 6 */
	BrDevCheckAdd                                                   , /* 7 */
	BrDevContainedCount                                             , /* 8 */
	BrDevContainedFind                                              , /* 9 */
	BrDevContainedFindMany                                          , /* 10 */
	BrDevCount                                                      , /* 11 */
	BrDevFind                                                       , /* 12 */
	BrDevFindMany                                                   , /* 13 */
	BrDevLastBeginQuery                                             , /* 14 */
	BrDevLastBeginSet                                               , /* 15 */
	BrDevRemove                                                     , /* 16 */
	BrFailure                                                       , /* 17 */
	BrFatal                                                         , /* 18 */
	BrFileAdvance                                                   , /* 19 */
	BrFileAttributes                                                , /* 20 */
	BrFileClose                                                     , /* 21 */
	BrFileEof                                                       , /* 22 */
	BrFileGetChar                                                   , /* 23 */
	BrFileGetLine                                                   , /* 24 */
	BrFileOpenRead                                                  , /* 25 */
	BrFileOpenWrite                                                 , /* 26 */
	BrFilePrintf                                                    , /* 27 */
	BrFilePutChar                                                   , /* 28 */
	BrFilePutLine                                                   , /* 29 */
	BrFileRead                                                      , /* 30 */
	BrFileWrite                                                     , /* 31 */
	BrGetEnv                                                        , /* 32 */
	BrImageAdd                                                      , /* 33 */
	BrImageDereference                                              , /* 34 */
	BrImageLookupName                                               , /* 35 */
	BrImageLookupOrdinal                                            , /* 36 */
	BrImageReference                                                , /* 37 */
	BrImageRemove                                                   , /* 38 */
	BrInsert                                                        , /* 39 */
	BrIsAlpha                                                       , /* 40 */
	BrIsDigit                                                       , /* 41 */
	BrIsPrint                                                       , /* 42 */
	BrIsSpace                                                       , /* 43 */
	BrLexerAllocate                                                 , /* 44 */
	BrLexerCommentSet                                               , /* 45 */
	BrLexerDumpSet                                                  , /* 46 */
	BrLexerErrorSet                                                 , /* 47 */
	BrLexerFree                                                     , /* 48 */
	BrLexerPop                                                      , /* 49 */
	BrLexerPosition                                                 , /* 50 */
	BrLexerPushFile                                                 , /* 51 */
	BrLexerPushString                                               , /* 52 */
	BrLexerTokenError                                               , /* 53 */
	BrLogPrintf                                                     , /* 54 */
	BrMemAllocate                                                   , /* 55 */
	BrMemCalloc                                                     , /* 56 */
	BrMemCmp                                                        , /* 57 */
	BrMemCpy                                                        , /* 58 */
	BrMemFree                                                       , /* 59 */
	BrMemInquire                                                    , /* 60 */
	BrMemSet                                                        , /* 61 */
	BrMemStrDup                                                     , /* 62 */
	BrNamePatternMatch                                              , /* 63 */
	BrNewList                                                       , /* 64 */
	BrObjectContainerFree                                           , /* 65 */
	BrObjectListAllocate                                            , /* 66 */
	BrParseFixed                                                    , /* 67 */
	BrParseFloat                                                    , /* 68 */
	BrParseInteger                                                  , /* 69 */
	BrParseMatrixFixed                                              , /* 70 */
	BrParseMatrixFloat                                              , /* 71 */
	BrParseVectorFixed                                              , /* 72 */
	BrParseVectorFloat                                              , /* 73 */
	BrPrimitiveLibraryFind                                          , /* 74 */
	BrQsort                                                         , /* 75 */
	BrRegistryAdd                                                   , /* 76 */
	BrRegistryAddMany                                               , /* 77 */
	BrRegistryAddStatic                                             , /* 78 */
	BrRegistryClear                                                 , /* 79 */
	BrRegistryCount                                                 , /* 80 */
	BrRegistryEnum                                                  , /* 81 */
	BrRegistryFind                                                  , /* 82 */
	BrRegistryFindMany                                              , /* 83 */
	BrRegistryNew                                                   , /* 84 */
	BrRegistryNewStatic                                             , /* 85 */
	BrRegistryRemove                                                , /* 86 */
	BrRegistryRemoveMany                                            , /* 87 */
	BrRegistryRemoveStatic                                          , /* 88 */
	BrRemHead                                                       , /* 89 */
	BrRemTail                                                       , /* 90 */
	BrRemove                                                        , /* 91 */
	BrRendererFacilityFind                                          , /* 92 */
	BrResAdd                                                        , /* 93 */
	BrResAllocate                                                   , /* 94 */
	BrResCheck                                                      , /* 95 */
	BrResChildEnum                                                  , /* 96 */
	BrResClass                                                      , /* 97 */
	BrResDump                                                       , /* 98 */
	BrResFree                                                       , /* 99 */
	BrResFreeNoCallback                                             , /* 100 */
	BrResIsChild                                                    , /* 101 */
	BrResRemove                                                     , /* 102 */
	BrResSize                                                       , /* 103 */
	BrResSizeTotal                                                  , /* 104 */
	BrResStrDup                                                     , /* 105 */
	BrSScanf                                                        , /* 106 */
	BrScratchAllocate                                               , /* 107 */
	BrScratchFlush                                                  , /* 108 */
	BrScratchFree                                                   , /* 109 */
	BrScratchInquire                                                , /* 110 */
	BrScratchString                                                 , /* 111 */
	BrScratchStringSize                                             , /* 112 */
	BrSimpleAddHead                                                 , /* 113 */
	BrSimpleInsert                                                  , /* 114 */
	BrSimpleNewList                                                 , /* 115 */
	BrSimpleRemHead                                                 , /* 116 */
	BrSimpleRemove                                                  , /* 117 */
	BrSprintf                                                       , /* 118 */
	BrSprintfN                                                      , /* 119 */
	BrStrCat                                                        , /* 120 */
	BrStrCmp                                                        , /* 121 */
	BrStrCpy                                                        , /* 122 */
	BrStrICmp                                                       , /* 123 */
	BrStrLen                                                        , /* 124 */
	BrStrNCmp                                                       , /* 125 */
	BrStrNCpy                                                       , /* 126 */
	BrStrNICmp                                                      , /* 127 */
	BrStrRChr                                                       , /* 128 */
	BrStrToD                                                        , /* 129 */
	BrStrToF                                                        , /* 130 */
	BrStrToL                                                        , /* 131 */
	BrStrToUL                                                       , /* 132 */
	BrStringToTokenValue                                            , /* 133 */
	BrSwap16                                                        , /* 134 */
	BrSwap32                                                        , /* 135 */
	BrSwapBlock                                                     , /* 136 */
	BrSwapFloat                                                     , /* 137 */
	BrTVTemplateAllocate                                            , /* 138 */
	BrTVTemplateFree                                                , /* 139 */
	BrTokenCount                                                    , /* 140 */
	BrTokenCreate                                                   , /* 141 */
	BrTokenFind                                                     , /* 142 */
	BrTokenFindMany                                                 , /* 143 */
	BrTokenIdentifier                                               , /* 144 */
	BrTokenType                                                     , /* 145 */
	BrTokenValueCompare                                             , /* 146 */
	BrTokenValueComparePartial                                      , /* 147 */
	BrTokenValueDump                                                , /* 148 */
	BrTokenValueQuery                                               , /* 149 */
	BrTokenValueQueryAll                                            , /* 150 */
	BrTokenValueQueryAllSize                                        , /* 151 */
	BrTokenValueQueryMany                                           , /* 152 */
	BrTokenValueQueryManySize                                       , /* 153 */
	BrTokenValueSet                                                 , /* 154 */
	BrTokenValueSetMany                                             , /* 155 */
	BrVSScanf                                                       , /* 156 */
	BrVSprintf                                                      , /* 157 */
	BrVSprintfN                                                     , /* 158 */
	BrWarning                                                       , /* 159 */
	_BrAssert                                                       , /* 160 */
	_BrExceptionBegin                                               , /* 161 */
	_BrExceptionEnd                                                 , /* 162 */
	_BrExceptionResource                                            , /* 163 */
	_BrExceptionThrow                                               , /* 164 */
	_BrExceptionValueFetch                                          , /* 165 */
	_BrUAssert                                                      , /* 166 */
	_EPI                                                            , /* 167 */
	_M_br_object_container_addFront                                 , /* 168 */
	_M_br_object_container_count                                    , /* 169 */
	_M_br_object_container_find                                     , /* 170 */
	_M_br_object_container_findMany                                 , /* 171 */
	_M_br_object_container_remove                                   , /* 172 */
	_M_br_object_container_removeFront                              , /* 173 */
	_M_br_object_container_tokensMatch                              , /* 174 */
	_M_br_object_container_tokensMatchBegin                         , /* 175 */
	_M_br_object_container_tokensMatchEnd                           , /* 176 */
	_M_br_object_query                                              , /* 177 */
	_M_br_object_queryAll                                           , /* 178 */
	_M_br_object_queryAllSize                                       , /* 179 */
	_M_br_object_queryBuffer                                        , /* 180 */
	_M_br_object_queryMany                                          , /* 181 */
	_M_br_object_queryManySize                                      , /* 182 */
	_PRO                                                            , /* 183 */
};

static br_image Image_BRCORE1 = {
	{0},
	"BRCORE1",						/* Identifier				*/
	BR_IMG_RESIDENT,				/* Image type				*/
	0,								/* Reference count			*/

	1,									/* Ordinal base				*/
	BR_ASIZE(functionPointers_BRCORE1),	/* Number of functions		*/
	functionPointers_BRCORE1,				/* Table of functions		*/

	BR_ASIZE(namePointers_BRCORE1),	/* Number of names			*/
	namePointers_BRCORE1,				/* Table of names 			*/
	nameOrdinals_BRCORE1,				/* Table of name ordinals	*/
	
	0,								/* Number of imports		*/
	NULL,							/* Table of imports			*/

	0,								/* Number of sections		*/
	NULL,							/* Table of sections		*/

	NULL,							/* Type pointer				*/
};

