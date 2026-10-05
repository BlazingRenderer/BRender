/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fw_p.h 1.2 1998/11/12 13:19:14 johng Exp $
 * $Locker: $
 *
 * Public function prototypes for BRender framework
    Last change:  MIP  29 Nov 96    5:50 pm
 */
#ifndef _FW_P_H_
#define _FW_P_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Global setup
 */

/**
 * \brief Initialise BRender. This function must be called before most BRender functions are used.
 *
 * Installs any missing handlers (diagnostic handler, memory allocator, filing
 * system) to appropriate platform defaults.
 *
 *
 * \pre Static initialisation has completed (main() has been entered). The
 *      BRender library has not yet been initialised, or has been terminated.
 *
 * \post Initialises the filing system, resource registry, and resource classes.
 * Creates various defaults (models, materials, etc.).
 *
 * \remark Do not call BrBegin() again without a preceding BrEnd().
 *
 * \return \c BRE_OK on success, or \c BRE_ALLREADY_ACTIVE if already initialised.
 *
 * \sa BrEnd()
 */
br_error BR_PUBLIC_ENTRY BrBegin(void);

/**
 * \brief End BRender, freeing its internal resources and memory.
 *
 * Releases all memory allocated in resource classes. This is effectively all
 * memory allocated by BRender for its own use and all memory allocated in the
 * various BRender allocation functions, such as BrModelAllocate().
 *
 * \pre The BRender library has been initialised and has not already been
 *      terminated.
 *
 * \remark Your application should have gracefully released all dependence upon
 *         BRender data and functions before calling BrEnd().
 *
 * \return \c BRE_OK on success, or \c BRE_NOT_ACTIVE if BRender is not
 *         currently active.
 *
 * \sa BrBegin()
 */
br_error BR_PUBLIC_ENTRY BrEnd(void);

/*
 * Framework Setup
 */
br_error BR_PUBLIC_ENTRY BrFwBegin(void);
br_error BR_PUBLIC_ENTRY BrFwEnd(void);

/*
 * Resource class handling
 */
/**
 * \brief Create a new resource class.
 *
 * \param pixelmap A non-NULL pointer to a resource class. The res_class member of rclass must be
 *                 set to a valid, unused class ID between BR_MEMORY_APPLICATION+1 and
 *                 BR_MEMORY_MAX-1 (inclusive).
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \return Returns a pointer to the resource class or NULL if unsuccessful.
 *
 * \remark Remember, that rclass must point to a resource class structure that will remain valid
 *         until the class is removed.
 */
br_resource_class *BR_PUBLIC_ENTRY     BrResClassAdd(br_resource_class *pixelmap);
/**
 * \brief Remove a resource class from use.
 *
 * \param pixelmap A non-NULL pointer to a resource class (that has been previously created using
 *                 BrResClassAdd()).
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \return Returns a pointer to the removed resource class.
 */
br_resource_class *BR_PUBLIC_ENTRY     BrResClassRemove(br_resource_class *pixelmap);
/**
 * \brief Find a resource class in the registry by name.
 *
 * A call-back function can be set up to be called if the search is unsuccessful. The search pattern
 * can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return Returns a pointer to the resource class if found, otherwise NULL. If a call-back exists
 *         and is called, the call-back's return value is returned.
 */
br_resource_class *BR_PUBLIC_ENTRY     BrResClassFind(const char *pattern);
/**
 * \brief An application defined call-back function used when BrResClassFind() or
 *        BrResClassFindMany() fail.
 *
 * \param name The search pattern supplied to BrResClassFind() or BrResClassFindMany() that did not
 *             match any resource class.
 *
 * \pre BRender has completed initialisation. No resource class has an identifier that successfully
 *      matches the search pattern.
 *
 * \post Application defined.
 *
 * \return Either return an existing resource class that is deemed appropriate for the search
 *         pattern, or NULL if there isn't one. This value will be returned by BrResClassFind() or
 *         BrResClassFindMany().
 *
 * \remark This could either be used to supply a default resource or to create a resource class. If
 *         some resource classes were created on demand, then this function could search another
 *         list of available resource classes (but not yet created) and see if the pattern matched
 *         any of them, if it did, one of them could be registered and returned.Note that there is
 *         no way to supply more than one resource class.
 *
 * \sa BrResClassFind(), BrResClassFindMany(), BrResClassFindHook().
 */
typedef br_resource_class *BR_CALLBACK br_resclass_find_cbfn(const char *name);
/**
 * \brief Set up a resource class find call-back.
 *
 * If BrResClassFind() is unsuccessful and a call-back has been set up, the call-back is passed the
 * search pattern as its only argument. The call-back should then return a pointer to a substitute
 * or default resource class.
 *
 * \param hook A pointer to a call-back function.
 *
 * \return Returns a pointer to the old call-back function.
 *
 * \par Example
 * \code{.c}
 * br_resource_class BR_CALLBACK * example_callback(char* pattern)
 * { br_resource_class default;
 * ...
 *     return(&default);
 * }
 * { br_resource_class *rc;
 * ...
 *     BrResClassFindHook(&example_callback);
 *     rc = BrResClassFind("non_existant_class");
 * }
 * \endcode
 */
br_resclass_find_cbfn *BR_PUBLIC_ENTRY BrResClassFindHook(br_resclass_find_cbfn *hook);
/**
 * \brief Create several new resource classes.
 *
 * \param items A non-NULL pointer to a series of pointers to resource classes. The res_class member
 *              of each resource class structure pointed to by items must be set to a valid, unused
 *              class ID between BR_MEMORY_APPLICATION+1 and BR_MEMORY_MAX-1 (inclusive).
 * \param n     Number of new resource classes to create.
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \return Returns the number of resource classes successfully created.
 *
 * \remark Remember, that each resource class structures must remain valid until it is removed.
 */
br_uint_32 BR_PUBLIC_ENTRY             BrResClassAddMany(br_resource_class **items, int n);
/**
 * \brief Remove a number of resource classes from use.
 *
 * \param items A non-NULL pointer to an array of pointers to resource classes (all having
 *              previously been created using BrResClassAdd()).
 * \param n     Number of resource classes to remove from use.
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \return Returns the number of resource classes successfully removed from use.
 */
br_uint_32 BR_PUBLIC_ENTRY             BrResClassRemoveMany(br_resource_class **items, int n);
/**
 * \brief Find a number of resource classes in the registry by name.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 * \param items   A pointer to an array of pointers to resource classes.
 * \param max     The maximum number of resource classes to find.
 *
 * \return Returns the number of resource classes found. The pointer array is filled with pointers
 *         to the resource classes.
 */
br_uint_32 BR_PUBLIC_ENTRY             BrResClassFindMany(const char *pattern, br_resource_class **items, int max);
/**
 * \brief Count the number of resource classes in the registry whose names match a given search
 *        pattern.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return Returns the number of resource classes matching the search string.
 */
br_uint_32 BR_PUBLIC_ENTRY             BrResClassCount(const char *pattern);

/**
 * \brief An application defined call-back function accepting a resource class and an application
 *        supplied argument (as supplied to BrResClassEnum()).
 *
 * \param item One of the resource classes selected by BrResClassEnum().
 * \param arg  The argument supplied to BrResClassEnum().
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. Avoid adding or removing resource classes within this function.
 *
 * \return Any non-zero value will terminate the enumeration and be returned by BrResClassEnum().
 *         Return zero to continue the enumeration.
 *
 * \sa BrResClassEnum(), BrResChildEnum().
 */
typedef br_uint_32 BR_CALLBACK br_resclass_enum_cbfn(br_resource_class *item, void *arg);

/**
 * \brief Call a call-back function for every resource class matching a given search pattern.
 *
 * The call-back is passed a pointer to each matching resource class, and its second argument is an
 * optional pointer supplied by the user. The search pattern can include the standard wild cards '*'
 * and '?'. The call-back itself returns a br_uint_32 value. The enumeration will halt at any stage
 * if the return value is non-zero.
 *
 * \param pattern  Search pattern.
 * \param callback A pointer to a call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \return Returns the first non-zero call-back return value, of zero if all resource classes are
 *         enumerated.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 BR_CALLBACK example_callback(br_resource_class* item,
 *     void* arg)
 * { br_uint_32 value;
 * ...
 *     return(value);
 * }
 * { br_uint_32 ev;
 * ...
 *     ev = BrResClassEnum("pattern",&example_callback,NULL);
 * }
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY BrResClassEnum(const char *pattern, br_resclass_enum_cbfn *callback, void *arg);

const char *BR_RESIDENT_ENTRY BrResClassIdentifier(br_uint_8 res_class);

/*
 * Block pool allocator
 */
/**
 * \brief Create a new block pool.
 *
 * \param block_size The size of each block, in bytes. Minimum value is 1, though note that this
 *                   size will be rounded up to the next multiple of a suitable alignment factor,
 *                   typically 8 (BR_POOL_ALIGN+1). Pools are not intended for simple data
 *                   structures.
 * \param chunk_size The number of blocks to allocate each time the pool becomes full. The value
 *                   should be between 1 and say, 10% of the maximum number of blocks allocated at
 *                   any one time. Naturally, there is a compromise between saving processing
 *                   overhead for each memory allocation (only when the pool grows), and saving
 *                   memory overhead in terms of average unused blocks.
 * \param mem_type   Memory type. More than one pool may be created for a particular memory class.
 *
 * \return Returns a pointer to the new pool, or NULL if it could not be created.
 */
br_pool *BR_PUBLIC_ENTRY BrPoolAllocate(int block_size, int chunk_size, br_uint_8 mem_type);
/**
 * \brief Deallocate an entire pool, thereby losing any blocks it may contain.
 *
 * \param pool A pointer to the pool to be deallocated.
 */
void BR_PUBLIC_ENTRY     BrPoolFree(br_pool *pool);

/**
 * \brief Allocate a block from a pool.
 *
 * If the pool is full, it will expand as necessary.
 *
 * \param pool A pointer to the relevant pool.
 *
 * \return Returns a pointer to the allocated block, or NULL if unsuccessful.
 */
void *BR_PUBLIC_ENTRY BrPoolBlockAllocate(struct br_pool *pool);
/**
 * \brief Deallocate a block from a pool.
 *
 * \param pool  A pointer to the relevant pool.
 * \param block A pointer to the block to deallocate.
 */
void BR_PUBLIC_ENTRY  BrPoolBlockFree(struct br_pool *pool, void *block);

/**
 * \brief Mark all blocks in a pool as unused.
 *
 * \param pool A pointer to the pool to be emptied.
 */
void BR_PUBLIC_ENTRY BrPoolEmpty(struct br_pool *pool);

/*
 * Byte swapping
 */
br_uint_32 BR_RESIDENT_ENTRY BrSwap32(br_uint_32 l);
br_uint_16 BR_RESIDENT_ENTRY BrSwap16(br_uint_16 s);
br_float BR_RESIDENT_ENTRY   BrSwapFloat(br_float f);
void *BR_RESIDENT_ENTRY      BrSwapBlock(void *block, int count, int size);

/*
 * Misc. support
 */
typedef int BR_CALLBACK br_qsort_cbfn(const void *, const void *);
void BR_RESIDENT_ENTRY  BrQsort(void *basep, unsigned int nelems, unsigned int size, br_qsort_cbfn *comp);

typedef int BR_CALLBACK br_bsearch_cbfn(const void *, const void *);
void *BR_RESIDENT_ENTRY BrBSearch(const void *key, const void *base, unsigned int nmemb, unsigned int size, br_bsearch_cbfn *comp);

/*
 * Diagnostic generation
 */
void BR_RESIDENT_ENTRY BrFailure(const char *s, ...);
void BR_RESIDENT_ENTRY BrWarning(const char *s, ...);
void BR_RESIDENT_ENTRY BrFatal(const char *name, int line, char *s, ...);

/*
 * Debug Break
 */
void BR_PUBLIC_ENTRY BrDebugBreak(void);

/*
 * Set new handlers
 */
/**
 * \brief Install a new diagnostic handler.
 *
 * \param newdh A pointer to an instance of a br_diaghandler structure (should really be static). If
 *              NULL, the default diagnostic handler will be used (uses the standard C I/O library).
 *
 * \return Returns a pointer to the old diagnostic handler (possibly NULL). This may be used to pass
 *         diagnostics on, if desired.
 *
 * \remark The diagnostic handler may be specified at any suitable time. BrBegin() will specify a
 *         default diagnostic handler if no handler is currently defined. Note that diagnostics
 *         generated without a diagnostic handler present will only be caught in the debug build. If
 *         not BrBegin() This should be the first BRender function called. You should cater for the
 *         circumstance arising, if in the process of handling diagnostics, your diagnostic handler
 *         may call functions that may themselves generate diagnostics.
 */
br_diaghandler *BR_PUBLIC_ENTRY BrDiagHandlerSet(br_diaghandler *newdh);
/**
 * \brief Install a new file system.
 *
 * \param newfs A pointer to an instance of a br_filesystem structure.
 *
 * \return Returns a pointer to the old br_filesystem structure.
 */
br_filesystem *BR_PUBLIC_ENTRY  BrFilesystemSet(br_filesystem *newfs);
/**
 * \brief Install a new set of memory allocation/deallocation functions.
 *
 * \param newal A pointer to an instance of a br_allocator structure.
 *
 * \return Returns a pointer to the old br_allocator structure.
 */
br_allocator *BR_PUBLIC_ENTRY   BrAllocatorSet(br_allocator *newal);
br_loghandler *BR_PUBLIC_ENTRY  BrLogHandlerSet(br_loghandler *newlh);

/*
 * Backwards compatibility
 */
#define BrErrorHandlerSet BrDiagHandlerSet

/*
 * Generic file IO
 */
/**
 * \brief Determine capabilities of the filing system.
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Interrogates filing system.
 *
 * \return The attributes of the filing system; its capabilities as defined by a combination of the
 *         following flag values: | Flag | Attribute | | --- | --- | | BR_FS_ATTR_READABLE | Filing
 *         system can read files | | BR_FS_ATTR_WRITEABLE | Filing system can write files | |
 *         BR_FS_ATTR_HAS_TEXT | Filing system can interpret ASCII text files | |
 *         BR_FS_ATTR_HAS_BINARY | Filing system can support binary files, i.e. maintains integrity
 *         of streams of any combination of bytes (8 bit). | | BR_FS_ATTR_HAS_ADVANCE | Filing
 *         system can directly skip bytes |
 */
br_uint_32 BR_PUBLIC_ENTRY BrFileAttributes(void);

/**
 * \brief Open a file for read access.
 *
 * \param name        Name of file.
 * \param n_magics    Number of characters required for mode_test to determine file type (less than
 *                    or equal to BR_MAX_FILE_MAGICS).
 * \param mode_test   Call-back function that can be used to determine file type given the first
 *                    n_magics characters of a file. Will not be used if NULL.
 * \param mode_result If this argument is non-NULL, the file type (if it could be determined) will
 *                    be stored at the address pointed to.
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed Initialisation.
 *
 * \post Searches for a file called name, if no path is specified with the file, looks in the
 *       current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if
 *       defined). Having found the file, use mode_test (if supplied) to find out if the file is
 *       text, binary or unknown. Store the result through mode_result (if non-NULL). Obtain a
 *       handle to the file.
 *
 * \return Return a file handle, or NULL if the file could not be opened.
 *
 * \sa Consult your Standard C library documentation.
 */
void *BR_PUBLIC_ENTRY     BrFileOpenRead(const char *name, br_size_t n_magics, br_mode_test_cbfn *mode_test, int *mode_result);
/**
 * \brief Open a file for writing, over-writing any existing file of the same name.
 *
 * \param name Name to open file as.
 * \param text Mode in which to open file (BR_FS_MODE_TEXT or BR_FS_MODE_BINARY). In the default
 *             implementation of the filing system (using the standard C library), this is
 *             effectively turned into a "w" or "wb" write mode parameter to fopen().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Overwrite or create file using specified name and mode.
 *
 * \return Return a file handle or NULL if the file could not be opened.
 *
 * \remark In the default implementation, when a file is opened for writing in text mode, 'end of
 *         file' (EOF, Ctrl-Z, 0x1A) characters may have special significance, and 'line feed' (LF,
 *         Ctrl-J, 0x0A, '\n') characters may be translated to 'carriage return, line feed'
 *         combinations. Consult your Standard C library documentation.
 */
void *BR_PUBLIC_ENTRY     BrFileOpenWrite(const char *name, int text);
/**
 * \brief Close a previously opened file.
 *
 * \param f Valid file handle - as returned by BrFileOpenRead() and BrFileOpenWrite().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Close file.
 *
 * \sa Consult your Standard C library documentation.
 */
void BR_PUBLIC_ENTRY      BrFileClose(void *f);
/**
 * \brief Test a file pointer for end of file.
 *
 * \param f Valid file handle - as returned by BrFileOpenRead() and BrFileOpenWrite().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \return Returns a non-zero value after the first read operation that attempts to read past the
 *         end of the file. It returns 0 if the current position is not end of file.
 *
 * \remark This function may be affected by the 'write mode' used to write the file. If the 'write
 *         mode' is BR_FS_MODE_TEXT, the result may indicate end-of-file upon reaching an EOF
 *         character.
 *
 * \sa Consult your Standard C library documentation.
 */
int BR_PUBLIC_ENTRY       BrFileEof(void *f);
/**
 * \brief Read a character from a file.
 *
 * \param f Valid file handle - as returned by BrFileOpenRead().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post If the file position is at the end of the file, the file enters the end-of-file state,
 *       otherwise a character is read and the file position is advanced.
 *
 * \return The character read from the file is returned (as though the character had been cast as
 *         (int)(unsigned char)). If a character could not be read because the file position was at
 *         the end of the file, BR_EOF is returned.
 *
 * \remark The data read from the file may be affected by the 'write mode' used to write the file.
 *
 * \sa Consult your Standard C library documentation.
 */
int BR_PUBLIC_ENTRY       BrFileGetChar(void *f);
/**
 * \brief Write a single character to a file.
 *
 * \param c Character to write.
 * \param f Valid file handle - as returned by BrFileOpenWrite().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Write the character to the file.
 *
 * \remark The data written to the file may be affected by the current 'write mode' of the file.
 *
 * \sa Consult your Standard C library documentation.
 */
void BR_PUBLIC_ENTRY      BrFilePutChar(int c, void *f);
/**
 * \brief Read a block of data from a file.
 *
 * \param buf  Buffer to receive block.
 * \param size Size of each element in block.
 * \param n    Maximum number of elements to read.
 * \param f    Valid file handle - as returned by BrFileOpenRead().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Read up to n elements of size bytes from the file f and store them in buf.
 *
 * \return Return the number of complete elements read, which may be less than n if the end of file
 *         is encountered before all the elements could be read.
 *
 * \remark The data read from the file may be affected by the 'write mode' used to write the file.
 *
 * \sa Consult your Standard C library documentation.
 */
br_size_t BR_PUBLIC_ENTRY BrFileRead(void *buf, br_size_t size, br_size_t n, void *f);
/**
 * \brief Write a block of data to a file.
 *
 * \param buf  Buffer containing block to be written.
 * \param size Size of each element in block.
 * \param n    Maximum number of elements to write.
 * \param f    Valid file handle - as returned by BrFileOpenWrite().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed Initialisation.
 *
 * \post Write up to n elements from buf to the file f.
 *
 * \return Return the number of complete elements written, which may be less than n if an error
 *         occurs (such as running out of file space).
 *
 * \remark The data written to the file may be affected by the current 'write mode' of the file.
 */
br_size_t BR_PUBLIC_ENTRY BrFileWrite(const void *buf, br_size_t size, br_size_t n, void *f);
/**
 * \brief Read a line of text (excluding terminators) from a file.
 *
 * \param buf     Buffer to hold text read.
 * \param buf_len Length of buffer (maximum number of characters to store - including '\0').
 * \param f       Valid file handle - as returned by BrFileOpenRead().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed Initialisation.
 *
 * \post Read characters into supplied buffer until buf_len-1 characters have been read, end of line
 *       has been read, or end of file has been reached. If the last character read was '\n' it is
 *       removed from the buffer.
 *
 * \return The number of characters stored in the buffer is returned. If at the end of file upon
 *         entry, zero will be returned.
 *
 * \remark The data read from the file may be affected by the 'write mode' used to write the file.
 *
 * \sa Consult your Standard C library documentation.
 */
br_size_t BR_PUBLIC_ENTRY BrFileGetLine(char *buf, br_size_t buf_len, void *f);
/**
 * \brief Write a line of text to a file, followed by writing the new-line character ('\n').
 *
 * \param buf Pointer to zero terminated string containing line of text to be written.
 * \param f   Valid file handle - as returned by BrFileOpenWrite().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Write the string to the file. Write the new-line character to the file ('\n').
 *
 * \remark The data written to the file may be affected by the current 'write mode' of the file.
 *
 * \sa Consult your Standard C library documentation.
 */
void BR_PUBLIC_ENTRY      BrFilePutLine(const char *buf, void *f);
/**
 * \brief Advance the file pointer a number of bytes through a binary stream.
 *
 * \param count Number of bytes to advance.
 * \param f     Valid file handle - as returned by BrFileOpenRead() and BrFileOpenWrite().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Advance file position by count bytes.
 *
 * \remark This function may be affected by the 'write mode' used to write the file. If the 'write
 *         mode' is BR_FS_MODE_TEXT, the accuracy of the file pointer is not guaranteed (unless
 *         count is zero).
 *
 * \sa Consult your Standard C library documentation.
 */
void BR_PUBLIC_ENTRY      BrFileAdvance(long int count, void *f);
void *BR_PUBLIC_ENTRY     BrFileLoad(void *res, const char *name, br_size_t *size);

/**
 * \brief Write a formatted string to a file.
 *
 * \param f   Valid file handle - as returned by BrFileOpenWrite().
 * \param fmt Format string as supplied to printf().
 * \param ... Any further necessary parameters as would be required in printf().
 *
 * \pre Filing system handler dependent. BRender's default filing system requires BRender to have
 *      completed initialisation.
 *
 * \post Write the string to the file (effectively using vsprintf()).
 *
 * \return Returns the number of characters written, or a negative value if an error occurs.
 *
 * \remark The data written to the file may be affected by the current 'write mode' of the file.
 *
 * \sa Consult your Standard C library documentation.
 */
int BR_PUBLIC_ENTRY BrFilePrintf(void *f, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

br_int_32 BR_RESIDENT_ENTRY BrSprintf(char *buf, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
br_int_32 BR_RESIDENT_ENTRY BrSprintfN(char *buf, br_size_t buf_size, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(3, 4);

br_int_32 BR_RESIDENT_ENTRY BrSScanf(const char *str, const char *fmt, ...) BR_SCANF_ATTRIBUTE(2, 3);

/*
 * Data file output type (one of BR_FS_MODE_xxx)
 */
/**
 * \brief Instruct BRender to output data files in text or binary for subsequent 'Save' operations.
 *
 * \param text Write mode. Either BR_FS_MODE_TEXT or BR_FS_MODE_BINARY.
 *
 * \post Records the mode with which BRender should subsequently open files for writing and how data
 *       should be written to them. This will affect export functions such as BrModelSave().
 *
 * \return Returns the old write mode.
 *
 * \remark The purpose of this function is primarily to indicate to BRender how it should output its
 *         data files, whether in compact binary form or in vaguely human readable, ASCII form. If
 *         the application produces binary and text files then it will specify the write mode upon
 *         opening a file for writing. When opening a file for read, it is opened in the same mode
 *         as it was written.
 */
int BR_PUBLIC_ENTRY BrWriteModeSet(int text);

/*
 * Generic memory allocation
 */
/**
 * \brief Allocate memory.
 *
 * \param size Size of memory block to be allocated.
 * \param type Memory type.
 *
 * \return Returns a pointer to the allocated memory, or NULL is unsuccessful.
 */
void *BR_RESIDENT_ENTRY     BrMemAllocate(br_size_t size, br_uint_8 type);
void *BR_RESIDENT_ENTRY     BrMemReallocate(void *block, br_size_t size, br_uint_8 type);
/**
 * \brief Deallocate memory.
 *
 * \param block A pointer to the block of memory to deallocate.
 */
void BR_RESIDENT_ENTRY      BrMemFree(void *block);
/**
 * \brief Find the amount of memory available of a given type.
 *
 * \param type Memory type.
 *
 * \return Returns memory available in bytes.
 */
br_size_t BR_RESIDENT_ENTRY BrMemInquire(br_uint_8 type);
br_int_32 BR_RESIDENT_ENTRY BrMemAlign(br_uint_8 type);

#if 0
void * BR_RESIDENT_ENTRY BrMemAllocateAlign(br_size_t size, br_uint_8 type, br_int_32 align);
#endif

/**
 * \brief Duplicate a string.
 *
 * \param str A pointer to the source string.
 *
 * \return Returns a pointer to the new copy of the string.
 */
char *BR_RESIDENT_ENTRY BrMemStrDup(const char *str);
/**
 * \brief Allocate and clear memory.
 *
 * \param nelems Number of elements to allocate.
 * \param size   Size of each element.
 * \param type   Memory type.
 *
 * \return Returns a pointer to the allocated memory, or NULL is unsuccessful.
 */
void *BR_RESIDENT_ENTRY BrMemCalloc(br_size_t nelems, br_size_t size, br_uint_8 type);

/*
 * Resource allocation
 */
/**
 * \brief Allocate a new resource block of a given class.
 *
 * \param vparent   A pointer to a parental resource block, or NULL if it will have no parent.
 * \param size      Size of block in bytes.
 * \param res_class Resource class (See Memory Management).
 *
 * \return Returns a pointer to the allocated resource block (of at least size bytes).
 */
void *BR_RESIDENT_ENTRY BrResAllocate(void *vparent, br_size_t size, br_uint_8 res_class);
/**
 * \brief Free a resource block.
 *
 * \param vres A pointer to a resource block (previously allocated using BrResAllocate() or
 *             BrResStrDup()).
 *
 * \post Frees the resource block, and any dependent resource blocks it has. If any resource classes
 *       have destructors (see br_resource_class), they are invoked when appropriate.
 *
 * \sa BrResClassAdd()
 *
 * \par Example
 * \code{.c}
 * void BR_CALLBACK example_destructor(void* res, br_uint_8 res_class, br_size_t size)
 * {
 * ...
 * }
 *
 * #define EXAMPLE_CLASS (BR_MEMORY_APPLICATION + 1)
 *
 * static br_resource_class example={"My Class",EXAMPLE_CLASS,example_destructor};
 *
 * {   BrResClassAdd(&example);
 *     ...
 *     { void *ptr;
 *        ptr = BrResAllocate(NULL,1024,EXAMPLE_CLASS);
 *        ...
 *        BrResFree(ptr);
 *     }
 * }
 * \endcode
 */
void BR_RESIDENT_ENTRY  BrResFree(void *vres);
void BR_RESIDENT_ENTRY  BrResFreeNoCallback(void *vres);
/**
 * \brief Duplicate a string.
 *
 * \param vparent A pointer to a parental resource block (or NULL if independent).
 * \param str     A non-NULL pointer to the source string.
 *
 * \post Allocates a resource block (from the BR_MEMORY_STRING resource class) and copies a string
 *       into it.
 *
 * \return Returns a pointer to the allocated resource block.
 *
 * \remark This is most useful for specifying identifiers for various BRender data structures.
 */
char *BR_RESIDENT_ENTRY BrResStrDup(void *vparent, const char *str);
char *BR_RESIDENT_ENTRY BrResVSprintf(void *vparent, const char *fmt, va_list ap);
char *BR_RESIDENT_ENTRY BrResSprintf(void *vparent, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

/**
 * \brief Add a resource block as a dependant or child of another.
 *
 * \param vparent A non-NULL pointer to the parent resource block.
 * \param vres    A non-NULL pointer to the child resource block (will be removed from any current
 *                parent).
 *
 * \return Returns a pointer to the child resource block.
 *
 * \remark Once a resource block has been added as a dependant, it can be freed due to its parent
 *         being freed.
 */
void *BR_RESIDENT_ENTRY       BrResAdd(void *vparent, void *vres);
/**
 * \brief Remove a resource block from a parent.
 *
 * \param vres A non-NULL pointer to the resource block to be removed.
 *
 * \return Returns a pointer to the removed resource block.
 *
 * \remark The resource block will become independent, as though it had been allocated specifying
 *         NULL as its parent.
 */
void *BR_RESIDENT_ENTRY       BrResRemove(void *vres);
/**
 * \brief Determine the class of a given resource block.
 *
 * \param vres A non-NULL pointer to a resource block.
 *
 * \return Returns the resource class.
 */
br_uint_8 BR_RESIDENT_ENTRY   BrResClass(void *vres);
/**
 * \brief Determine the size of a given resource block.
 *
 * \param vres A non-NULL pointer to a resource block.
 *
 * \return Returns the size of the resource block in bytes (may not necessarily be the same as that
 *         specified upon allocation, but will not be less).
 */
br_size_t BR_RESIDENT_ENTRY   BrResSize(void *vres);
br_size_t BR_RESIDENT_ENTRY   BrResSizeTotal(void *vres);
/**
 * \brief An application defined call-back function accepting a resource block and an application
 *        supplied argument (as supplied to BrResChildEnum()).
 *
 * \param vres One of the resource blocks attached as a child of the resource block supplied to
 *             BrResChildEnum().
 * \param arg  The argument as supplied to BrResChildEnum().
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. If blocks are attached to the parent during the enumeration, they may
 *       not be included in the enumeration. Only the current, supplied child resource block should
 *       be detached or freed (if desired).
 *
 * \return Any non-zero value will terminate the enumeration and be returned by BrResChildEnum().
 *         Return zero to continue the enumeration.
 *
 * \sa BrResChildEnum(), BrResClassEnum().
 */
typedef br_size_t BR_CALLBACK br_resenum_cbfn(void *vres, void *arg);
/**
 * \brief Enumerate through all dependant resource blocks of a particular resource block.
 *
 * \param vres     A non-NULL pointer to a resource block.
 * \param callback A non-NULL pointer to a call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \post Invoke a call-back function for each child of a resource block. The call-back is passed a
 *       pointer to each child, and its second argument is an optional pointer (arg) supplied by the
 *       user. The call-back itself returns a br_uint_32 value. The enumeration will halt at any
 *       stage if the return value is non-zero.
 *
 * \return Returns the first non-zero call-back return value, or zero if all children are
 *         enumerated.
 *
 * \remark This function does not recurse throughout all descendants. If you require such behaviour,
 *         you'll need to implement it yourself.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 example_callback(void *vres, void *arg)
 * { br_uint_32 value;
 *   ...
 *   return(value);
 * }
 * { br_uint_32 ev;
 *   void *rblock;
 *   ...
 *   ev = BrResChildEnum(rblock,&example_callback,NULL);
 * }
 * \endcode
 */
br_size_t BR_RESIDENT_ENTRY   BrResChildEnum(void *vres, br_resenum_cbfn *callback, void *arg);
br_boolean BR_RESIDENT_ENTRY  BrResIsChild(void *vparent, void *vchild);

/*
 * Block operations (XXX should be made private)
 */
/**
 * \brief Fast fill a block of memory.
 *
 * \param dest_ptr A pointer to the block to be filled.
 * \param value    Value to fill the block with.
 * \param dwords   Size of block (number of 32-bit words).
 *
 * \sa BrBlockCopy()
 */
void BR_ASM_CALL BrBlockFill(void *dest_ptr, int value, int dwords);
/**
 * \brief Copy a block of memory.
 *
 * The source and destination blocks must not overlap.
 *
 * \param dest_ptr Destination pointer.
 * \param src_ptr  Source pointer.
 * \param dwords   Size of block to copy (number of 32-bit words).
 *
 * \sa BrBlockFill()
 */
void BR_ASM_CALL BrBlockCopy(void *dest_ptr, void *src_ptr, int dwords);

#if BR_HAS_FAR
void BR_ASM_CALL BrFarBlockCopy(void __far *dest_ptr, void *src_ptr, int dwords);
#endif

/*
 * Scratchpad buffer allocation - Currenty, only one allocation
 * may be outstanding at any time
 */
void *BR_RESIDENT_ENTRY     BrScratchAllocate(br_size_t size);
void BR_RESIDENT_ENTRY      BrScratchFree(void *scratch);
br_size_t BR_RESIDENT_ENTRY BrScratchInquire(void);
void BR_RESIDENT_ENTRY      BrScratchFlush(void);

/*
 * A seperate, fixed size scratch buffer, typically used as a scratch string workspace
 *
 * At least 512 bytes in size
 */
br_size_t BR_RESIDENT_ENTRY BrScratchStringSize(void);
char *BR_RESIDENT_ENTRY     BrScratchString(void);

/*
 * Error retrieval
 */
br_error BR_RESIDENT_ENTRY    BrGetLastError(void **valuep);
const char *BR_RESIDENT_ENTRY BrStrError(br_error error);

/*
 * Device mangement
 */
struct br_device;

typedef struct br_device *BR_EXPORT br_device_begin_fn(const char *arguments);

br_error BR_RESIDENT_ENTRY BrDevAdd(struct br_device **pdev, const char *image, const char *args);
br_error BR_RESIDENT_ENTRY BrDevCheckAdd(struct br_device **pdev, const char *image, const char *args);

br_error BR_RESIDENT_ENTRY BrDevAddStatic(struct br_device **pdev, br_device_begin_fn *begin, const char *args);
br_error BR_RESIDENT_ENTRY BrDevAddConfig(const char *config);
br_error BR_RESIDENT_ENTRY BrDevRemove(struct br_device *dev);

br_error BR_RESIDENT_ENTRY BrDevicesQuery(struct br_device **devices, br_int_32 *ndevices, br_int_32 max_devices);

br_error BR_RESIDENT_ENTRY BrDevFindMany(struct br_device **devices, br_int_32 *ndevices, br_int_32 max_devices, const char *pattern);
br_error BR_RESIDENT_ENTRY BrDevCount(br_int_32 *ndevices, const char *pattern);
br_error BR_RESIDENT_ENTRY BrDevFind(struct br_device **pdev, const char *pattern);

br_error BR_RESIDENT_ENTRY BrDevContainedFind(struct br_object **ph, br_token type, const char *pattern, br_token_value *tv);
br_error BR_RESIDENT_ENTRY BrDevContainedFindMany(struct br_object **objects, br_int_32 max_objects, br_int_32 *pnum_objects, br_token type,
                                                  const char *pattern, br_token_value *tv);
br_error BR_RESIDENT_ENTRY BrDevContainedCount(br_int_32 *pcount, br_token type, const char *pattern, br_token_value *tv);

br_error BR_PUBLIC_ENTRY     BrDevBegin(br_pixelmap **ppm, const char *setup_string);
br_error BR_PUBLIC_ENTRY     BrDevBeginVar(br_pixelmap **ppm, const char *device_name, ...);
br_error BR_PUBLIC_ENTRY     BrDevBeginTV(br_pixelmap **ppm, const char *device_name, br_token_value *tv);
br_pixelmap *BR_PUBLIC_ENTRY BrDevBeginOld(const char *setup_string);
void BR_PUBLIC_ENTRY         BrDevEndOld(void);
void BR_PUBLIC_ENTRY         BrDevPaletteSetOld(br_pixelmap *pm);
void BR_PUBLIC_ENTRY         BrDevPaletteSetEntryOld(int i, br_colour colour);

/*
 * Callback function invoked when a device is enumerated
 */
typedef br_boolean BR_CALLBACK br_device_enum_cbfn(const char *identifer, br_uint_32 version, const char *creator, const char *title,
                                                   const char *product, const char *product_version, void *args);

/*
 * Callback function invoked when an output facility is enumerated
 */
typedef struct br_outfcty_desc {
    br_int_32  width;
    br_int_32  width_min;
    br_int_32  width_max;
    br_int_32  height;
    br_int_32  height_min;
    br_int_32  height_max;
    br_uint_8  pmtype;
    br_int_32  pmbits;
    br_boolean indexed;
    br_boolean fullscreen;

    /*
     * A pointer to the output facility.  Note that this is only guaranteed
     * to remain valid within the callback routine, and is is strictly
     * for the purposes of gathering further information
     */
    struct br_output_facility *output_facility;

} br_outfcty_desc;

typedef br_boolean BR_CALLBACK br_outfcty_enum_cbfn(const char *identifier, br_outfcty_desc *desc, void *args);

struct br_device_pixelmap;
struct br_renderer;
struct br_geometry;
struct br_lexer_source;
struct br_lexer;

/*
 * Enumeration routines.
 */
br_error BR_PUBLIC_ENTRY BrDeviceEnum(br_device_enum_cbfn *cbfn, void *args);
br_error BR_PUBLIC_ENTRY BrOutputFacilityEnum(const char *name, br_outfcty_enum_cbfn *cbfn, void *args);

struct br_renderer_facility;

br_error BR_RESIDENT_ENTRY BrRendererFacilityFind(struct br_renderer_facility **prf, struct br_device_pixelmap *destination, br_token scalar_type);

br_error BR_RESIDENT_ENTRY BrRendererFacilityListFind(struct br_renderer_facility **prf, br_int_32 *num_rf, br_int_32 max_rf,
                                                      struct br_device_pixelmap *destination, br_token scalar_type);

struct br_primitive_library;

br_error BR_RESIDENT_ENTRY BrPrimitiveLibraryFind(struct br_primitive_library **ppl, struct br_device_pixelmap *destination, br_token scalar_type);

br_error BR_RESIDENT_ENTRY BrPrimitiveLibraryListFind(struct br_primitive_library **ppl, br_int_32 *num_pl, br_int_32 max_pl,
                                                      struct br_device_pixelmap *destination, br_token scalar_type);

br_error BR_RESIDENT_ENTRY BrGeometryFormatFind(struct br_geometry **pgf, struct br_renderer *renderer,
                                                struct br_renderer_facility *renderer_facility, br_token scalar_type, br_token format_type);

/*
 * lists
 */

void BR_RESIDENT_ENTRY BrNewList(br_list *list);
void BR_RESIDENT_ENTRY BrAddHead(br_list *list, br_node *node);
void BR_RESIDENT_ENTRY BrAddTail(br_list *list, br_node *node);

br_node *BR_RESIDENT_ENTRY BrRemHead(br_list *list);
br_node *BR_RESIDENT_ENTRY BrRemTail(br_list *list);

void BR_RESIDENT_ENTRY     BrInsert(br_list *list, br_node *here, br_node *node);
br_node *BR_RESIDENT_ENTRY BrRemove(br_node *node);

void BR_RESIDENT_ENTRY            BrSimpleNewList(br_simple_list *list);
void BR_RESIDENT_ENTRY            BrSimpleAddHead(br_simple_list *list, br_simple_node *node);
br_simple_node *BR_RESIDENT_ENTRY BrSimpleRemHead(br_simple_list *list);
void BR_RESIDENT_ENTRY            BrSimpleInsert(br_simple_list *list, br_simple_node *here, br_simple_node *node);
br_simple_node *BR_RESIDENT_ENTRY BrSimpleRemove(br_simple_node *node);

/*
 * Hash
 */
br_hash BR_RESIDENT_ENTRY BrHash(const void *data, size_t size);
br_hash BR_RESIDENT_ENTRY BrHashString(const char *s);

/*
 * Hash Map.
 */

/**
 * @brief Manually hash a key.
 *
 * @param   hm The hash map instance. May not be NULL.
 * @param   key A pointer to the key.
 * @return  The hash of the provided key. This will never be #BR_INVALID_HASH.
 */
br_hash BR_RESIDENT_ENTRY BrHashMapHash(const br_hashmap *hm, const void *key);

/**
 * @brief Compare two keys for equality.
 *
 * @param   hm The hash map instance. May not be NULL.
 * @param   a A pointer to the first key.
 * @param   b A pointer to the second key.
 * @return  If the keys are equal, returns BR_TRUE, or BR_FALSE if they're not.
 */
br_boolean BR_RESIDENT_ENTRY BrHashMapCompare(const br_hashmap *hm, const void *a, const void *b);

br_hashmap *BR_RESIDENT_ENTRY BrHashMapAllocate(void *vparent, br_hashmap_hash_cbfn *hash, br_hashmap_compare_cbfn *compare);
int BR_RESIDENT_ENTRY         BrHashMapClear(br_hashmap *hm);

void BR_RESIDENT_ENTRY BrHashMapSetResizePolicy(br_hashmap *hm, br_hashmap_resize_policy policy);

/**
 * @brief Configure the minimum and maximum load factors of the hash map.
 *
 * @param   hm      The hash map instance.
 * @param   min_num The numerator of the minimum load factor.
 * @param   min_den The denominator of the minimum load factor.
 * @param   max_num The numerator of the maximum load factor.
 * @param   max_den The denominator of the maximum load factor.
 * @return  On success, returns 0. If the function fails, it returns a negative error value.
 *          The function can fail under the following conditions:
 *          - `hm` is NULL.
 *          - One or both of the numerators or denominators are 0.
 *          - One of both of the numerators are greater than or equal to its
 *            respective denominator.
 *          - The minimum load factor is greater than the maximum load factor.
 */
int BR_RESIDENT_ENTRY BrHashMapSetLoadFactor(br_hashmap *hm, uint16_t min_num, uint16_t min_den, uint16_t max_num, uint16_t max_den);

/**
 * @brief Reset a hash map to its default state, releasing all memory.
 *
 * It may be used as if `BrHashMapAllocate()` has just been called.
 *
 * @param hm The hash map instance. Must not be NULL.
 */
void BR_RESIDENT_ENTRY BrHashMapReset(br_hashmap *hm);

/**
 * @brief Free a hash map and all contents.
 *
 * @param hm The hash map instance. Must not be NULL.
 *
 * @remark If attached to a parent resource, this call may be omitted.
 */
void BR_RESIDENT_ENTRY BrHashMapFree(br_hashmap *hm);

/**
 * @brief Resize a hash map so it can hold `nelem` elements.
 *
 * @param   hm The hash map instance. Must not be NULL.
 * @param   nelem The number of elements this map must be able to hold.
 *          This cannot be less than the current number of elements in the map.
 * @return
 *
 * @remark  This is *not* affected by the current resize policy.
 */
int BR_RESIDENT_ENTRY        BrHashMapResize(br_hashmap *hm, br_size_t nelem);
int BR_RESIDENT_ENTRY        BrHashMapInsert(br_hashmap *hm, const void *key, void *value);
void *BR_RESIDENT_ENTRY      BrHashMapFindByHash(const br_hashmap *hm, br_hash hash);
void *BR_RESIDENT_ENTRY      BrHashMapFind(const br_hashmap *hm, const void *key);
br_boolean BR_RESIDENT_ENTRY BrHashMapUpdate(br_hashmap *hm, const void *key, void *value);
void *BR_RESIDENT_ENTRY      BrHashMapRemove(br_hashmap *hm, const void *key);
br_size_t BR_RESIDENT_ENTRY  BrHashMapSize(const br_hashmap *hm);

int BR_RESIDENT_ENTRY BrHashMapEnumerate(const br_hashmap *hm, br_hashmap_enum_cbfn proc, void *user);

br_hash BR_RESIDENT_ENTRY    BrHashMapDefaultHash(const void *k);
br_boolean BR_RESIDENT_ENTRY BrHashMapDefaultCompare(const void *a, const void *b);

br_hash BR_RESIDENT_ENTRY    BrHashMapStringHash(const void *s);
br_boolean BR_RESIDENT_ENTRY BrHashMapStringCompare(const void *a, const void *b);

/*
 * Logging routines.
 */
br_uint_8 BR_RESIDENT_ENTRY BrLogSetLevel(br_uint_8 level);
br_uint_8 BR_RESIDENT_ENTRY BrLogGetLevel(void);

void BR_RESIDENT_ENTRY BrLogV(br_uint_8 level, const char *component, const char *fmt, va_list ap);
void BR_RESIDENT_ENTRY BrLog(br_uint_8 level, const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(3, 4);
void BR_RESIDENT_ENTRY BrLogTrace(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogDebug(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogInfo(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogWarn(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogError(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif
