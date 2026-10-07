/*
 * softprim timestamps
 */
#ifndef _SOFTPRIM_TIMESTMP_H_
#define _SOFTPRIM_TIMESTMP_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef br_uint_32 br_timestamp;

extern br_timestamp SoftPrimTimestamp;

#define Timestamp()     (SoftPrimTimestamp += 2)

#define TIMESTAMP_START 1

#ifdef __cplusplus
};
#endif
#endif
