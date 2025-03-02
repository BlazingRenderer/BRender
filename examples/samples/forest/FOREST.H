/* Copyright (c) 1993-1996 Argonaut Technologies Limited. All rights reserved.
 * Sample 10 Header File
 * 
 */

#define SCREEN_WIDTH	320

#ifdef _NTSC
#define SCREEN_HEIGHT	240		/* NTSC screen depth */
#else
#define SCREEN_HEIGHT	256		/* PAL screen depth */
#endif

/*
 * generic functions for samples
 */
br_int_8 _GenericInitDevice(void);
br_int_8 _GenericTerminate(void);
br_int_8 _GenericLoopTest(void);
br_int_8 _GenericDoubleBuffer(void);

br_uint_32 BR_CALLBACK ActorEnumCbfn(br_actor *mat, void *arg);

typedef br_int_8 _sample_fn(void);

/*
 * test harness generic functions
 */
typedef struct _sample_fn_list {

	 _sample_fn *sample_init_device;	/* init PS hardware, BRender, PSGfx */
	 _sample_fn *sample_init_world;		/* init data structures */
	 _sample_fn *sample_render_fn;		/* rendering function */
	 _sample_fn *sample_double_buffer;	/* double buffer function */
	 _sample_fn *sample_update_fn;		/* update world */
	 void *sample_double_buffer_cbfn;	/* double buffer callback */
	 _sample_fn *sample_loop_test;		/* loop decision */
	 _sample_fn *sample_pad_test;		/* joypad */
	 _sample_fn *sample_term;		/* terminate sample (shut down BRender, PSGfx */
    
} _sample_fn_list;

#define PAD_UP		PADLup
#define PAD_DOWN	PADLdown
#define PAD_LEFT	PADLleft
#define PAD_RIGHT	PADLright
#define	PAD_TRI		PADRup
#define PAD_CROSS	PADRdown
#define PAD_SQUARE	PADRleft
#define PAD_CIRCLE	PADRright
#define PAD_FRONT_LTOP	PADn
#define PAD_FRONT_LBOT	PADo
#define PAD_FRONT_RTOP	PADl
#define PAD_FRONT_RBOT	PADm
#define PAD_SELECT	PADk
#define PAD_START	PADh

#define PAD_COMPARE(a,b) ((a & b)?1:0)

#ifdef __PS__
#ifdef _NTSC
#define HBL_PER_VBL	262
#define VBL_PER_SEC	60
#else
#define HBL_PER_VBL	315
#define VBL_PER_SEC	50
#endif
#endif

