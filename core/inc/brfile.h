/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brfile.h 1.2 1998/08/26 18:13:49 johng Exp $
 * $Locker: $
 *
 * Brender's interface to file IO
 */

#ifndef _BRFILE_H_
#define _BRFILE_H_

/*
 * Generic putline callback function used for dumps
 */
typedef void BR_CALLBACK br_putline_cbfn(char *str, void *arg);

/**
 * \brief An application defined call-back function determining the type of a file from a given
 *        number of characters (specified with the call-back function) from the start of the file.
 *
 * \param magics   Pointer to n_magics characters.
 * \param n_magics Number of characters pointed to.
 *
 * \post Interpret the characters and determine the mode of the file (or file type).
 *
 * \return If the file is binary return BR_FS_MODE_BINARY, if text return BR_FS_MODE_TEXT, otherwise
 *         return BR_FS_MODE_UNKNOWN.
 */
typedef int BR_CALLBACK br_mode_test_cbfn(br_uint_8 *magics, br_size_t n_magics);

/*
 * Interface to filesystem
 */
/**
 * \brief An application defined call-back function returning capabilities of the filing system.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post None.
 *
 * \return The attributes of the filing system. Its capabilities as defined by a combination of the
 *         following flag values: - BR_FS_ATTR_READABLE — Filing system can read files -
 *         BR_FS_ATTR_WRITEABLE — Filing system can write files - BR_FS_ATTR_HAS_TEXT — Filing
 *         system can interpret ASCII text files - BR_FS_ATTR_HAS_BINARY — Filing system can support
 *         binary files, i.e. maintains integrity of streams of any combination of bytes (8 bit). -
 *         BR_FS_ATTR_HAS_ADVANCE — Filing system can directly skip bytes
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef br_uint_32 BR_CALLBACK brfile_attributes_cbfn(void);
/**
 * \brief An application defined call-back function to open a file for read access.
 *
 * \param name        Name of file.
 * \param n_magics    Number of characters required for mode_test to determine file type (less than
 *                    or equal to BR_MAX_FILE_MAGICS).
 * \param mode_test   Call-back function that can be used to determine file type given the first
 *                    n_magics characters of a file. Will not be used if NULL.
 * \param mode_result If this argument is non-NULL, the file type (if it could be determined) will
 *                    be stored at the address pointed to.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Searches for a file called name, if no path is specified with the file, looks in the
 *       current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if
 *       defined). Having found the file, use mode_test (if supplied) to find out if the file is
 *       text, binary or unknown. Store the result through mode_result (if non-NULL). Obtain a
 *       handle to the file.
 *
 * \return Return a file handle or NULL if the file could not be opened.
 *
 * \remark Text mode files are primarily used for debugging but can be useful to allow hand editing
 *         of input data.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef void *BR_CALLBACK      brfile_open_read_cbfn(const char *name, br_size_t n_magics, br_mode_test_cbfn *mode_test, int *mode_result);
/**
 * \brief Open a file for writing, overwriting any existing file of the same name.
 *
 * \param name Name to open file as.
 * \param text Mode in which to open file (BR_FS_MODE_TEXT or BR_FS_MODE_BINARY).
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Overwrite or create file using specified name and mode.
 *
 * \return Return a file handle or NULL if the file could not be opened.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef void *BR_CALLBACK      brfile_open_write_cbfn(const char *name, int text);
/**
 * \brief An application defined call-back function closing a previously opened file.
 *
 * \param f Valid file handle - as returned by brfile_open_read_cbfn and brfile_open_write_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Close file.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef void BR_CALLBACK       brfile_close_cbfn(void *f);
/**
 * \brief An application defined call-back function testing a file pointer for end of file.
 *
 * \param f Valid file handle - as returned by brfile_open_read_cbfn and brfile_open_write_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post None.
 *
 * \return Returns a non-zero value after the first read operation that attempts to read past the
 *         end of the file. It returns 0 if the current position is not end of file.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef int BR_CALLBACK        brfile_eof_cbfn(void *f);
/**
 * \brief An application defined call-back function reading a character from a file.
 *
 * \param f Valid file handle - as returned by brfile_open_read_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post If the file position is at the end of the file, the file enters the end-of-file state,
 *       otherwise a character is read and the file position is advanced.
 *
 * \return The character read from the file is returned (as though the character had been cast as
 *         (int)(unsigned char)). If a character could not be read because the file position was at
 *         the end of the file, BR_EOF is returned.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef int BR_CALLBACK        brfile_getchr_cbfn(void *f);
/**
 * \brief An application defined call-back function writing a single character to file.
 *
 * \param c Character to write.
 * \param f Valid file handle - as returned by brfile_open_write_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Write the character to the file.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef void BR_CALLBACK       brfile_putchr_cbfn(int c, void *f);
/**
 * \brief An application defined call-back function reading a block from a file.
 *
 * \param buf    Buffer to receive block.
 * \param size   Size of each element in block.
 * \param nelems Maximum number of elements to read.
 * \param f      Valid file handle - as returned by brfile_open_read_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Read up to nelems elements of size bytes from the file f and store them in buf.
 *
 * \return Return the number of complete elements read, which may be less than nelems if the end of
 *         file is encountered before all the elements could be read.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef br_size_t BR_CALLBACK  brfile_read_cbfn(void *buf, br_size_t size, br_size_t nelems, void *f);
/**
 * \brief An application defined call-back function writing a block to a file.
 *
 * \param buf    Buffer containing block to be written.
 * \param size   Size of each element in block.
 * \param nelems Maximum number of elements to write.
 * \param f      Valid file handle - as returned by brfile_open_write_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Write up to nelems elements from buf to the file f.
 *
 * \return Return the number of complete elements written, which may be less than nelems if an error
 *         occurs (such as running out of file space).
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef br_size_t BR_CALLBACK  brfile_write_cbfn(const void *buf, br_size_t size, br_size_t nelems, void *f);
/**
 * \brief An application defined call-back function reading a line of text (excluding terminators)
 *        from a file.
 *
 * \param buf     Buffer to hold text read.
 * \param buf_len Length of buffer (maximum number of characters to store - including '\0').
 * \param f       Valid file handle - as returned by brfile_open_read_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Read characters into supplied buffer until buf_len-1 characters have been read, end of line
 *       has been read, or end of file has been reached. If the last character read was '\n' it is
 *       removed from the buffer.
 *
 * \return The number of characters stored in the buffer is returned. If at the end of file upon
 *         entry, zero will be returned.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef br_size_t BR_CALLBACK  brfile_getline_cbfn(char *buf, br_size_t buf_len, void *f);
/**
 * \brief An application defined call-back function writing a line of text to a file, followed by
 *        writing the new-line character ('\n').
 *
 * \param buf Pointer to zero terminated string containing line of text to be written.
 * \param f   Valid file handle - as returned by brfile_open_write_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Write the string to the file. Write the new-line character to the file ('\n').
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef void BR_CALLBACK       brfile_putline_cbfn(const char *buf, void *f);
/**
 * \brief An application defined call-back function advancing a file pointer a number of bytes
 *        through a binary stream.
 *
 * \param count Number of bytes to advance.
 * \param f     Valid file handle - as returned by brfile_open_read_cbfn and brfile_open_write_cbfn.
 *
 * \pre BRender has completed initialisation. BRender is the only direct caller of this function.
 *
 * \post Advance file position by count bytes.
 *
 * \par Example
 * \code{.c}
 * See stdfile.c for examples of filing system functions.
 * \endcode
 */
typedef void BR_CALLBACK       brfile_advance_cbfn(br_size_t count, void *f);
typedef void *BR_CALLBACK      brfile_load_cbfn(void *res, const char *name, br_size_t *size);

/**
 * \brief BRender routes all filing system calls through an instance of this structure.
 *
 * The syntax of each call-back function corresponds exactly with the standard C library calls. This
 * allows the user to tailor BRender's file system characteristics to suit any platform. See
 * BrFilesystemSet() for details of how to specify a particular filing system handler.
 */
typedef struct br_filesystem {
    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * A string constant is recommended.
     */
    const char *identifier;

    /**
     * \brief This is a pointer to the function to obtain attributes of the filing system (see
     *        brfile_attributes_cbfn).
     *
     * This is called by BrFileAttributes() (See Filing System Support, page 57).
     */
    brfile_attributes_cbfn *attributes;

    /**
     * \brief This is a pointer to the function to open a file for reading (see brfile_open_read_cbfn).
     *
     * This is called by BrFileOpenRead() (See Filing System Support, page 57).
     */
    brfile_open_read_cbfn *open_read;

    /**
     * \brief This is a pointer to the function to open a file for writing (see brfile_open_write_cbfn).
     *
     * This is called by BrFileOpenWrite() (See Filing System Support, page 57).
     */
    brfile_open_write_cbfn *open_write;

    /**
     * \brief This is a pointer to the function to close a file (see brfile_close_cbfn).
     *
     * This is called by BrFileClose() (See Filing System Support, page 57).
     */
    brfile_close_cbfn *close;

    /**
     * \brief This is a pointer to the function to check for end of file (see brfile_eof_cbfn).
     *
     * This is called by BrFileEof() (See Filing System Support, page 57).
     */
    brfile_eof_cbfn *eof;

    /**
     * \brief This is a pointer to the function to read one character (see brfile_getchr_cbfn).
     *
     * This is called by BrFileGetChar() (See Filing System Support, page 57).
     */
    brfile_getchr_cbfn *getchr;
    /**
     * \brief This is a pointer to the function to write one character (see brfile_putchr_cbfn).
     *
     * This is called by BrFilePutChar() (See Filing System Support, page 57).
     */
    brfile_putchr_cbfn *putchr;

    /**
     * \brief This is a pointer to the function to read a block (see brfile_read_cbfn).
     *
     * This is called by BrFileRead() (See Filing System Support, page 57).
     */
    brfile_read_cbfn  *read;
    /**
     * \brief This is a pointer to the function to write a block (see brfile_write_cbfn).
     *
     * This is called by BrFileWrite() (See Filing System Support, page 57).
     */
    brfile_write_cbfn *write;

    /**
     * \brief This is a pointer to the function to read a line of text (see brfile_getline_cbfn).
     *
     * This is called by BrFileGetLine() (See Filing System Support, page 57).
     */
    brfile_getline_cbfn *getline;
    /**
     * \brief This is a pointer to the function to write a line of text (see brfile_putline_cbfn).
     *
     * This is called by BrFilePutLine() (See Filing System Support, page 57).
     */
    brfile_putline_cbfn *putline;

    /**
     * \brief This is a pointer to the function to advance through a stream (see brfile_advance_cbfn).
     *
     * This is called by BrFileAdvance() (See Filing System Support, page 57).
     */
    brfile_advance_cbfn *advance;

    /*
     * Load an entire file into memory.
     */
    brfile_load_cbfn *load;

} br_filesystem;

/*
 * Bitmask returned by fs->attributes
 */
enum br_filesystem_attributes {
    BR_FS_ATTR_READABLE    = 0x0001,
    BR_FS_ATTR_WRITEABLE   = 0x0002,
    BR_FS_ATTR_HAS_TEXT    = 0x0004,
    BR_FS_ATTR_HAS_BINARY  = 0x0008,
    BR_FS_ATTR_HAS_ADVANCE = 0x0010
};

/*
 * Possible values returner by open_read identify callback
 */
enum br_filesystem_identify {
    BR_FS_MODE_BINARY,
    BR_FS_MODE_TEXT,
    BR_FS_MODE_UNKNOWN
};

/*
 * Maximum number of magic bytes that can be requested on open_read
 */
#define BR_MAX_FILE_MAGICS 16

/*
 * Returned by filesys->getchr at end of file
 */
#define BR_EOF (-1)

#endif
