/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: datafile.c 2.12 1997/04/23 16:15:25 jon Exp $
 * $Locker: $
 *
 * Data file support routines
 */
#include "fw.h"
#include "brassert.h"
#include "brhton.h"
#include "datafile.h"
#include "shortcut.h"

BR_RCS_ID("$Id: datafile.c 2.12 1997/04/23 16:15:25 jon Exp $")

#define CHUNK_LOG 0

/*
 * maximum length of lines in text data files
 */
#define TEXTF_MAX_LINE 256

/*
 * Number of bytes per line in block text mode
 */
#define TEXT_BLOCK_LINE 32

/*
 * Debug printing while loading data files
 */
#define DATA_LOG 0

/*
 * Local version of ispsace()
 */
#define ISSPACE(c) ((c)==' ' || (c)=='\t')

/*
 * A Stack of generic data pointers used while interpreting incoming
 * files
 */
#define DFSTACK_MAX 1024

static struct {
	int type;
	void * value;
	int count;
} DatafileStack[DFSTACK_MAX];

static int DatafileStackTop = 0;

/*
 * Push a pointer to a vector of typed items onto stack
 */
void DfPush(int type, void *value, int count)
{
	ASSERT((type > DFST_NONE) && (type < DFST_MAX));
	ASSERT(value != NULL);
	ASSERT(count > 0);

	if(DatafileStackTop >= DFSTACK_MAX)
		BR_ERROR("DatafileStack Overflow");

	DatafileStack[DatafileStackTop].type = type;
	DatafileStack[DatafileStackTop].value = value;
	DatafileStack[DatafileStackTop].count = count;

	DatafileStackTop++;
}

/*
 * Pop a pointer to a vector of items and the count - check the
 * assumed type matches that popped from the stack.
 */
void *DfPop(int type, int *countp)
{
	ASSERT((type > DFST_NONE) && (type < DFST_MAX));

	if(DatafileStackTop <= 0)
		BR_ERROR("DatafileStack Underflow");

	DatafileStackTop--;

	if(type != DatafileStack[DatafileStackTop].type)
		BR_ERROR2("DatafileStack type mismatch, wanted %d, got %d",type,
										DatafileStack[DatafileStackTop].type);

	if(countp)
		*countp = DatafileStack[DatafileStackTop].count;

	return DatafileStack[DatafileStackTop].value;
}

/*
 * Get a pointer to a vector of items and the count from the top of
 * the stack - check the* assumed type matches that fetched.
 */
void *DfTop(int type, int *countp)
{
	ASSERT((type > DFST_NONE) && (type < DFST_MAX));

	if(DatafileStackTop <= 0)
		BR_ERROR("DatafileStack Underflow");

	if(type != DatafileStack[DatafileStackTop-1].type)
		BR_ERROR2("DatafileStack type mismatch, wanted %d, got %d",type,
										DatafileStack[DatafileStackTop].type);

	if(countp)
		*countp = DatafileStack[DatafileStackTop-1].count;

	return DatafileStack[DatafileStackTop-1].value;
}

/*
 * Find the type of the items on top of the stack - return DFST_NONE
 * if stack is empty
 */
int DfTopType(void)
{
	if(DatafileStackTop <= 0)
		return DFST_NONE;

	return DatafileStack[DatafileStackTop-1].type;
}

/*
 * Get next line from input and break it into <ident> <data>
 *
 * Return 0 at end of file
 */
STATIC int TextReadLine(br_datafile *df, char **ident, char **data)
{
	char *cp;

	/*
	 * Consume blank lines
	 */
	do {

		BrFileGetLine(BrScratchString(),TEXTF_MAX_LINE,df->h);

		if(BrFileEof(df->h))
			return 0;

		/*
		 * Skip white to ident
		 */
		cp = BrScratchString();
		while(ISSPACE(*cp))	cp++;

	} while(*cp == '\0');

	*ident = cp;

	/*
	 * Skip ident to white
	 */
	while(!ISSPACE(*cp) && *cp != '\0')
		cp++;

	/*
	 * Terminate ident
	 */
	*cp++ = '\0';

	/*
	 * Skip white to data
	 */
	while(ISSPACE(*cp))	cp++;
	
	*data = cp;

	if(*cp == '"') {
		/*
		 * Null terminate data at closing quote
		 */
		cp++;

		while((*cp != '"') && (*cp != '\0')) cp++;
		
	} else {
		/*
		 * Null terminate data at next white space
		 */
		while(!ISSPACE(*cp) && *cp != '\0') cp++;
	}

	/*
	 * Terminate data
	 */
	*cp = '\0';

	return 1;
}

/**
 ** reading and writing data files, both in binary and text formats
 **/
/*
 * Names of various structure elements
 */
STATIC char *member_type_names[] = {
	[FSM_INT_8]            = "int_8",
	[FSM_UINT_8]           = "uint_8",
	[FSM_INT_16]           = "int_16",
	[FSM_UINT_16]          = "uint_16",
	[FSM_INT_32]           = "int_32",
	[FSM_UINT_32]          = "uint_32",
	[FSM_FIXED]            = "fixed",
	[FSM_ANGLE]            = "angle",
	[FSM_FLOAT]            = "float",
	[FSM_DOUBLE]           = "double",
	[FSM_SCALAR]           = "scalar",
	[FSM_FRACTION]         = "fraction",
	[FSM_UFRACTION]        = "ufraction",
	[FSM_ENUM_8]           = "enum_8",
	[FSM_ENUM_16]          = "enum_16",
	[FSM_ENUM_32]          = "enum_32",
	[FSM_STRUCT]           = "struct",
	[FSM_ASCIZ]            = "asciz",
	[FSM_COLOUR]           = "colour",
	[FSM_VECTOR2]          = "vector2",
	[FSM_VECTOR3]          = "vector3",
	[FSM_VECTOR4]          = "vector4",
	[FSM_FVECTOR2]         = "fvector2",
	[FSM_FVECTOR3]         = "fvector3",
	[FSM_FVECTOR4]         = "fvector4",
	[FSM_FIXED_FRACTION]   = "fixed_fraction",
	[FSM_FIXED_UFRACTION]  = "fixed_ufraction",
	[FSM_FLOAT_FRACTION]   = "float_fraction",
	[FSM_FLOAT_UFRACTION]  = "float_ufraction",
	[FSM_FIXED_VECTOR2]    = "fixed_vector2",
	[FSM_FIXED_VECTOR3]    = "fixed_vector3",
	[FSM_FIXED_VECTOR4]    = "fixed_vector4",
	[FSM_FLOAT_VECTOR2]    = "float_vector2",
	[FSM_FLOAT_VECTOR3]    = "float_vector3",
	[FSM_FLOAT_VECTOR4]    = "float_vector4",
	[FSM_FIXED_FVECTOR2]   = "fixed_fvector2",
	[FSM_FIXED_FVECTOR3]   = "fixed_fvector3",
	[FSM_FIXED_FVECTOR4]   = "fixed_fvector4",
	[FSM_FLOAT_FVECTOR2]   = "float_fvector2",
	[FSM_FLOAT_FVECTOR3]   = "float_fvector3",
	[FSM_FLOAT_FVECTOR4]   = "float_fvector4",
	[FSM_COLOUR_ALPHA]     = "colour_alpha",
};

static br_uint_16 scalarTypeConvert(br_datafile *df, br_uint_16 t)
{
	/*
	 * Convert scalar types
	 */
	switch(df->scalar_type) {
	case BRT_FLOAT:
		switch(t) {
		case FSM_SCALAR:	return FSM_FLOAT;
		case FSM_FRACTION:	return FSM_FLOAT_FRACTION;
		case FSM_UFRACTION:	return FSM_FLOAT_UFRACTION;
		case FSM_VECTOR2:	return FSM_FLOAT_VECTOR2;
		case FSM_VECTOR3:	return FSM_FLOAT_VECTOR3;
		case FSM_VECTOR4:	return FSM_FLOAT_VECTOR4;
		case FSM_FVECTOR2:	return FSM_FLOAT_FVECTOR2;
		case FSM_FVECTOR3:	return FSM_FLOAT_FVECTOR3;
		case FSM_FVECTOR4:	return FSM_FLOAT_FVECTOR4;
		default:			return t;
		}
		break;

	case BRT_FIXED:
		switch(t) {
		case FSM_SCALAR:	return FSM_FIXED;
		case FSM_FRACTION:	return FSM_FIXED_FRACTION;
		case FSM_UFRACTION:	return FSM_FIXED_UFRACTION;
		case FSM_VECTOR2:	return FSM_FIXED_VECTOR2;
		case FSM_VECTOR3:	return FSM_FIXED_VECTOR3;
		case FSM_VECTOR4:	return FSM_FIXED_VECTOR4;
		case FSM_FVECTOR2:	return FSM_FIXED_FVECTOR2;
		case FSM_FVECTOR3:	return FSM_FIXED_FVECTOR3;
		case FSM_FVECTOR4:	return FSM_FIXED_FVECTOR4;
		default:			return t;
		}
		break;

	default:
		switch(t) {
		case FSM_SCALAR:
		case FSM_FRACTION:
		case FSM_UFRACTION:
		case FSM_VECTOR2:
		case FSM_VECTOR3:
		case FSM_VECTOR4:
		case FSM_FVECTOR2:
		case FSM_FVECTOR3:
		case FSM_FVECTOR4:
			BR_ERROR0("Incorrect scalar type");
		default:
			return t;
			break;
		}
	}
}

/*
 * Write a structure to a binary file using the given template
 *
 */
STATIC br_uint_32 DfStructWriteBinary(br_datafile *df, br_file_struct *str, void *base)
{
	unsigned int m;
	int i,n;
	unsigned char *mp;
	br_file_struct_member *sm;

	union {
		unsigned char b[8];
		float f;
	} conv;

	for(m = 0, sm = str->members ; m < str->nmembers; m++, sm++) {
		/*
		 * Get pointer to source memory
		 */
		mp = (char *)base + sm->offset;


		/*
		 * Do type specific processing
		 */
		switch(scalarTypeConvert(df, sm->type)) {

		case FSM_INT_8:
		case FSM_UINT_8:
		case FSM_ENUM_8:
			BrFilePutChar(mp[0],df->h);
			break;

		case FSM_INT_16:
		case FSM_UINT_16:
		case FSM_ENUM_16:
		case FSM_ANGLE:
			BrFilePutChar(mp[BR_HTON_16(0)],df->h);
			BrFilePutChar(mp[BR_HTON_16(1)],df->h);
			break;

		case FSM_FLOAT:
		case FSM_FLOAT_FRACTION:
		case FSM_FLOAT_UFRACTION:
		case FSM_INT_32:
		case FSM_UINT_32:
		case FSM_ENUM_32:
		case FSM_COLOUR_ALPHA:
			BrFilePutChar(mp[BR_HTON_32(0)],df->h);

		case FSM_COLOUR:
			BrFilePutChar(mp[BR_HTON_32(1)],df->h);
			BrFilePutChar(mp[BR_HTON_32(2)],df->h);
			BrFilePutChar(mp[BR_HTON_32(3)],df->h);
			break;

		case FSM_DOUBLE:
			BrFilePutChar(mp[BR_HTON_D(0)],df->h);
			BrFilePutChar(mp[BR_HTON_D(1)],df->h);
			BrFilePutChar(mp[BR_HTON_D(2)],df->h);
			BrFilePutChar(mp[BR_HTON_D(3)],df->h);
			BrFilePutChar(mp[BR_HTON_D(4)],df->h);
			BrFilePutChar(mp[BR_HTON_D(5)],df->h);
			BrFilePutChar(mp[BR_HTON_D(6)],df->h);
			BrFilePutChar(mp[BR_HTON_D(7)],df->h);
			break;

		case FSM_FIXED:
			conv.f = BrFixedToFloat(*((br_fixed_ls *)mp));
		put_four:			
			BrFilePutChar(conv.b[BR_HTON_32(0)],df->h);
			BrFilePutChar(conv.b[BR_HTON_32(1)],df->h);
			BrFilePutChar(conv.b[BR_HTON_32(2)],df->h);
			BrFilePutChar(conv.b[BR_HTON_32(3)],df->h);
			break;

		case FSM_FIXED_FRACTION:
			conv.f = BrFixedLSFToFloat(*((br_fixed_lsf *)mp));
			goto put_four;

		case FSM_FIXED_UFRACTION:
			conv.f = BrFixedLUFToFloat(*((br_fixed_luf *)mp));
			goto put_four;

		case FSM_FIXED_VECTOR2:
			n = 2;
			goto put_fixed_vector;

		case FSM_FIXED_VECTOR3:
			n = 3;
			goto put_fixed_vector;

		case FSM_FIXED_VECTOR4:
			n = 4;
		put_fixed_vector:
			/*
			 * Assumes vector2 and vector3 are reduced versions of vector4
			 */
			for(i=0;i<n;i++) {
				conv.f = BrFixedToFloat(((br_vector4_x *)mp)->v[i]);

				BrFilePutChar(conv.b[BR_HTON_32(0)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(1)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(2)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(3)],df->h);
			}
			break;

		case FSM_FIXED_FVECTOR2:
			n = 2;
			goto put_fixed_fvector;

		case FSM_FIXED_FVECTOR3:
			n = 3;
			goto put_fixed_fvector;

		case FSM_FIXED_FVECTOR4:
			n = 4;
		put_fixed_fvector:
			/*
			 * Assumes fvector2 and fvector3 are reduced versions of fvector4
			 */
			for(i=0;i<n;i++) {
				conv.f = BrFixedLSFToFloat(((br_fvector4_x *)mp)->v[i]);

				BrFilePutChar(conv.b[BR_HTON_32(0)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(1)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(2)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(3)],df->h);
			}
			break;

		case FSM_FLOAT_VECTOR2:
			n = 2;
			goto put_float_vector;

		case FSM_FLOAT_VECTOR3:
			n = 3;
			goto put_float_vector;

		case FSM_FLOAT_VECTOR4:
			n = 4;
		put_float_vector:
			/*
			 * Assumes vector2 and vector3 are reduced versions of vector4
			 */
			for(i=0;i<n;i++) {
				conv.f = ((br_vector4_f *)mp)->v[i];

				BrFilePutChar(conv.b[BR_HTON_32(0)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(1)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(2)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(3)],df->h);
			}
			break;

		case FSM_FLOAT_FVECTOR2:
			n = 2;
			goto put_float_fvector;

		case FSM_FLOAT_FVECTOR3:
			n = 3;
			goto put_float_fvector;

		case FSM_FLOAT_FVECTOR4:
			n = 4;
		put_float_fvector:
			/*
			 * Assumes fvector2 and fvector3 are reduced versions of fvector4
			 */
			for(i=0;i<n;i++) {
				conv.f = ((br_fvector4_f *)mp)->v[i];

				BrFilePutChar(conv.b[BR_HTON_32(0)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(1)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(2)],df->h);
				BrFilePutChar(conv.b[BR_HTON_32(3)],df->h);
			}
			break;

		case FSM_STRUCT:

			DfStructWriteBinary(df,sm->extra,mp);

			break;

		case FSM_ASCIZ:

			if(*((char **)mp))
				BrFileWrite(*((char **)mp),1,BrStrLen(*((char **)mp)),df->h);

			BrFilePutChar('\0',df->h);
			break;
		}
	}

	return 1;
}

/*
 * Read a structure from a binary file using the given template
 *
 */
STATIC br_uint_32 DfStructReadBinary(br_datafile *df, br_file_struct *str, void *base)
{
	char tmp_string[TEXTF_MAX_LINE];
	unsigned int m;
	int i,c,n;
	unsigned char *mp;
	br_file_struct_member *sm;

	union {
		unsigned char b[8];
		float f;
	} conv;

	for(m = 0, sm = str->members ; m < str->nmembers; m++, sm++) {
		/*
		 * Get pointer to destination memory
		 */
		mp = (char *)base + sm->offset;

		/*
		 * Do type specific processing
		 */
		switch(scalarTypeConvert(df, sm->type)) {

		case FSM_INT_8:
		case FSM_UINT_8:
		case FSM_ENUM_8:
			mp[0] = BrFileGetChar(df->h);
			break;

		case FSM_ANGLE:
		case FSM_UINT_16:
		case FSM_INT_16:
		case FSM_ENUM_16:
			mp[BR_NTOH_16(0)] = BrFileGetChar(df->h);
			mp[BR_NTOH_16(1)] = BrFileGetChar(df->h);
			break;

		case FSM_FLOAT:
		case FSM_FLOAT_FRACTION:
		case FSM_FLOAT_UFRACTION:
		case FSM_INT_32:
		case FSM_UINT_32:
		case FSM_ENUM_32:
		case FSM_COLOUR_ALPHA:
			mp[BR_NTOH_32(0)] = BrFileGetChar(df->h);

		case FSM_COLOUR:
			mp[BR_NTOH_32(1)] = BrFileGetChar(df->h);
			mp[BR_NTOH_32(2)] = BrFileGetChar(df->h);
			mp[BR_NTOH_32(3)] = BrFileGetChar(df->h);
			break;

		case FSM_FIXED:
			conv.b[BR_NTOH_F(0)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(1)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(2)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(3)] = BrFileGetChar(df->h);

			*((br_fixed_ls *)mp) = BrFloatToFixedLS(conv.f);
			break;

		case FSM_FIXED_FRACTION:
			conv.b[BR_NTOH_F(0)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(1)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(2)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(3)] = BrFileGetChar(df->h);

			*((br_fixed_lsf *)mp) = BrFloatToFixedLSF(conv.f);
			break;

		case FSM_FIXED_UFRACTION:
			conv.b[BR_NTOH_F(0)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(1)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(2)] = BrFileGetChar(df->h);
			conv.b[BR_NTOH_F(3)] = BrFileGetChar(df->h);

			*((br_fixed_luf *)mp) = BrFloatToFixedLUF(conv.f);
			break;

		case FSM_DOUBLE:
			mp[BR_NTOH_D(0)] = BrFileGetChar(df->h);
			mp[BR_NTOH_D(1)] = BrFileGetChar(df->h);
			mp[BR_NTOH_D(2)] = BrFileGetChar(df->h);
			mp[BR_NTOH_D(3)] = BrFileGetChar(df->h);
			mp[BR_NTOH_D(4)] = BrFileGetChar(df->h);
			mp[BR_NTOH_D(5)] = BrFileGetChar(df->h);
			mp[BR_NTOH_D(6)] = BrFileGetChar(df->h);
			mp[BR_NTOH_D(7)] = BrFileGetChar(df->h);
			break;

		case FSM_FIXED_VECTOR2:
			n = 2;
			goto get_fixed_vector;

		case FSM_FIXED_VECTOR3:
			n = 3;
			goto get_fixed_vector;

		case FSM_FIXED_VECTOR4:
			n = 4;
		get_fixed_vector:
			/*
			 * Assumes vector2 and vector3 are reduced versions of vector4
			 */
			for(i=0;i<n;i++) {

				conv.b[BR_NTOH_F(0)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(1)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(2)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(3)] = BrFileGetChar(df->h);

				((br_vector4_x *)mp)->v[i] = BrFloatToFixedLS(conv.f);

			}
			break;

		case FSM_FLOAT_VECTOR2:
			n = 2;
			goto get_float_vector;

		case FSM_FLOAT_VECTOR3:
			n = 3;
			goto get_float_vector;

		case FSM_FLOAT_VECTOR4:
			n = 4;
		get_float_vector:
			/*
			 * Assumes vector2 and vector3 are reduced versions of vector4
			 */
			for(i=0;i<n;i++) {

				conv.b[BR_NTOH_F(0)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(1)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(2)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(3)] = BrFileGetChar(df->h);

				((br_vector4_f *)mp)->v[i] = conv.f;

			}
			break;

		case FSM_FIXED_FVECTOR2:
			n = 2;
			goto get_fixed_fvector;

		case FSM_FIXED_FVECTOR3:
			n = 3;
			goto get_fixed_fvector;

		case FSM_FIXED_FVECTOR4:
			n = 4;
		get_fixed_fvector:
			/*
			 * Assumes fvector2 and fvector3 are reduced versions of fvector4
			 */
			for(i=0;i<n;i++) {

				conv.b[BR_NTOH_F(0)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(1)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(2)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(3)] = BrFileGetChar(df->h);

				((br_fvector4_x *)mp)->v[i] = BrFloatToFixedLSF(conv.f);

			}
			break;

		case FSM_FLOAT_FVECTOR2:
			n = 2;
			goto get_float_fvector;

		case FSM_FLOAT_FVECTOR3:
			n = 3;
			goto get_float_fvector;

		case FSM_FLOAT_FVECTOR4:
			n = 4;
		get_float_fvector:
			/*
			 * Assumes fvector2 and fvector3 are reduced versions of fvector4
			 */
			for(i=0;i<n;i++) {

				conv.b[BR_NTOH_F(0)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(1)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(2)] = BrFileGetChar(df->h);
				conv.b[BR_NTOH_F(3)] = BrFileGetChar(df->h);

				((br_fvector4_f *)mp)->v[i] = conv.f;

			}
			break;

		case FSM_STRUCT:
			DfStructReadBinary(df,sm->extra,mp);

			break;

		case FSM_ASCIZ:
			/*
			 * Read string into a temporary buffer
			 */
			for(i=0; i < TEXTF_MAX_LINE-1; i++) {
				c = BrFileGetChar(df->h);
				if(c == '\0' || c == BR_EOF)
					break;

				tmp_string[i] = c;
			}

			/*
			 * Terminate it
			 */
			tmp_string[i] = '\0';
			
			*((char **)mp) = BrResStrDup(df->res?df->res:fw.res, tmp_string);

			break;
		}
	}

	return 1;
}

/*
 * Find the size a structure would occupy when written to a binary file
 *
 * If the structure instance is NULL, then ignore variable sized memebers
 */
STATIC br_uint_32 DfStructSizeBinary(br_datafile *df, br_file_struct *str, void *base)
{
	unsigned char *mp;
	unsigned int m;
	br_file_struct_member *sm;
	br_uint_32 bytes = 0;

	for(m = 0, sm = str->members ; m < str->nmembers; m++, sm++) {

		/*
		 * Do type specific processing
		 */
		switch(scalarTypeConvert(df, sm->type)) {

		case FSM_INT_8:
		case FSM_UINT_8:
		case FSM_ENUM_8:
			bytes += 1;
			break;

		case FSM_ANGLE:
		case FSM_INT_16:
		case FSM_UINT_16:
		case FSM_ENUM_16:
			bytes += 2;
			break;

		case FSM_COLOUR:
			bytes += 3;
			break;

		case FSM_COLOUR_ALPHA:
			bytes += 4;
			break;

		case FSM_FIXED:
		case FSM_FIXED_FRACTION:
		case FSM_FIXED_UFRACTION:
		case FSM_INT_32:
		case FSM_UINT_32:
		case FSM_ENUM_32:
		case FSM_FLOAT:
		case FSM_FLOAT_FRACTION:
		case FSM_FLOAT_UFRACTION:
			bytes += 4;
			break;

		case FSM_DOUBLE:
		case FSM_FLOAT_VECTOR2:
		case FSM_FIXED_VECTOR2:
		case FSM_FLOAT_FVECTOR2:
		case FSM_FIXED_FVECTOR2:
			bytes += 8;
			break;

		case FSM_FLOAT_VECTOR3:
		case FSM_FIXED_VECTOR3:
		case FSM_FLOAT_FVECTOR3:
		case FSM_FIXED_FVECTOR3:
			bytes += 12;
			break;

		case FSM_FLOAT_VECTOR4:
		case FSM_FIXED_VECTOR4:
		case FSM_FLOAT_FVECTOR4:
		case FSM_FIXED_FVECTOR4:
			bytes += 16;
			break;

		case FSM_STRUCT:
			bytes += DfStructSizeBinary(df,sm->extra,
					base?((char *)base + sm->offset):NULL);
			break;

		case FSM_ASCIZ:
			mp = (char *)base + sm->offset;

			if(*((char **)mp))
				bytes += (br_uint_32)BrStrLen(*((char **)mp));

			bytes += 1;
			break;
		}
	}

	return bytes;
}

/*
 * Lookup enums in tables
 */
STATIC int EnumFromString(br_file_enum *e, char *str)
{
	unsigned int m;

	for(m=0; m < e->nmembers; m++) 
		if((e->members[m].name[0] == *str) && !BrStrCmp(e->members[m].name,str))
			return e->members[m].value;

	BR_ERROR1("Unknown enum string: %s",str);

	return 0;
}

STATIC char *EnumToString(br_file_enum *e, int num)
{
	unsigned int m;

	for(m=0; m < e->nmembers; m++) 
		if(num == e->members[m].value)
			return e->members[m].name;

	BR_ERROR1("Unknown enum %d",num);

	return NULL;
}


/*
 * Write a structure to a text file using the given template
 *
 */
STATIC br_uint_32 StructWriteTextSub(br_datafile *df, br_file_struct *str, void *base, int indent);

STATIC br_uint_32 DfStructWriteText(br_datafile *df, br_file_struct *str, void *base)
{
	BrFilePrintf(df->h,"  struct    %s\n",str->name);

	StructWriteTextSub(df,str,base,4);

	return 1;
}

STATIC br_uint_32 StructWriteTextSub(br_datafile *df, br_file_struct *str, void *base, int indent)
{
	unsigned int m;
	int i,w,add_comment;
	void *mp;
	br_file_struct_member *sm;

	for(m = 0, sm = str->members ; m < str->nmembers; m++, sm++) {
		/*
		 * Get pointer to source memory
		 */
		mp = (char *)base + sm->offset;

		/*
		 * Identifer at start of line
		 */
		for(i=0; i< indent; i++)
			BrFilePutChar(' ',df->h);

		ASSERT(sm->type < BR_ASIZE(member_type_names));

		BrFilePrintf(df->h,"%-10s",member_type_names[sm->type]);

		/*
		 * By default, add member comment to end of line
		 */
		add_comment = 1;

		/*
		 * Do type specific processing
		 */
		switch(scalarTypeConvert(df, sm->type)) {
		case FSM_INT_8:
			w = BrFilePrintf(df->h,"%d",*((br_int_8 *)mp));
			break;

		case FSM_UINT_8:
			w = BrFilePrintf(df->h,"%u",*((br_uint_8 *)mp));
			break;

		case FSM_INT_16:
			w = BrFilePrintf(df->h,"%d",*((br_int_16 *)mp));
			break;

		case FSM_UINT_16:
			w = BrFilePrintf(df->h,"%u",*((br_uint_16 *)mp));
			break;

		case FSM_COLOUR:
			w = BrFilePrintf(df->h,"%d,%d,%d",
				BR_RED(*((br_colour *)mp)),
				BR_GRN(*((br_colour *)mp)),
				BR_BLU(*((br_colour *)mp)));
			break;

		case FSM_COLOUR_ALPHA:
			w = BrFilePrintf(df->h,"%d,%d,%d,%d",
				BR_RED(*((br_colour *)mp)),
				BR_GRN(*((br_colour *)mp)),
				BR_BLU(*((br_colour *)mp)),
				BR_ALPHA(*((br_colour *)mp)));
			break;

		case FSM_INT_32:
			w = BrFilePrintf(df->h,"%d",*((br_int_32 *)mp));
			break;

		case FSM_UINT_32:
			w = BrFilePrintf(df->h,"%u",*((br_uint_32 *)mp));
			break;

		case FSM_FIXED:
			w = BrFilePrintf(df->h,"%g",BrFixedToFloat(*((br_fixed_ls *)mp)));
			break;

		case FSM_FIXED_FRACTION:
			w = BrFilePrintf(df->h,"%g",BrFixedLSFToFloat(*((br_fixed_lsf *)mp)));
			break;

		case FSM_FIXED_UFRACTION:
			w = BrFilePrintf(df->h,"%g",BrFixedLUFToFloat(*((br_fixed_luf *)mp)));
			break;

		case FSM_ANGLE:
			w = BrFilePrintf(df->h,"%g",BrScalarToFloat(BrAngleToDegree(*((br_angle *)mp))));
			break;

		case FSM_FLOAT:
		case FSM_FLOAT_FRACTION:
		case FSM_FLOAT_UFRACTION:
			w = BrFilePrintf(df->h,"%g",*((float *)mp));
			break;

		case FSM_DOUBLE:
			w = BrFilePrintf(df->h,"%g",*((double *)mp));
			break;

		case FSM_ENUM_8:
			w = BrFilePrintf(df->h,"%s",EnumToString(sm->extra,*((br_uint_8 *)mp)));
			break;

		case FSM_ENUM_16:
			w = BrFilePrintf(df->h,"%s",EnumToString(sm->extra,*((br_uint_16 *)mp)));
			break;

		case FSM_ENUM_32:
			w = BrFilePrintf(df->h,"%s",EnumToString(sm->extra,*((br_uint_32 *)mp)));
			break;

		case FSM_FIXED_VECTOR2:
			w = BrFilePrintf(df->h,"%g,%g",
				BrFixedToFloat(((br_vector2_x *)mp)->v[0]),
				BrFixedToFloat(((br_vector2_x *)mp)->v[1]));
			break;

		case FSM_FIXED_VECTOR3:
			w = BrFilePrintf(df->h,"%g,%g,%g",
				BrFixedToFloat(((br_vector3_x *)mp)->v[0]),
				BrFixedToFloat(((br_vector3_x *)mp)->v[1]),
				BrFixedToFloat(((br_vector3_x *)mp)->v[2]));
			break;

		case FSM_FIXED_VECTOR4:
			w = BrFilePrintf(df->h,"%g,%g,%g,%g",
				BrFixedToFloat(((br_vector4_x *)mp)->v[0]),
				BrFixedToFloat(((br_vector4_x *)mp)->v[1]),
				BrFixedToFloat(((br_vector4_x *)mp)->v[2]),
				BrFixedToFloat(((br_vector4_x *)mp)->v[3]));
			break;

		case FSM_FLOAT_VECTOR2:
			w = BrFilePrintf(df->h,"%g,%g",
				((br_vector2_f *)mp)->v[0],
				((br_vector2_f *)mp)->v[1]);
			break;

		case FSM_FLOAT_VECTOR3:
			w = BrFilePrintf(df->h,"%g,%g,%g",
				((br_vector3_f *)mp)->v[0],
				((br_vector3_f *)mp)->v[1],
				((br_vector3_f *)mp)->v[2]);
			break;

		case FSM_FLOAT_VECTOR4:
			w = BrFilePrintf(df->h,"%g,%g,%g,%g",
				((br_vector4_f *)mp)->v[0],
				((br_vector4_f *)mp)->v[1],
				((br_vector4_f *)mp)->v[2],
				((br_vector4_f *)mp)->v[3]);
			break;


		case FSM_FIXED_FVECTOR2:
			w = BrFilePrintf(df->h,"%g,%g",
				BrFixedLSFToFloat(((br_fvector2_x *)mp)->v[0]),
				BrFixedLSFToFloat(((br_fvector2_x *)mp)->v[1]));
			break;

		case FSM_FIXED_FVECTOR3:
			w = BrFilePrintf(df->h,"%g,%g,%g",
				BrFixedLSFToFloat(((br_fvector3_x *)mp)->v[0]),
				BrFixedLSFToFloat(((br_fvector3_x *)mp)->v[1]),
				BrFixedLSFToFloat(((br_fvector3_x *)mp)->v[2]));
			break;

		case FSM_FIXED_FVECTOR4:
			w = BrFilePrintf(df->h,"%g,%g,%g,%g",
				BrFixedLSFToFloat(((br_fvector4_x *)mp)->v[0]),
				BrFixedLSFToFloat(((br_fvector4_x *)mp)->v[1]),
				BrFixedLSFToFloat(((br_fvector4_x *)mp)->v[2]),
				BrFixedLSFToFloat(((br_fvector4_x *)mp)->v[3]));
			break;

		case FSM_FLOAT_FVECTOR2:
			w = BrFilePrintf(df->h,"%g,%g",
				((br_fvector2_f *)mp)->v[0],
				((br_fvector2_f *)mp)->v[1]);
			break;

		case FSM_FLOAT_FVECTOR3:
			w = BrFilePrintf(df->h,"%g,%g,%g",
				((br_fvector3_f *)mp)->v[0],
				((br_fvector3_f *)mp)->v[1],
				((br_fvector3_f *)mp)->v[2]);
			break;

		case FSM_FLOAT_FVECTOR4:
			w = BrFilePrintf(df->h,"%g,%g,%g,%g",
				((br_fvector4_f *)mp)->v[0],
				((br_fvector4_f *)mp)->v[1],
				((br_fvector4_f *)mp)->v[2],
				((br_fvector4_f *)mp)->v[3]);
			break;

		case FSM_STRUCT:

			w = BrFilePrintf(df->h,"%s",((br_file_struct *)sm->extra)->name);

			/*
			 * Put member comment before structure
			 */
			add_comment = 0;

			if(sm->name) {
				for(i=w; i < 40; i++)
					BrFilePutChar(' ',df->h);

				BrFilePrintf(df->h," # %s",sm->name);
			}
			BrFilePutChar('\n',df->h);

			StructWriteTextSub(df,sm->extra,mp,indent+2);
			break;

		case FSM_ASCIZ:
			if(*((char **)mp))
				w = BrFilePrintf(df->h,"\"%s\"",*((char **)mp));
			else
				w = BrFilePrintf(df->h,"NULL");
			break;
		}

		/*
		 * Put member comment at end of line (column 60+indent)
		 */
		if(add_comment && sm->name) {
			for(i=w; i < 40; i++)
				BrFilePutChar(' ',df->h);

			BrFilePrintf(df->h," # %s\n",sm->name);
		}
	}

	return 1;
}


STATIC br_uint_32 StructReadTextSub(br_datafile *df, br_file_struct *str, void *base);

STATIC br_uint_32 DfStructReadText(br_datafile *df, br_file_struct *str, void *base)
{
	char *id,*data;

	/*
	 * Check input is the correct structure
	 */
	TextReadLine(df,&id,&data);

	if(BrStrCmp(id,"struct"))
		BR_ERROR1("Unknown text identifer \"%s\"",id);

	if(BrStrCmp(data,str->name))
		BR_ERROR1("Incorrect structure name \"%s\"",data);
	
	StructReadTextSub(df,str,base);

	return 1;
}

STATIC br_uint_32 StructReadTextSub(br_datafile *df, br_file_struct *str, void *base)
{
	unsigned int m,r,g,b,a;
	int i,n;
	void *mp;
	br_file_struct_member *sm;
	char *id,*data,*ep;

	/*
	 * Check input is the correct structure
	 */
	for(m = 0, sm = str->members ; m < str->nmembers; m++, sm++) {
		/*
		 * Get pointer to destination memory
		 */
		mp = (char *)base + sm->offset;

		/*
		 * Read next line
		 */
		if(!TextReadLine(df,&id,&data))
			BR_ERROR0("Unexpected EOF in strructure");

		if(BrStrCmp(id,member_type_names[sm->type]))
			BR_ERROR1("Unknown member identifer \"%s\"",id);

		/*
		 * Do type specific processing
		 */
		switch(scalarTypeConvert(df, sm->type)) {
		case FSM_INT_8:
			*((br_int_8 *)mp) = (br_int_8)BrStrToL(data,0,0);
			break;

		case FSM_UINT_8:
			*((br_uint_8 *)mp) = (br_uint_8)BrStrToUL(data,0,0);
			break;

		case FSM_INT_16:
			*((br_int_16 *)mp) = (br_int_16)BrStrToL(data,0,0);
			break;

		case FSM_UINT_16:
			*((br_uint_16 *)mp) = (br_uint_16)BrStrToUL(data,0,0);
			break;

		case FSM_COLOUR:
			r = BrStrToUL(data,&ep,0);
			if(*ep != ',')
				BR_ERROR0("Incorrect colour");

			g = BrStrToUL(ep+1,&ep,0);
			if(*ep != ',')
				BR_ERROR0("Incorrect colour");

			b = BrStrToUL(ep+1,&ep,0);

			*((br_colour *)mp) = BR_COLOUR_RGB(r,g,b);

			break;

		case FSM_COLOUR_ALPHA:
			r = BrStrToUL(data,&ep,0);
			if(*ep != ',')
				BR_ERROR0("Incorrect colour");

			g = BrStrToUL(ep+1,&ep,0);
			if(*ep != ',')
				BR_ERROR0("Incorrect colour");

			b = BrStrToUL(ep+1,&ep,0);
			if(*ep != ',')
				BR_ERROR0("Incorrect colour");

			a = BrStrToUL(ep+1,&ep,0);

			*((br_colour *)mp) = BR_COLOUR_RGBA(r,g,b,a);

			break;

		case FSM_INT_32:
			*((br_int_32 *)mp) = (br_int_32)BrStrToL(data,0,0);
			break;

		case FSM_UINT_32:
			*((br_uint_32 *)mp) = (br_uint_32)BrStrToUL(data,0,0);
			break;

		case FSM_FIXED:
			*((br_fixed_ls *)mp) = BrFloatToFixed(BrStrToD(data,NULL));
			break;

		case FSM_FIXED_FRACTION:
			*((br_fixed_lsf *)mp) = BrFloatToFixedLSF(BrStrToD(data,NULL));
			break;

		case FSM_FIXED_UFRACTION:
			*((br_fixed_luf *)mp) = BrFloatToFixedLUF(BrStrToD(data,NULL));
			break;

		case FSM_ANGLE:
			*((br_angle *)mp) = BrDegreeToAngle(BrFloatToScalar(BrStrToF(data,NULL)));
			break;

		case FSM_FLOAT:
		case FSM_FLOAT_FRACTION:
		case FSM_FLOAT_UFRACTION:
			*((float *)mp) = BrStrToF(data, NULL);
			break;

		case FSM_DOUBLE:
			*((double *)mp) = BrStrToD(data, NULL);
			break;

		case FSM_ENUM_8:
			*((br_uint_8 *)mp) = EnumFromString(sm->extra,data);
			break;

		case FSM_ENUM_16:
			*((br_uint_16 *)mp) = EnumFromString(sm->extra,data);
			break;

		case FSM_ENUM_32:
			*((br_uint_32 *)mp) = EnumFromString(sm->extra,data);
			break;

		case FSM_FIXED_VECTOR2:
			n = 2;
			goto get_fixed_vector;

		case FSM_FIXED_VECTOR3:
			n = 3;
			goto get_fixed_vector;

		case FSM_FIXED_VECTOR4:
			n = 4;
		get_fixed_vector:
			for(i=0; i< n; i++) {
				while(*data == ',' || ISSPACE(*data))
					data++;

				if(*data == '\0')
					BR_ERROR0("Incorrect vector");

				((br_vector4_x *)mp)->v[i] = BrFloatToFixed(BrStrToF(data, NULL));

				while(*data != '\0' && *data != ',' && !ISSPACE(*data))
					data++;
			}
			break;

		case FSM_FLOAT_VECTOR2:
			n = 2;
			goto get_float_vector;

		case FSM_FLOAT_VECTOR3:
			n = 3;
			goto get_float_vector;

		case FSM_FLOAT_VECTOR4:
			n = 4;
		get_float_vector:
			for(i=0; i< n; i++) {
				while(*data == ',' || ISSPACE(*data))
					data++;

				if(*data == '\0')
					BR_ERROR0("Incorrect vector");

				((br_vector4_f *)mp)->v[i] = BrStrToF(data, NULL);

				while(*data != '\0' && *data != ',' && !ISSPACE(*data))
					data++;
			}
			break;

		case FSM_FIXED_FVECTOR2:
			n = 2;
			goto get_fixed_fvector;

		case FSM_FIXED_FVECTOR3:
			n = 3;
			goto get_fixed_fvector;

		case FSM_FIXED_FVECTOR4:
			n = 4;
		get_fixed_fvector:
			for(i=0; i< n; i++) {
				while(*data == ',' || ISSPACE(*data))
					data++;

				if(*data == '\0')
					BR_ERROR0("Incorrect vector");

				((br_fvector4_x *)mp)->v[i] = BrFloatToFixedLSF(BrStrToF(data, NULL));

				while(*data != '\0' && *data != ',' && !ISSPACE(*data))
					data++;
			}
			break;

		case FSM_FLOAT_FVECTOR2:
			n = 2;
			goto get_float_fvector;

		case FSM_FLOAT_FVECTOR3:
			n = 3;
			goto get_float_fvector;

		case FSM_FLOAT_FVECTOR4:
			n = 4;
		get_float_fvector:
			for(i=0; i< n; i++) {
				while(*data == ',' || ISSPACE(*data))
					data++;

				if(*data == '\0')
					BR_ERROR0("Incorrect vector");

				((br_fvector4_f *)mp)->v[i] = BrStrToF(data, NULL);

				while(*data != '\0' && *data != ',' && !ISSPACE(*data))
					data++;
			}
			break;

		case FSM_STRUCT:
			if(BrStrCmp(data,((br_file_struct *)sm->extra)->name))
				BR_ERROR1("Incorrect structure name \"%s\"",data);

			StructReadTextSub(df,sm->extra,mp);
			break;

		case FSM_ASCIZ:
			if(!BrStrCmp(data,"NULL") || (*data != '"')) {
				*((char **)mp) = NULL;
			} else {
				*((char **)mp) = BrResStrDup(df->res?df->res:fw.res, data+1);
			};
		}
	}

	return 1;
}

/*
 * Find the number of lines a structure would occupy when written to a
 * text file
 */
STATIC br_uint_32 DfStructSizeText(br_datafile *df, br_file_struct *str, void *base)
{
	unsigned int m;
	br_file_struct_member *sm;
	br_uint_32 lines = 1;	/* Structure has 1 line header */

	/*
	 * Each member occupies one line, except structures,
	 * for which we recurse
	 */
	for(m = 0, sm = str->members ; m < str->nmembers; m++, sm++) {
		if(sm->type == FSM_STRUCT)
			 lines += DfStructSizeText(df,sm->extra, (char *)base+sm->offset);
		else
			lines++;
	}

	return lines;
}

/*
 * Write an array of structures to the current file as a chunk
 */
br_uint_32 DfStructWriteArray(br_datafile *df, br_file_struct *str, void *base, int n)
{
	char *cp = base;
	int i;

	for(i=0; i< n; i++, cp += str->mem_size)
		df->prims->struct_write(df,str,cp);

	return i;
}

/*
 * Read a number of structures from a file
 */
br_uint_32 DfStructReadArray(br_datafile *df, br_file_struct *str,void *base, int n)
{
	char *cp = base;
	int i;

	for(i=0; i< n; i++, cp += str->mem_size)
		if(BrFileEof(df->h))
			break;
		else
		df->prims->struct_read(df,str,cp);

	return i;
}


/**
 ** Reading and writing chunk headers
 **/
STATIC char *ChunkNames[] = {
	[FID_END]                   = "END",
	[FID_IMAGE_PLANE]           = "IMAGE_PLANE",
	[FID_RLE_IMAGE_PLANE]       = "RLE_IMAGE_PLANE",
	[FID_OLD_PIXELMAP]          = "OLD_PIXELMAP",
	[FID_MATERIAL_OLD]          = "MATERIAL_OLD",
	[FID_ADD_MATERIAL]          = "ADD_MATERIAL",
	[FID_OLD_ACTOR]             = "OLD_ACTOR",
	[FID_OLD_ADD_SIBLING]       = "OLD_ADD_SIBLING",
	[FID_OLD_ADD_CHILD]         = "OLD_ADD_CHILD",
	[FID_OLD_MATERIAL_INDEX]    = "OLD_MATERIAL_INDEX",
	[FID_OLD_VERTICES]          = "OLD_VERTICES",
	[FID_OLD_VERTICES_UV]       = "OLD_VERTICES_UV",
	[FID_OLD_FACES]             = "OLD_FACES",
	[FID_OLD_MODEL]             = "OLD_MODEL",
	[FID_ADD_MODEL]             = "ADD_MODEL",
	[FID_ANIM]                  = "ANIM",
	[FID_ANIM_TRANSFORM]        = "ANIM_TRANSFORM",
	[FID_ANIM_RATE]             = "ANIM_RATE",
	[FID_FILE_INFO]             = "FILE_INFO",
	[FID_OLD_LIGHT]             = "OLD_LIGHT",
	[FID_OLD_CAMERA]            = "OLD_CAMERA",
	[FID_PIVOT]                 = "PIVOT",
	[FID_MATERIAL_INDEX]        = "MATERIAL_INDEX",
	[FID_VERTICES]              = "VERTICES",
	[FID_VERTEX_UV]             = "VERTEX_UV",
	[FID_OLD_FACES_1]           = "OLD_FACES_1",
	[FID_FACE_MATERIAL]         = "FACE_MATERIAL",
	[FID_OLD_MODEL_1]           = "OLD_MODEL_1",
	[FID_COLOUR_MAP_REF]        = "COLOUR_MAP_REF",
	[FID_OPACITY_MAP_REF]       = "OPACITY_MAP_REF",
	[FID_INDEX_BLEND_REF]       = "INDEX_BLEND_REF",
	[FID_INDEX_SHADE_REF]       = "INDEX_SHADE_REF",
	[FID_SCREENDOOR_REF]        = "SCREENDOOR_REF",
	[FID_PIXELS]                = "PIXELS",
	[FID_ADD_MAP]               = "ADD_MAP",
	[FID_ACTOR]                 = "ACTOR",
	[FID_ACTOR_MODEL]           = "ACTOR_MODEL",
	[FID_ACTOR_TRANSFORM]       = "ACTOR_TRANSFORM",
	[FID_ACTOR_MATERIAL]        = "ACTOR_MATERIAL",
	[FID_ACTOR_LIGHT]           = "ACTOR_LIGHT",
	[FID_ACTOR_CAMERA]          = "ACTOR_CAMERA",
	[FID_ACTOR_BOUNDS]          = "ACTOR_BOUNDS",
	[FID_ACTOR_ADD_CHILD]       = "ACTOR_ADD_CHILD",
	[FID_TRANSFORM_MATRIX34]    = "TRANSFORM_MATRIX34",
	[FID_TRANSFORM_MATRIX34_LP] = "TRANSFORM_MATRIX34_LP",
	[FID_TRANSFORM_QUAT]        = "TRANSFORM_QUAT",
	[FID_TRANSFORM_EULER]       = "TRANSFORM_EULER",
	[FID_TRANSFORM_LOOK_UP]     = "TRANSFORM_LOOK_UP",
	[FID_TRANSFORM_TRANSLATION] = "TRANSFORM_TRANSLATION",
	[FID_TRANSFORM_IDENTITY]    = "TRANSFORM_IDENTITY",
	[FID_BOUNDS]                = "BOUNDS",
	[FID_LIGHT_OLD]             = "LIGHT_OLD",
	[FID_CAMERA]                = "CAMERA",
	[FID_FACES]                 = "FACES",
	[FID_OLD_MODEL_2]           = "OLD_MODEL_2",
	[FID_ACTOR_CLIP_PLANE]      = "ACTOR_CLIP_PLANE",
	[FID_PLANE]                 = "PLANE",
	[FID_SATURN_FACES]          = "SATURN_FACES",
	[FID_SATURN_MODEL]          = "SATURN_MODEL",
	[FID_INDEX_FOG_REF]         = "INDEX_FOG_REF",
	[FID_MATERIAL]              = "MATERIAL",
	[FID_PIXELMAP]              = "PIXELMAP",
	[FID_RESERVED_0]            = "RESERVED_0",
	[FID_LIGHT]                 = "LIGHT",
	[FID_MODEL]                 = "MODEL",
	[FID_VERTEX_COLOUR]         = "VERTEX_COLOUR",
	[FID_VERTEX_NORMAL]         = "VERTEX_NORMAL",
	[FID_FACE_COLOUR]           = "FACE_COLOUR",
	[FID_FACE_EQUATION]         = "FACE_EQUATION",
};

/*
 * Write out a chunk header in text format
 */
STATIC int DfChunkWriteText(br_datafile *df, br_uint_32 id, br_uint_32 length)
{
	ASSERT(id < BR_ASIZE(ChunkNames));

	if(id < BR_ASIZE(ChunkNames))
		BrFilePrintf(df->h,"*%-16s %d\n",ChunkNames[id],length);
	else
		BrFilePrintf(df->h,"*0x%08x %d\n",id,length);
		
	return 0;
}

/*
 * Read a chunk header in text format
 */
STATIC int DfChunkReadText(br_datafile *df, br_uint_32 *plength)
{
	int i;
	char *id,*data;

	if(!TextReadLine(df,&id,&data))
		return -1;

	if(id[0] != '*')
		BR_ERROR0("Chunk ID not found");

	id++;

	/*
	 * Convert or look up name
	 */
	if(id[0] == '0') {
		i = BrStrToUL(id,0,0);
	} else {
		for(i=0; i< BR_ASIZE(ChunkNames) ; i++)
			if(!BrStrCmp(ChunkNames[i],id))
				break;

		if(i >= BR_ASIZE(ChunkNames))
			BR_ERROR1("Chunk ID not known: %s",id);
	}

	/*
	 * Convert length
	 */
	if(plength)
		*plength = BrStrToUL(data,0,0);

 	return i;
}

/*
 * Write out a chunk header in binary format
 */
STATIC int DfChunkWriteBinary(br_datafile *df, br_uint_32 id, br_uint_32 length)
{
	br_uint_32 l;

	l = BrHtoNL(id);
	BrFileWrite(&l,sizeof(l),1,df->h);

	l = BrHtoNL(length);
	BrFileWrite(&l,sizeof(l),1,df->h);

	return 0;
}

/*
 * Read a chunk header in binary format
 */
STATIC int DfChunkReadBinary(br_datafile *df, br_uint_32 *plength)
{
	br_uint_32 id,l;

	if(BrFileEof(df->h))
		return -1;

	BrFileRead(&id,sizeof(id),1,df->h);

	if(BrFileEof(df->h))
		return -1;

	id = BrHtoNL(id);
	
	BrFileRead(&l,sizeof(l),1,df->h);

	if(BrFileEof(df->h))
		return -1;

	if(plength)
		*plength = BrHtoNL(l);

	return id;
}


STATIC void DfCountWriteText(br_datafile *df, br_uint_32 count)
{
	BrFilePrintf(df->h," count %d\n",count);
}

STATIC br_uint_32 DfCountReadText(br_datafile *df)
{
	char *id,*data;

	TextReadLine(df,&id,&data);	
	
	if(BrStrCmp(id,"count"))
		BR_ERROR0("no element count for chunk");

	return BrStrToUL(data,0,0);
}

STATIC void DfCountWriteBinary(br_datafile *df, br_uint_32 count)
{
	br_uint_32 l;

	l = BrHtoNL(count);
	BrFileWrite(&l,sizeof(l),1,df->h);
}

STATIC br_uint_32 DfCountReadBinary(br_datafile *df)
{
	br_uint_32 l;

	BrFileRead(&l,sizeof(l),1,df->h);

	return BrHtoNL(l);
}


br_size_t DfCountSizeText(br_datafile *df)
{
	return 1;
}

STATIC br_size_t DfCountSizeBinary(br_datafile *df)
{
	return sizeof(br_uint_32);
}

STATIC br_uint_8 *BlockWriteSetup(void *base, int block_size, int block_stride, int block_count, int size)
{
	int b;
	br_uint_8 *block, *sp, *dp;

#if 1
	/*
	 * If blocks are contiguous, merge into 1 block
	 */
	if(block_stride == block_size) {
		block_size *= block_count;
		block_count = 1;
	}
#endif

#if !BR_ENDIAN_BIG
	if(size == 1 && block_count == 1) 
#else
	if(block_count == 1) 
#endif
		return base;

	block = BrScratchAllocate(block_count * block_size * size);

	sp = base;
	dp = block;

	for(b=0; b < block_count; b++) {
		BrMemCpy(dp,sp,block_size*size);
		sp += block_stride;
		dp += block_size*size;
	}

#if !BR_ENDIAN_BIG
	BrSwapBlock(block,block_size * block_count,size);
#endif

	return block;
}


/*
 * Write out a block of elements in text format
 */
STATIC int DfBlockWriteText(br_datafile *df, void *base, int block_size, int block_stride, int block_count, int size)
{
	int i;
	br_uint_8 *cp,*block;
	int count = block_count * block_size;

	block = BlockWriteSetup(base, block_size, block_stride, block_count, size);

 	BrFilePrintf(df->h,"  block %d\n", count);
 	BrFilePrintf(df->h,"  size %d\n",size);

	for(i=0, cp=block; i < size*count; i++,cp++) {
		if((i%TEXT_BLOCK_LINE)==0)
			BrFilePrintf(df->h,"    %08x: %02x",i,*cp);
		else
			BrFilePrintf(df->h,"%02x",*cp);
		
		if((i%TEXT_BLOCK_LINE)==(TEXT_BLOCK_LINE-1))
			BrFilePutChar('\n',df->h);
	}

	if((i%TEXT_BLOCK_LINE) != 0)
		BrFilePutChar('\n',df->h);

	/*
	 * Release any allocated buffer
	 */
	if(block != base)
		BrScratchFree(block);

	return 0;
}

/*
 * Read a block of bytes in text format
 */
STATIC void *DfBlockReadText(br_datafile *df, void *base, int *count, int size, int mtype)
{
	char *id,*data;
	int l,s,a;
	char b[3];

	/*
	 * Find length of incoming block
	 */
	TextReadLine(df,&id,&data);
	if(BrStrCmp(id,"block"))
		BR_ERROR0("no block");
	l = BrStrToUL(data,0,0);

	TextReadLine(df,&id,&data);
	if(BrStrCmp(id,"size"))
		BR_ERROR0("no size");
	s = BrStrToUL(data,0,0);

	if(s != size)
		BR_ERROR0("block size mismatch");

	if(base == NULL) {
		/*
		 * If no existing block, allocate a new one and read data in
		 */
		base = BrResAllocate(df->res?df->res:fw.res,(br_size_t)l*size,(br_uint_8)mtype);
	} else {
		/*
		 * An existing block - limit the amount read
		 */
		if(*count < l)
			BR_ERROR1("DfBlockReadText: block too long: %d",l);
	}

	/*
	 * Return the actual size 
	 */
	*count = l;

	/*
	 * Read the block
	 */
	a = 0;
	b[2] = '\0';
	while(a < l*size) {
		TextReadLine(df,&id,&data);

		/*
		 * Check address of line
		 */
		if(a != BrStrToL(id,0,16))
			BR_ERROR0("block address mismatch");

		/*
		 * Consume pairs of hex digits
		 */
		for( ; data[0] && data[1]; data +=2, a++) {
			b[0] = data[0];
			b[1] = data[1];
			((br_uint_8 *)base)[a] = (br_uint_8)BrStrToUL(b,0,16);
		}
	}

	/*
	 * Byte swap it if necessary
	 */
#if !BR_ENDIAN_BIG
	BrSwapBlock(base,l,size);
#endif
	return base;
}

/*
 * Write out a block of bytes in binary format
 */
STATIC int DfBlockWriteBinary(br_datafile *df, void *base, int block_size, int block_stride, int block_count, int size)
{
	int count = block_size * block_count;
	br_uint_32 l = BrHtoNL(count);
	br_uint_32 s = BrHtoNL(size);
	void *block;

	block = BlockWriteSetup(base, block_size, block_stride, block_count, size);

	BrFileWrite(&l,sizeof(l),1,df->h);
	BrFileWrite(&s,sizeof(s),1,df->h);
	BrFileWrite(block,count,size,df->h);

	/*
	 * Release any allocated buffer
	 */
	if(block != base)
		BrScratchFree(block);

	return 0;
}

/*
 * Read a block of bytes in binary format
 */
STATIC void *DfBlockReadBinary(br_datafile *df, void *base, int *count, int size, int mtype)
{
	int l,s;

	/*
	 * Find length of incoming block
	 */
	BrFileRead(&l,sizeof(l),1,df->h);
	l = BrHtoNL(l);

	BrFileRead(&s,sizeof(s),1,df->h);
	s = BrHtoNL(s);

	if(s != size)
		BR_ERROR0("block size mismatch");

	if(base == NULL) {
		/*
		 * If no existing block, allocate a new one and read data in
		 */
		base = BrResAllocate(df->res?df->res:fw.res,(br_size_t)l*size,(br_uint_8)mtype);
	} else {
		/*
		 * An existing block - limit the amount read
		 */
		if(*count < l)
			BR_ERROR1("DfBlockReadBinary: block too long: %d",l);

	}

	/*
	 * Return the actual size 
	 */
	*count = l;

	/*
	 * Read the block
	 */
	BrFileRead(base,l,size,df->h);

#if !BR_ENDIAN_BIG
	BrSwapBlock(base,l,size);
#endif

	return base;
}

/*
 * Return size of a block of bytes in text format
 */
STATIC br_size_t DfBlockSizeText(br_datafile *df, void *base, int block_size, int block_stride, int block_count, int size)
{
	return (size*block_count*block_size+TEXT_BLOCK_LINE-1)/TEXT_BLOCK_LINE+2;
}

/*
 * Return size of a block of bytes in binary format
 */
STATIC size_t DfBlockSizeBinary(br_datafile *df, void *base, int block_size, int block_stride, int block_count, int size)
{
	return size*block_count*block_size+sizeof(br_uint_32)*2;
}

/**
 ** Names
 **/
STATIC char *DfNameReadText(br_datafile *df, char *name)
{
	char *id,*data;

	TextReadLine(df,&id,&data);

	if(BrStrCmp(id,"name"))
		BR_ERROR0("no name");
	if(data == NULL || *data != '\"')
		BR_ERROR0("no name string");
	
	/*
	 * Copy name into callers buffer
	 */
	BrStrNCpy(name, data+1, BR_MAX_NAME-1);
	name[BR_MAX_NAME-1] ='\0';

	return name;
}

STATIC int DfNameWriteText(br_datafile *df, const char *name)
{
	BrFilePrintf(df->h,"  name \"%s\"\n",name?name:"NULL");

	return 0;
}

STATIC br_size_t DfNameSizeText(br_datafile *df, const char *name)
{
	return 1;
}

STATIC char *DfNameReadBinary(br_datafile *df, char *name)
{
	int c;
	int i;

	for(i=0; i < BR_MAX_NAME-1; i++) {
		c = BrFileGetChar(df->h);
		if(c == '\0' || c == BR_EOF)
			break;

		name[i] = c;
	}

	/*
	 * Terminate
	 */
	name[i] = '\0';

	return name;
}

STATIC int DfNameWriteBinary(br_datafile *df, const char *name)
{
	if(name)
		BrFileWrite(name,1,BrStrLen(name),df->h);

	BrFilePutChar('\0',df->h);
	return 0;
}

STATIC br_size_t DfNameSizeBinary(br_datafile *df, const char *name)
{
	if(name)
		return BrStrLen(name)+1;
	else
		return 1;
}

/*
 * Skip a given length in text file
 */
STATIC int DfSkipText(br_datafile *df, br_uint_32 length)
{
	char *id, *data;

	while(!BrFileEof(df->h) && length--)
		TextReadLine(df,&id,&data);

	return 0;
}

/*
 * Skip a given length in binary file
 */
STATIC int DfSkipBinary(br_datafile *df, br_uint_32 length)
{
	BrFileAdvance(length, df->h);

	return 0;
}

/*
 * Interpret a stream of chunks from a file by calling
 * routines looked up from a given table
 */
int DfChunksInterpret(br_datafile *df, br_chunks_table *table)
{
	br_uint_32 length,count;
	br_uint_32 id;
	int r,i;

#if CHUNK_LOG
		BrLogPrintf("ChunkInterpret\n");
#endif
	/*
	 * Consume chunks until End Of File or a handler return != 0
	 *
	 * Return 0 at EOF, or handler return
	 */
	for(;;) {

		/*
		 * Read the next chunk header
		 */
		id  = df->prims->chunk_read(df, &length);

		/*
		 * Catch EOF
		 */
		if(id == 0xFFFFFFFF) {
#if CHUNK_LOG
			BrLogPrintf("EOF\n");
#endif
			return 0;
		}

		/*
		 * Lookup ID in table
		 */
		for(i=0; i < table->nentries; i++)
			if(id == table->entries[i].id)
				break;

#if CHUNK_LOG
		BrLogPrintf("%s ",ChunkNames[id]);
#endif

		/*
		 * If handler exists, call it (with count if needed)
		 */
		if(i < table->nentries) {
#if CHUNK_LOG
			BrLogPrintf("calling\n");
#endif
			if(table->entries[i].has_count)
				count = df->prims->count_read(df);
			else
				count = 0;

			r = table->entries[i].handler(df,id,length,count);
			if(r)
				return r;

		} else {
#if CHUNK_LOG
			BrLogPrintf("skipping\n");
#endif
			df->prims->skip(df,length);
		}
	}
}

STATIC int BrNullSkip(br_datafile *df, br_uint_32 length)
{
	BR_FATAL0("Invald file primitive call");
	return -1;
}

STATIC int BrNullChunkWrite(br_datafile *df, br_uint_32 id, br_uint_32 length)
{
	BR_FATAL0("Invald file primitive call");
	return -1;
}

STATIC int BrNullChunkRead(br_datafile *df, br_uint_32 *plength)
{
	BR_FATAL0("Invald file primitive call");
	return -1;
}

STATIC void BrNullCountWrite(br_datafile *df, br_uint_32 count)
{
	BR_FATAL0("Invald file primitive call");
}

STATIC br_uint_32 BrNullCountRead(br_datafile *df)
{
	BR_FATAL0("Invald file primitive call");
	return 0;
}

STATIC br_size_t BrNullCountSize(br_datafile *df)
{
	BR_FATAL0("Invald file primitive call");
	return 0;
}

STATIC br_uint_32 BrNullStructWrite(br_datafile *df, br_file_struct *str, void *base)
{
	BR_FATAL0("Invald file primitive call");
	return 0;
}

STATIC br_uint_32 BrNullStructRead(br_datafile *df, br_file_struct *str, void *base)
{
	BR_FATAL0("Invald file primitive call");
	return 0;
}

STATIC br_uint_32 BrNullStructSize(br_datafile *df, br_file_struct *str, void *base)
{
	BR_FATAL0("Invald file primitive call");
	return 0;
}

STATIC int BrNullBlockWrite(br_datafile *df, void *base, int block_size, int block_stride, int block_count, int size)
{
	BR_FATAL0("Invald file primitive call");
	return 1;
}

STATIC void * BrNullBlockRead(br_datafile *df, void *base, int *pcount, int size, int mtype)
{
	BR_FATAL0("Invald file primitive call");
	return NULL;
}

STATIC br_size_t BrNullBlockSize(br_datafile *df, void *base, int block_size, int block_stride, int block_count, int size)
{
	BR_FATAL0("Invald file primitive call");
	return 0;
}

STATIC int BrNullNameWrite(br_datafile *df, const char *name)
{
	BR_FATAL0("Invald file primitive call");
	return -1;
}

STATIC char * BrNullNameRead(br_datafile *df, char *name)
{
	BR_FATAL0("Invald file primitive call");
	return NULL;
}

STATIC br_size_t BrNullNameSize(br_datafile *df, const char *name)
{
	BR_FATAL0("Invald file primitive call");
	return 0;
}

/*
 * Handler structures for getting at the above functions
 */
br_file_primitives _BrFilePrimsNull = {
	.identifier     = "NULL",

	.skip           = BrNullSkip,

	.chunk_write    = BrNullChunkWrite,
	.chunk_read     = BrNullChunkRead,

	.count_write    = BrNullCountWrite,
	.count_read     = BrNullCountRead,
	.count_size     = BrNullCountSize,

	.struct_write   = BrNullStructWrite,
	.struct_read    = BrNullStructRead,
	.struct_size    = BrNullStructSize,

	.block_write    = BrNullBlockWrite,
	.block_read     = BrNullBlockRead,
	.block_size     = BrNullBlockSize,

	.name_write     = BrNullNameWrite,
	.name_read      = BrNullNameRead,
	.name_size      = BrNullNameSize,
};

STATIC br_file_primitives _BrFilePrimsReadBinary = {
	.identifier     = "Read Binary",

	.skip           = DfSkipBinary,

	.chunk_write    = BrNullChunkWrite,
	.chunk_read     = DfChunkReadBinary,

	.count_write    = BrNullCountWrite,
	.count_read     = DfCountReadBinary,
	.count_size     = DfCountSizeBinary,

	.struct_write   = BrNullStructWrite,
	.struct_read    = DfStructReadBinary,
	.struct_size    = DfStructSizeBinary,

	.block_write    = BrNullBlockWrite,
	.block_read     = DfBlockReadBinary,
	.block_size     = DfBlockSizeBinary,

	.name_write     = BrNullNameWrite,
	.name_read      = DfNameReadBinary,
	.name_size      = DfNameSizeBinary,
};

STATIC br_file_primitives _BrFilePrimsWriteBinary = {
	.identifier     = "Write Binary",

	.skip           = DfSkipBinary,

	.chunk_write    = DfChunkWriteBinary,
	.chunk_read     = BrNullChunkRead,

	.count_write    = DfCountWriteBinary,
	.count_read     = BrNullCountRead,
	.count_size     = DfCountSizeBinary,

	.struct_write   = DfStructWriteBinary,
	.struct_read    = BrNullStructRead,
	.struct_size    = DfStructSizeBinary,

	.block_write    = DfBlockWriteBinary,
	.block_read     = BrNullBlockRead,
	.block_size     = DfBlockSizeBinary,

	.name_write     = DfNameWriteBinary,
	.name_read      = BrNullNameRead,
	.name_size      = DfNameSizeBinary,
};

STATIC br_file_primitives _BrFilePrimsReadText = {
	.identifier     = "Read Text",

	.skip           = DfSkipText,

	.chunk_write    = BrNullChunkWrite,
	.chunk_read     = DfChunkReadText,

	.count_write    = BrNullCountWrite,
	.count_read     = DfCountReadText,
	.count_size     = DfCountSizeText,

	.struct_write   = BrNullStructWrite,
	.struct_read    = DfStructReadText,
	.struct_size    = DfStructSizeText,

	.block_write    = BrNullBlockWrite,
	.block_read     = DfBlockReadText,
	.block_size     = DfBlockSizeText,

	.name_write     = BrNullNameWrite,
	.name_read      = DfNameReadText,
	.name_size      = DfNameSizeText,
};

STATIC br_file_primitives _BrFilePrimsWriteText = {
	.identifier     = "Write Text",

	.skip           = DfSkipText,

	.chunk_write    = DfChunkWriteText,
	.chunk_read     = BrNullChunkRead,

	.count_write    = DfCountWriteText,
	.count_read     = BrNullCountRead,
	.count_size     = DfCountSizeText,

	.struct_write   = DfStructWriteText,
	.struct_read    = BrNullStructRead,
	.struct_size    = DfStructSizeText,

	.block_write    = DfBlockWriteText,
	.block_read     = BrNullBlockRead,
	.block_size     = DfBlockSizeText,

	.name_write     = DfNameWriteText,
	.name_read      = BrNullNameRead,
	.name_size      = DfNameSizeText,
};

/*
 * Identify a file type from it's magic numbers
 */
STATIC int BR_CALLBACK DfFileIdentify(br_uint_8 *magics,br_size_t n_magics)
{
	static char text_magics[8]   = "*FILE_IN";
	static char binary_magics[8] = "\0\0\0\022\0\0\0\010";

	ASSERT(n_magics == BR_ASIZE(text_magics));

	if(!BrMemCmp(magics,text_magics,BR_ASIZE(text_magics)))
		return BR_FS_MODE_TEXT;

	ASSERT(n_magics == BR_ASIZE(binary_magics));
	if(!BrMemCmp(magics,binary_magics,BR_ASIZE(binary_magics)))
		return BR_FS_MODE_BINARY;

	return BR_FS_MODE_UNKNOWN;
}

/*
 * Sets up a new current datafile for reading or writing, working out whether it 
 * is in text mode or not, returns true if open suceeded
 */
br_datafile *DfOpen(const char *name, int write, br_token scalar_type)
{
	int mode = fw.open_mode;
	br_datafile * df;
	void *h;

	/*
	 * Try and open file
	 */
	if(write)
		h = BrFileOpenWrite(name, mode);
	else
		h = BrFileOpenRead(name, 2 * sizeof(br_uint_32), DfFileIdentify, &mode);
	
	/*
 	 * Set up datafile structure
	 */

	if(h == NULL)
		return NULL;

	df = BrResAllocate(fw.res,sizeof(*df),BR_MEMORY_DATAFILE);
	df->h = h;
	df->prims = &_BrFilePrimsNull;
	df->scalar_type = scalar_type;

	switch(mode) {
	case BR_FS_MODE_TEXT:
		df->prims = write?&_BrFilePrimsWriteText:&_BrFilePrimsReadText;
		break;

	case BR_FS_MODE_BINARY:
		df->prims = write?&_BrFilePrimsWriteBinary:&_BrFilePrimsReadBinary;
		break;
	}

	/*
	 * Put a mark on stack
	 */
	DfPush(DFST_MARK,df,1);

	return df;
}


/*
 * Close the current datafile
 */
void DfClose(br_datafile *df)
{
	br_datafile *dfp;

	UASSERT(df != NULL);
	ASSERT(df->h != NULL);

	/*
	 * Clear back down to mark
	 */
	while(DfTopType() != DFST_MARK)
		DfPop(DfTopType(),NULL);

	/*
	 * Check for correct mark
	 */
	dfp = DfPop(DFST_MARK,NULL);
	ASSERT(df == dfp);

	BrFileClose(df->h);

	BrResFree(df);
}

/*
 * Set the current output mode   - page 64
   either BR_FS_MODE_TEXT or BR_FS_MODE_BINARY should be passed.
 */
int BR_PUBLIC_ENTRY BrWriteModeSet(int mode)
{


	int old = fw.open_mode;

UASSERT_MESSAGE("Invalid mode passed to BrWriteModeSet.\n\r Must be BR_FS_MODE_TEXT or BR_FS_MODE_BINARY.",((mode== BR_FS_MODE_TEXT) || (mode == BR_FS_MODE_BINARY)));

	fw.open_mode = mode;

	return old;
}
