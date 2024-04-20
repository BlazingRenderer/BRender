/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fwsetup.c 1.2 1998/07/20 21:52:55 jon Exp $
 * $Locker: $
 *
 */
#include "fw.h"
#include "brassert.h"
#include "datafile.h"
#include "syscn_p.h"
#include "brmath.h"

BR_RCS_ID("$Id: fwsetup.c 1.2 1998/07/20 21:52:55 jon Exp $")

static br_image Image_BRCORE1;

/*
 * Stubs left over from dealing with Watcom's
 * magic symbols.
 */
static void _PRO()
{
}
static void _EPI()
{
}

/*
 * Global renderer state
 */
br_framework_state BR_ASM_DATA fw;

/*
 * Various bits of default data
 */
extern br_diaghandler *BR_ASM_DATA _BrDefaultDiagHandler;
extern br_filesystem *BR_ASM_DATA  _BrDefaultFilesystem;
extern br_allocator *BR_ASM_DATA   _BrDefaultAllocator;
extern br_loghandler *BR_ASM_DATA  _BrDefaultLogHandler;
extern br_file_primitives          _BrFilePrimsNull;

/*
 * System resource classes
 */
static br_resource_class resourceClasses[] = {
  /*
  * These two classes have to be initialised by hand
  */
    {0, "REGISTRY",          BR_MEMORY_REGISTRY,          NULL,          0 },
    {0, "ANCHOR",            BR_MEMORY_ANCHOR,            NULL,          0 },

    {0, "RESOURCE_CLASS",    BR_MEMORY_RESOURCE_CLASS,    NULL,          0 },
    {0, "SCRATCH,",          BR_MEMORY_SCRATCH,           NULL,          16},
    {0, "STRING",            BR_MEMORY_STRING,            NULL,          1 },
    {0, "FILE",              BR_MEMORY_FILE,              _BrFileFree,   0 },
    {0, "POOL",              BR_MEMORY_POOL,              NULL,          16},
    {0, "EXCEPTION_HANDLER", BR_MEMORY_EXCEPTION_HANDLER, NULL,          0 },
    {0, "TOKEN",             BR_MEMORY_TOKEN,             NULL,          0 },
    {0, "TOKEN_MAP",         BR_MEMORY_TOKEN_MAP,         NULL,          8 },
    {0, "DATAFILE",          BR_MEMORY_DATAFILE,          NULL,          0 },
    {0, "LEXER",             BR_MEMORY_LEXER,             NULL,          0 },
    {0, "OBJECT_LIST",       BR_MEMORY_OBJECT_LIST,       NULL,          0 },
    {0, "OBJECT_LIST_ENTRY", BR_MEMORY_OBJECT_LIST_ENTRY, NULL,          0 },
    {0, "APPLICATION",       BR_MEMORY_APPLICATION,       NULL,          0 },

    {0, "IMAGE",             BR_MEMORY_IMAGE,             _BrImageFree,  0 },
    {0, "IMAGE_ARENA",       BR_MEMORY_IMAGE_ARENA,       NULL,          0 },
    {0, "IMAGE_SECTIONS",    BR_MEMORY_IMAGE_SECTIONS,    NULL,          0 },
    {0, "IMAGE_NAMES",       BR_MEMORY_IMAGE_NAMES,       NULL,          0 },
    {0, "OBJECT",            BR_MEMORY_OBJECT,            _BrObjectFree, 16},
    {0, "OBJECT_DATA",       BR_MEMORY_OBJECT_DATA,       NULL,          16},
    {0, "DRIVER",            BR_MEMORY_DRIVER,            NULL,          0 },
    {0, "FMT_RESULTS",       BR_MEMORY_FMT_RESULTS,       NULL,          0 },
    {0, "TOKEN_VALUE",       BR_MEMORY_TOKEN_VALUE,       NULL,          0 },
    {0, "TOKEN_TEMPLATE",    BR_MEMORY_TOKEN_TEMPLATE,    NULL,          0 },
};

br_error BR_PUBLIC_ENTRY BrFwBegin(void)
{
    int i;

    if(fw.active)
        return BRE_ALLREADY_ACTIVE;

    /*
     * Set handlers to use defaults
     */
    if(fw.diag == NULL)
        fw.diag = _BrDefaultDiagHandler;

    if(fw.fsys == NULL)
        fw.fsys = _BrDefaultFilesystem;

    if(fw.mem == NULL)
        fw.mem = _BrDefaultAllocator;

    if(fw.log == NULL)
        fw.log = _BrDefaultLogHandler;

    /*
     * Set up logging level
     */
    if(fw.log_level == BR_LOG_OFF)
        fw.log_level = BR_LOG_INFO;

    /*
     * Set up initial state of file writing
     */
    fw.open_mode = BR_FS_MODE_BINARY;

    /*
     * Initialise all registries
     */
    BrRegistryNew(&fw.reg_resource_classes);

    /*
     * Fake the resource classes that are required to
     * support the registry
     */
    fw.resource_class_index[BR_MEMORY_REGISTRY] = resourceClasses + 0;

    fw.resource_class_index[BR_MEMORY_ANCHOR] = resourceClasses + 1;

    /*
     * Allocate the zero sized base resource instance for
     * the framework
     */
    fw.res = BrResAllocate(NULL, 0, BR_MEMORY_ANCHOR);

    /*
     * Register all the system resource classes
     */
    for(i = 0; i < BR_ASIZE(resourceClasses); i++)
        BrResClassAdd(resourceClasses + i);

    /*
     * Setup list of loaded images
     */
    BrNewList(&fw.images);

    /*
     * Initialise table of devices
     */
    fw.dev_slots  = BrResAllocate(fw.res, NDEV_SLOTS * sizeof(*fw.dev_slots), BR_MEMORY_DRIVER);
    fw.ndev_slots = NDEV_SLOTS;

    /*
     * Mark as active
     */
    fw.active = BR_TRUE;

    /*
     * Set up tokens
     */
    BrTokenBegin();

    /*
     *  Set up default system configuration
     */

    BrSystemConfigBegin();

    // Mark default drivers as still to be loaded.

    fw.bAlreadyLoadedDrivers = BR_FALSE;

    /*
     * Set up resident framework image
     */
    BrImageAdd(&Image_BRCORE1);

    return BRE_OK;
}

br_error BR_PUBLIC_ENTRY BrFwEnd(void)
{
    if(!fw.active)
        return BRE_NOT_ACTIVE;

    BrImageRemove(&Image_BRCORE1);

    /*
     * Free all resources ...
     */
    BrResFree(fw.res);

    /*
     * Clear out fw structure
     */
    BrMemSet(&fw, 0, sizeof(fw));

    return BRE_OK;
}

/*
 * User functions for setting new errorhandler, filesystem, or allocator
 */
br_diaghandler *BR_PUBLIC_ENTRY BrDiagHandlerSet(br_diaghandler *newdh)
{
    br_diaghandler *old = fw.diag;

    if(newdh == NULL)
        fw.diag = _BrDefaultDiagHandler;
    else
        fw.diag = newdh;

    return old;
}

br_filesystem *BR_PUBLIC_ENTRY BrFilesystemSet(br_filesystem *newfs)
{
    br_filesystem *old = fw.fsys;

    if(newfs == NULL)
        fw.fsys = _BrDefaultFilesystem;
    else
        fw.fsys = newfs;

    return old;
}

br_allocator *BR_PUBLIC_ENTRY BrAllocatorSet(br_allocator *newal)
{
    br_allocator *old = fw.mem;

    if(newal == NULL)
        fw.mem = _BrDefaultAllocator;
    else
        fw.mem = newal;

    return old;
}

br_loghandler *BR_PUBLIC_ENTRY BrLogHandlerSet(br_loghandler *newlh)
{
    br_loghandler *old = fw.log;

    if(newlh == NULL)
        fw.log = _BrDefaultLogHandler;
    else
        fw.log = newlh;

    return old;
}

// clang-format off
#define REGMETHOD(m)   {.name = "_" BR_STR(m),           .proc = (br_image_proc)m }
#define DEVMETHOD(t,m) {.name = "_" BR_CMETHOD_STR(t,m), .proc = (br_image_proc)BR_CMETHOD_REF(t, m)}
// clang-format on

static const br_image_function_info functions[] = {
    REGMETHOD(BrAbort),
    REGMETHOD(BrAddHead),
    REGMETHOD(BrAddTail),
    REGMETHOD(BrBSearch),
    REGMETHOD(BrDevAdd),
    REGMETHOD(BrDevAddStatic),
    REGMETHOD(BrDevBegin),
    REGMETHOD(BrDevBeginVar),
    REGMETHOD(BrDevCheckAdd),
    REGMETHOD(BrDevContainedCount),
    REGMETHOD(BrDevContainedFind),
    REGMETHOD(BrDevContainedFindMany),
    REGMETHOD(BrDevCount),
    REGMETHOD(BrDevFind),
    REGMETHOD(BrDevFindMany),
    REGMETHOD(BrDevLastBeginQuery),
    REGMETHOD(BrDevLastBeginSet),
    REGMETHOD(BrDevRemove),
    REGMETHOD(BrFailure),
    REGMETHOD(BrFatal),
    REGMETHOD(BrFileAdvance),
    REGMETHOD(BrFileAttributes),
    REGMETHOD(BrFileClose),
    REGMETHOD(BrFileEof),
    REGMETHOD(BrFileGetChar),
    REGMETHOD(BrFileGetLine),
    REGMETHOD(BrFileOpenRead),
    REGMETHOD(BrFileOpenWrite),
    REGMETHOD(BrFilePrintf),
    REGMETHOD(BrFilePutChar),
    REGMETHOD(BrFilePutLine),
    REGMETHOD(BrFileRead),
    REGMETHOD(BrFileWrite),
    REGMETHOD(BrFloatAtan2),
    REGMETHOD(BrFloatCeil),
    REGMETHOD(BrFloatFloor),
    REGMETHOD(BrFloatPow),
    REGMETHOD(BrFloatSqrt),
    REGMETHOD(BrGetEnv),
    REGMETHOD(BrImageAdd),
    REGMETHOD(BrImageDereference),
    REGMETHOD(BrImageLookupName),
    REGMETHOD(BrImageLookupOrdinal),
    REGMETHOD(BrImageReference),
    REGMETHOD(BrImageRemove),
    REGMETHOD(BrInsert),
    REGMETHOD(BrIsAlpha),
    REGMETHOD(BrIsDigit),
    REGMETHOD(BrIsPrint),
    REGMETHOD(BrIsSpace),
    REGMETHOD(BrLastErrorGet),
    REGMETHOD(BrLastErrorSet),
    REGMETHOD(BrLexerAllocate),
    REGMETHOD(BrLexerCommentSet),
    REGMETHOD(BrLexerDumpSet),
    REGMETHOD(BrLexerErrorSet),
    REGMETHOD(BrLexerFree),
    REGMETHOD(BrLexerPop),
    REGMETHOD(BrLexerPosition),
    REGMETHOD(BrLexerPushFile),
    REGMETHOD(BrLexerPushString),
    REGMETHOD(BrLexerTokenError),
    REGMETHOD(BrLogPrintf),
    REGMETHOD(BrMemAllocate),
    REGMETHOD(BrMemCalloc),
    REGMETHOD(BrMemCmp),
    REGMETHOD(BrMemCpy),
    REGMETHOD(BrMemFree),
    REGMETHOD(BrMemInquire),
    REGMETHOD(BrMemSet),
    REGMETHOD(BrMemStrDup),
    REGMETHOD(BrNamePatternMatch),
    REGMETHOD(BrNewList),
    REGMETHOD(BrObjectContainerFree),
    REGMETHOD(BrObjectListAllocate),
    REGMETHOD(BrParseFixed),
    REGMETHOD(BrParseFloat),
    REGMETHOD(BrParseInteger),
    REGMETHOD(BrParseMatrixFixed),
    REGMETHOD(BrParseMatrixFloat),
    REGMETHOD(BrParseVectorFixed),
    REGMETHOD(BrParseVectorFloat),
    REGMETHOD(BrPrimitiveLibraryFind),
    REGMETHOD(BrQsort),
    REGMETHOD(BrRegistryAdd),
    REGMETHOD(BrRegistryAddMany),
    REGMETHOD(BrRegistryAddStatic),
    REGMETHOD(BrRegistryClear),
    REGMETHOD(BrRegistryCount),
    REGMETHOD(BrRegistryEnum),
    REGMETHOD(BrRegistryFind),
    REGMETHOD(BrRegistryFindMany),
    REGMETHOD(BrRegistryNew),
    REGMETHOD(BrRegistryNewStatic),
    REGMETHOD(BrRegistryRemove),
    REGMETHOD(BrRegistryRemoveMany),
    REGMETHOD(BrRegistryRemoveStatic),
    REGMETHOD(BrRemHead),
    REGMETHOD(BrRemTail),
    REGMETHOD(BrRemove),
    REGMETHOD(BrRendererFacilityFind),
    REGMETHOD(BrResAdd),
    REGMETHOD(BrResAllocate),
    REGMETHOD(BrResCheck),
    REGMETHOD(BrResChildEnum),
    REGMETHOD(BrResClass),
    REGMETHOD(BrResDump),
    REGMETHOD(BrResFree),
    REGMETHOD(BrResFreeNoCallback),
    REGMETHOD(BrResIsChild),
    REGMETHOD(BrResRemove),
    REGMETHOD(BrResSize),
    REGMETHOD(BrResSizeTotal),
    REGMETHOD(BrResSprintf),
    REGMETHOD(BrResStrDup),
    REGMETHOD(BrResVSprintf),
    REGMETHOD(BrSScanf),
    REGMETHOD(BrScratchAllocate),
    REGMETHOD(BrScratchFlush),
    REGMETHOD(BrScratchFree),
    REGMETHOD(BrScratchInquire),
    REGMETHOD(BrScratchString),
    REGMETHOD(BrScratchStringSize),
    REGMETHOD(BrSimpleAddHead),
    REGMETHOD(BrSimpleInsert),
    REGMETHOD(BrSimpleNewList),
    REGMETHOD(BrSimpleRemHead),
    REGMETHOD(BrSimpleRemove),
    REGMETHOD(BrSprintf),
    REGMETHOD(BrSprintfN),
    REGMETHOD(BrStrCat),
    REGMETHOD(BrStrCmp),
    REGMETHOD(BrStrCpy),
    REGMETHOD(BrStrICmp),
    REGMETHOD(BrStrLen),
    REGMETHOD(BrStrNCmp),
    REGMETHOD(BrStrNCpy),
    REGMETHOD(BrStrNICmp),
    REGMETHOD(BrStrRChr),
    REGMETHOD(BrStrToD),
    REGMETHOD(BrStrToF),
    REGMETHOD(BrStrToL),
    REGMETHOD(BrStrToLL),
    REGMETHOD(BrStrToUL),
    REGMETHOD(BrStrToULL),
    REGMETHOD(BrStringToTokenValue),
    REGMETHOD(BrSwap16),
    REGMETHOD(BrSwap32),
    REGMETHOD(BrSwapBlock),
    REGMETHOD(BrSwapFloat),
    REGMETHOD(BrTVTemplateAllocate),
    REGMETHOD(BrTVTemplateFree),
    REGMETHOD(BrTokenCount),
    REGMETHOD(BrTokenCreate),
    REGMETHOD(BrTokenFind),
    REGMETHOD(BrTokenFindMany),
    REGMETHOD(BrTokenIdentifier),
    REGMETHOD(BrTokenSize),
    REGMETHOD(BrTokenType),
    REGMETHOD(BrTokenValueCompare),
    REGMETHOD(BrTokenValueComparePartial),
    REGMETHOD(BrTokenValueDump),
    REGMETHOD(BrTokenValueQuery),
    REGMETHOD(BrTokenValueQueryAll),
    REGMETHOD(BrTokenValueQueryAllSize),
    REGMETHOD(BrTokenValueQueryMany),
    REGMETHOD(BrTokenValueQueryManySize),
    REGMETHOD(BrTokenValueSet),
    REGMETHOD(BrTokenValueSetMany),
    REGMETHOD(BrVSScanf),
    REGMETHOD(BrVSprintf),
    REGMETHOD(BrVSprintfN),
    REGMETHOD(BrWarning),
    REGMETHOD(_BrAssert),
    REGMETHOD(_BrExceptionBegin),
    REGMETHOD(_BrExceptionEnd),
    REGMETHOD(_BrExceptionResource),
    REGMETHOD(_BrExceptionThrow),
    REGMETHOD(_BrExceptionValueFetch),
    REGMETHOD(_BrUAssert),
    REGMETHOD(_EPI),
    DEVMETHOD(br_object_container, addFront),
    DEVMETHOD(br_object_container, count),
    DEVMETHOD(br_object_container, find),
    DEVMETHOD(br_object_container, findMany),
    DEVMETHOD(br_object_container, remove),
    DEVMETHOD(br_object_container, removeFront),
    DEVMETHOD(br_object_container, tokensMatch),
    DEVMETHOD(br_object_container, tokensMatchBegin),
    DEVMETHOD(br_object_container, tokensMatchEnd),
    DEVMETHOD(br_object, query),
    DEVMETHOD(br_object, queryAll),
    DEVMETHOD(br_object, queryAllSize),
    DEVMETHOD(br_object, queryBuffer),
    DEVMETHOD(br_object, queryMany),
    DEVMETHOD(br_object, queryManySize),
    REGMETHOD(_PRO),
};

/*
 * Resident image that is used to expose resident framework
 * functions to loaded devices.
 */
static br_image Image_BRCORE1 = {
    .node         = {0},
    .identifier   = "BRCORE1",
    .type         = BR_IMG_RESIDENT,
    .ref_count    = 0,
    .n_functions  = BR_ASIZE(functions),
    .functions    = functions,
    .type_pointer = NULL,
};
