#include <stddef.h>
#include "brender.h"

#define width_space      0x02
#define width_exclam     0x01
#define width_dquote     0x03
#define width_hash       0x05
#define width_dollar     0x03
#define width_percent    0x05
#define width_ampersand  0x03
#define width_quote      0x02
#define width_lparen     0x02
#define width_rparen     0x02
#define width_asterisk   0x03
#define width_plus       0x03
#define width_comma      0x02
#define width_minus      0x03
#define width_period     0x02
#define width_slash      0x05
#define width_0          0x04
#define width_1          0x04
#define width_2          0x04
#define width_3          0x04
#define width_4          0x04
#define width_5          0x04
#define width_6          0x04
#define width_7          0x04
#define width_8          0x04
#define width_9          0x04
#define width_colon      0x01
#define width_semicolon  0x02
#define width_langle     0x03
#define width_equal      0x03
#define width_rangle     0x03
#define width_query      0x03
#define width_comat      0x04
#define width_A          0x04
#define width_B          0x04
#define width_C          0x04
#define width_D          0x04
#define width_E          0x04
#define width_F          0x04
#define width_G          0x04
#define width_H          0x04
#define width_I          0x01
#define width_J          0x04
#define width_K          0x04
#define width_L          0x04
#define width_M          0x05
#define width_N          0x04
#define width_O          0x04
#define width_P          0x04
#define width_Q          0x04
#define width_R          0x04
#define width_S          0x04
#define width_T          0x03
#define width_U          0x04
#define width_V          0x05
#define width_W          0x05
#define width_X          0x05
#define width_Y          0x05
#define width_Z          0x05
#define width_lsquare    0x02
#define width_bslash     0x05
#define width_rsquare    0x02
#define width_caret      0x03
#define width_underbar   0x04
#define width_bquote     0x02
#define width_a          0x03
#define width_b          0x03
#define width_c          0x03
#define width_d          0x03
#define width_e          0x03
#define width_f          0x02
#define width_g          0x03
#define width_h          0x03
#define width_i          0x01
#define width_j          0x02
#define width_k          0x03
#define width_l          0x01
#define width_m          0x05
#define width_n          0x03
#define width_o          0x03
#define width_p          0x03
#define width_q          0x03
#define width_r          0x03
#define width_s          0x03
#define width_t          0x03
#define width_u          0x03
#define width_v          0x03
#define width_w          0x05
#define width_x          0x03
#define width_y          0x03
#define width_z          0x03
#define width_lbrace     0x03
#define width_vbar       0x01
#define width_rbrace     0x03
#define width_tilde      0x04
#define width_bullet     0x03
#define width_cross      0x03
#define width_tick       0x05
#define width_uparrow    0x03
#define width_downarrow  0x03
#define width_rightarrow 0x04
#define width_leftarrow  0x04
#define width_solidbox   0x04
#define width_cursor     0x03

#pragma pack(push, 1)

typedef struct {
    br_uint_8 glyph_space[6];
    br_uint_8 glyph_exclam[6];
    br_uint_8 glyph_dquote[6];
    br_uint_8 glyph_hash[6];
    br_uint_8 glyph_dollar[6];
    br_uint_8 glyph_percent[6];
    br_uint_8 glyph_ampersand[6];
    br_uint_8 glyph_quote[6];
    br_uint_8 glyph_lparen[6];
    br_uint_8 glyph_rparen[6];
    br_uint_8 glyph_asterisk[6];
    br_uint_8 glyph_plus[6];
    br_uint_8 glyph_comma[6];
    br_uint_8 glyph_minus[6];
    br_uint_8 glyph_period[6];
    br_uint_8 glyph_slash[6];
    br_uint_8 glyph_0[6];
    br_uint_8 glyph_1[6];
    br_uint_8 glyph_2[6];
    br_uint_8 glyph_3[6];
    br_uint_8 glyph_4[6];
    br_uint_8 glyph_5[6];
    br_uint_8 glyph_6[6];
    br_uint_8 glyph_7[6];
    br_uint_8 glyph_8[6];
    br_uint_8 glyph_9[6];
    br_uint_8 glyph_colon[6];
    br_uint_8 glyph_semicolon[6];
    br_uint_8 glyph_langle[6];
    br_uint_8 glyph_equal[6];
    br_uint_8 glyph_rangle[6];
    br_uint_8 glyph_query[6];
    br_uint_8 glyph_comat[6];
    br_uint_8 glyph_A[6];
    br_uint_8 glyph_B[6];
    br_uint_8 glyph_C[6];
    br_uint_8 glyph_D[6];
    br_uint_8 glyph_E[6];
    br_uint_8 glyph_F[6];
    br_uint_8 glyph_G[6];
    br_uint_8 glyph_H[6];
    br_uint_8 glyph_I[6];
    br_uint_8 glyph_J[6];
    br_uint_8 glyph_K[6];
    br_uint_8 glyph_L[6];
    br_uint_8 glyph_M[6];
    br_uint_8 glyph_N[6];
    br_uint_8 glyph_O[6];
    br_uint_8 glyph_P[6];
    br_uint_8 glyph_Q[6];
    br_uint_8 glyph_R[6];
    br_uint_8 glyph_S[6];
    br_uint_8 glyph_T[6];
    br_uint_8 glyph_U[6];
    br_uint_8 glyph_V[6];
    br_uint_8 glyph_W[6];
    br_uint_8 glyph_X[6];
    br_uint_8 glyph_Y[6];
    br_uint_8 glyph_Z[6];
    br_uint_8 glyph_lsquare[6];
    br_uint_8 glyph_bslash[6];
    br_uint_8 glyph_rsquare[6];
    br_uint_8 glyph_caret[6];
    br_uint_8 glyph_underbar[6];
    br_uint_8 glyph_bquote[6];
    br_uint_8 glyph_a[6];
    br_uint_8 glyph_b[6];
    br_uint_8 glyph_c[6];
    br_uint_8 glyph_d[6];
    br_uint_8 glyph_e[6];
    br_uint_8 glyph_f[6];
    br_uint_8 glyph_g[6];
    br_uint_8 glyph_h[6];
    br_uint_8 glyph_i[6];
    br_uint_8 glyph_j[6];
    br_uint_8 glyph_k[6];
    br_uint_8 glyph_l[6];
    br_uint_8 glyph_m[6];
    br_uint_8 glyph_n[6];
    br_uint_8 glyph_o[6];
    br_uint_8 glyph_p[6];
    br_uint_8 glyph_q[6];
    br_uint_8 glyph_r[6];
    br_uint_8 glyph_s[6];
    br_uint_8 glyph_t[6];
    br_uint_8 glyph_u[6];
    br_uint_8 glyph_v[6];
    br_uint_8 glyph_w[6];
    br_uint_8 glyph_x[6];
    br_uint_8 glyph_y[6];
    br_uint_8 glyph_z[6];
    br_uint_8 glyph_lbrace[6];
    br_uint_8 glyph_vbar[6];
    br_uint_8 glyph_rbrace[6];
    br_uint_8 glyph_tilde[6];
    br_uint_8 glyph_bullet[6];
    br_uint_8 glyph_cross[6];
    br_uint_8 glyph_tick[6];
    br_uint_8 glyph_uparrow[6];
    br_uint_8 glyph_downarrow[6];
    br_uint_8 glyph_rightarrow[6];
    br_uint_8 glyph_leftarrow[6];
    br_uint_8 glyph_solidbox[6];
    br_uint_8 glyph_cursor[6];
} br_glyphs_t;

static br_glyphs_t glyphs = {
    {0,   0,   0,   0,   0,   0  }, /* glyph_space */
    {128, 128, 128, 0,   128, 0  }, /* glyph_exclam */
    {160, 160, 0,   0,   0,   0  }, /* glyph_dquote */
    {80,  248, 80,  248, 80,  0  }, /* glyph_hash */
    {64,  96,  192, 32,  192, 64 }, /* glyph_dollar */
    {200, 208, 32,  88,  152, 0  }, /* glyph_percent */
    {64,  160, 160, 64,  160, 0  }, /* glyph_ampersand */
    {64,  128, 0,   0,   0,   0  }, /* glyph_quote */
    {64,  128, 128, 128, 64,  0  }, /* glyph_lparen */
    {128, 64,  64,  64,  128, 0  }, /* glyph_rparen */
    {0,   160, 64,  160, 0,   0  }, /* glyph_asterisk */
    {0,   64,  224, 64,  0,   0  }, /* glyph_plus */
    {0,   0,   0,   0,   64,  128}, /* glyph_comma */
    {0,   0,   224, 0,   0,   0  }, /* glyph_minus */
    {0,   0,   0,   0,   128, 0  }, /* glyph_period */
    {8,   16,  32,  64,  128, 0  }, /* glyph_slash */
    {96,  144, 144, 144, 96,  0  }, /* glyph_0 */
    {32,  96,  32,  32,  112, 0  }, /* glyph_1 */
    {224, 16,  96,  128, 240, 0  }, /* glyph_2 */
    {224, 16,  96,  16,  224, 0  }, /* glyph_3 */
    {128, 128, 160, 240, 32,  0  }, /* glyph_4 */
    {240, 128, 224, 16,  224, 0  }, /* glyph_5 */
    {96,  128, 224, 144, 96,  0  }, /* glyph_6 */
    {240, 16,  32,  64,  128, 0  }, /* glyph_7 */
    {96,  144, 96,  144, 96,  0  }, /* glyph_8 */
    {96,  144, 112, 16,  96,  0  }, /* glyph_9 */
    {0,   128, 0,   128, 0,   0  }, /* glyph_colon */
    {0,   64,  0,   64,  128, 0  }, /* glyph_semicolon */
    {32,  64,  128, 64,  32,  0  }, /* glyph_langle */
    {0,   224, 0,   224, 0,   0  }, /* glyph_equal */
    {128, 64,  32,  64,  128, 0  }, /* glyph_rangle */
    {192, 32,  64,  0,   64,  0  }, /* glyph_query */
    {96,  176, 176, 128, 96,  0  }, /* glyph_comat */
    {96,  144, 240, 144, 144, 0  }, /* glyph_A */
    {224, 144, 224, 144, 224, 0  }, /* glyph_B */
    {96,  144, 128, 144, 96,  0  }, /* glyph_C */
    {224, 144, 144, 144, 224, 0  }, /* glyph_D */
    {240, 128, 224, 128, 240, 0  }, /* glyph_E */
    {240, 128, 224, 128, 128, 0  }, /* glyph_F */
    {96,  128, 176, 144, 96,  0  }, /* glyph_G */
    {144, 144, 240, 144, 144, 0  }, /* glyph_H */
    {128, 128, 128, 128, 128, 0  }, /* glyph_I */
    {16,  16,  16,  144, 96,  0  }, /* glyph_J */
    {144, 160, 192, 160, 144, 0  }, /* glyph_K */
    {128, 128, 128, 128, 240, 0  }, /* glyph_L */
    {136, 216, 168, 136, 136, 0  }, /* glyph_M */
    {144, 208, 176, 144, 144, 0  }, /* glyph_N */
    {96,  144, 144, 144, 96,  0  }, /* glyph_O */
    {224, 144, 224, 128, 128, 0  }, /* glyph_P */
    {96,  144, 144, 176, 112, 0  }, /* glyph_Q */
    {224, 144, 224, 144, 144, 0  }, /* glyph_R */
    {112, 128, 96,  16,  224, 0  }, /* glyph_S */
    {224, 64,  64,  64,  64,  0  }, /* glyph_T */
    {144, 144, 144, 144, 96,  0  }, /* glyph_U */
    {136, 136, 80,  80,  32,  0  }, /* glyph_V */
    {136, 136, 168, 168, 80,  0  }, /* glyph_W */
    {136, 80,  32,  80,  136, 0  }, /* glyph_X */
    {136, 80,  32,  32,  32,  0  }, /* glyph_Y */
    {248, 16,  32,  64,  248, 0  }, /* glyph_Z */
    {192, 128, 128, 128, 192, 0  }, /* glyph_lsquare */
    {128, 64,  32,  16,  8,   0  }, /* glyph_bslash */
    {192, 64,  64,  64,  192, 0  }, /* glyph_rsquare */
    {64,  160, 0,   0,   0,   0  }, /* glyph_caret */
    {0,   0,   0,   0,   0,   240}, /* glyph_underbar */
    {128, 64,  0,   0,   0,   0  }, /* glyph_bquote */
    {0,   96,  160, 160, 96,  0  }, /* glyph_a */
    {128, 192, 160, 160, 192, 0  }, /* glyph_b */
    {0,   96,  128, 128, 96,  0  }, /* glyph_c */
    {32,  96,  160, 160, 96,  0  }, /* glyph_d */
    {0,   64,  160, 192, 96,  0  }, /* glyph_e */
    {64,  128, 192, 128, 128, 0  }, /* glyph_f */
    {0,   96,  160, 96,  32,  64 }, /* glyph_g */
    {128, 192, 160, 160, 160, 0  }, /* glyph_h */
    {128, 0,   128, 128, 128, 0  }, /* glyph_i */
    {64,  0,   64,  64,  64,  128}, /* glyph_j */
    {128, 160, 192, 192, 160, 0  }, /* glyph_k */
    {128, 128, 128, 128, 128, 0  }, /* glyph_l */
    {0,   208, 168, 168, 168, 0  }, /* glyph_m */
    {0,   192, 160, 160, 160, 0  }, /* glyph_n */
    {0,   64,  160, 160, 64,  0  }, /* glyph_o */
    {0,   192, 160, 160, 192, 128}, /* glyph_p */
    {0,   96,  160, 160, 96,  32 }, /* glyph_q */
    {0,   96,  128, 128, 128, 0  }, /* glyph_r */
    {0,   96,  192, 32,  192, 0  }, /* glyph_s */
    {64,  224, 64,  64,  64,  0  }, /* glyph_t */
    {0,   160, 160, 160, 96,  0  }, /* glyph_u */
    {0,   160, 160, 64,  64,  0  }, /* glyph_v */
    {0,   168, 168, 168, 80,  0  }, /* glyph_w */
    {0,   160, 64,  64,  160, 0  }, /* glyph_x */
    {0,   160, 160, 96,  32,  64 }, /* glyph_y */
    {0,   224, 32,  192, 224, 0  }, /* glyph_z */
    {32,  64,  192, 64,  32,  0  }, /* glyph_lbrace */
    {128, 128, 0,   128, 128, 0  }, /* glyph_vbar */
    {128, 64,  96,  64,  128, 0  }, /* glyph_rbrace */
    {80,  160, 0,   0,   0,   0  }, /* glyph_tilde */
    {0,   224, 224, 224, 0,   0  }, /* glyph_bullet */
    {0,   160, 64,  160, 0,   0  }, /* glyph_cross */
    {0,   8,   16,  160, 64,  0  }, /* glyph_tick */
    {0,   64,  224, 64,  64,  0  }, /* glyph_uparrow */
    {0,   64,  64,  224, 64,  0  }, /* glyph_downarrow */
    {0,   32,  240, 32,  0,   0  }, /* glyph_rightarrow */
    {0,   64,  240, 64,  0,   0  }, /* glyph_leftarrow */
    {240, 240, 240, 240, 240, 240}, /* glyph_solidbox */
    {160, 64,  64,  64,  64,  160}  /* glyph_cursor */
};

// clang-format off
static br_uint_16 encoding[] = {
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_bullet),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_uparrow),
    offsetof(br_glyphs_t, glyph_downarrow),
    offsetof(br_glyphs_t, glyph_rightarrow),
    offsetof(br_glyphs_t, glyph_leftarrow),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_exclam),
    offsetof(br_glyphs_t, glyph_dquote),
    offsetof(br_glyphs_t, glyph_hash),
    offsetof(br_glyphs_t, glyph_dollar),
    offsetof(br_glyphs_t, glyph_percent),
    offsetof(br_glyphs_t, glyph_ampersand),
    offsetof(br_glyphs_t, glyph_quote),
    offsetof(br_glyphs_t, glyph_lparen),
    offsetof(br_glyphs_t, glyph_rparen),
    offsetof(br_glyphs_t, glyph_asterisk),
    offsetof(br_glyphs_t, glyph_plus),
    offsetof(br_glyphs_t, glyph_comma),
    offsetof(br_glyphs_t, glyph_minus),
    offsetof(br_glyphs_t, glyph_period),
    offsetof(br_glyphs_t, glyph_slash),
    offsetof(br_glyphs_t, glyph_0),
    offsetof(br_glyphs_t, glyph_1),
    offsetof(br_glyphs_t, glyph_2),
    offsetof(br_glyphs_t, glyph_3),
    offsetof(br_glyphs_t, glyph_4),
    offsetof(br_glyphs_t, glyph_5),
    offsetof(br_glyphs_t, glyph_6),
    offsetof(br_glyphs_t, glyph_7),
    offsetof(br_glyphs_t, glyph_8),
    offsetof(br_glyphs_t, glyph_9),
    offsetof(br_glyphs_t, glyph_colon),
    offsetof(br_glyphs_t, glyph_semicolon),
    offsetof(br_glyphs_t, glyph_langle),
    offsetof(br_glyphs_t, glyph_equal),
    offsetof(br_glyphs_t, glyph_rangle),
    offsetof(br_glyphs_t, glyph_query),
    offsetof(br_glyphs_t, glyph_comat),
    offsetof(br_glyphs_t, glyph_A),
    offsetof(br_glyphs_t, glyph_B),
    offsetof(br_glyphs_t, glyph_C),
    offsetof(br_glyphs_t, glyph_D),
    offsetof(br_glyphs_t, glyph_E),
    offsetof(br_glyphs_t, glyph_F),
    offsetof(br_glyphs_t, glyph_G),
    offsetof(br_glyphs_t, glyph_H),
    offsetof(br_glyphs_t, glyph_I),
    offsetof(br_glyphs_t, glyph_J),
    offsetof(br_glyphs_t, glyph_K),
    offsetof(br_glyphs_t, glyph_L),
    offsetof(br_glyphs_t, glyph_M),
    offsetof(br_glyphs_t, glyph_N),
    offsetof(br_glyphs_t, glyph_O),
    offsetof(br_glyphs_t, glyph_P),
    offsetof(br_glyphs_t, glyph_Q),
    offsetof(br_glyphs_t, glyph_R),
    offsetof(br_glyphs_t, glyph_S),
    offsetof(br_glyphs_t, glyph_T),
    offsetof(br_glyphs_t, glyph_U),
    offsetof(br_glyphs_t, glyph_V),
    offsetof(br_glyphs_t, glyph_W),
    offsetof(br_glyphs_t, glyph_X),
    offsetof(br_glyphs_t, glyph_Y),
    offsetof(br_glyphs_t, glyph_Z),
    offsetof(br_glyphs_t, glyph_lsquare),
    offsetof(br_glyphs_t, glyph_bslash),
    offsetof(br_glyphs_t, glyph_rsquare),
    offsetof(br_glyphs_t, glyph_caret),
    offsetof(br_glyphs_t, glyph_underbar),
    offsetof(br_glyphs_t, glyph_bquote),
    offsetof(br_glyphs_t, glyph_a),
    offsetof(br_glyphs_t, glyph_b),
    offsetof(br_glyphs_t, glyph_c),
    offsetof(br_glyphs_t, glyph_d),
    offsetof(br_glyphs_t, glyph_e),
    offsetof(br_glyphs_t, glyph_f),
    offsetof(br_glyphs_t, glyph_g),
    offsetof(br_glyphs_t, glyph_h),
    offsetof(br_glyphs_t, glyph_i),
    offsetof(br_glyphs_t, glyph_j),
    offsetof(br_glyphs_t, glyph_k),
    offsetof(br_glyphs_t, glyph_l),
    offsetof(br_glyphs_t, glyph_m),
    offsetof(br_glyphs_t, glyph_n),
    offsetof(br_glyphs_t, glyph_o),
    offsetof(br_glyphs_t, glyph_p),
    offsetof(br_glyphs_t, glyph_q),
    offsetof(br_glyphs_t, glyph_r),
    offsetof(br_glyphs_t, glyph_s),
    offsetof(br_glyphs_t, glyph_t),
    offsetof(br_glyphs_t, glyph_u),
    offsetof(br_glyphs_t, glyph_v),
    offsetof(br_glyphs_t, glyph_w),
    offsetof(br_glyphs_t, glyph_x),
    offsetof(br_glyphs_t, glyph_y),
    offsetof(br_glyphs_t, glyph_z),
    offsetof(br_glyphs_t, glyph_lbrace),
    offsetof(br_glyphs_t, glyph_vbar),
    offsetof(br_glyphs_t, glyph_rbrace),
    offsetof(br_glyphs_t, glyph_tilde),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_cursor),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_solidbox),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_tick),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
};

static br_uint_8 widths[] = {
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_bullet,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_uparrow,
    width_downarrow,
    width_rightarrow,
    width_leftarrow,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_exclam,
    width_dquote,
    width_hash,
    width_dollar,
    width_percent,
    width_ampersand,
    width_quote,
    width_lparen,
    width_rparen,
    width_asterisk,
    width_plus,
    width_comma,
    width_minus,
    width_period,
    width_slash,
    width_0,
    width_1,
    width_2,
    width_3,
    width_4,
    width_5,
    width_6,
    width_7,
    width_8,
    width_9,
    width_colon,
    width_semicolon,
    width_langle,
    width_equal,
    width_rangle,
    width_query,
    width_comat,
    width_A,
    width_B,
    width_C,
    width_D,
    width_E,
    width_F,
    width_G,
    width_H,
    width_I,
    width_J,
    width_K,
    width_L,
    width_M,
    width_N,
    width_O,
    width_P,
    width_Q,
    width_R,
    width_S,
    width_T,
    width_U,
    width_V,
    width_W,
    width_X,
    width_Y,
    width_Z,
    width_lsquare,
    width_bslash,
    width_rsquare,
    width_caret,
    width_underbar,
    width_bquote,
    width_a,
    width_b,
    width_c,
    width_d,
    width_e,
    width_f,
    width_g,
    width_h,
    width_i,
    width_j,
    width_k,
    width_l,
    width_m,
    width_n,
    width_o,
    width_p,
    width_q,
    width_r,
    width_s,
    width_t,
    width_u,
    width_v,
    width_w,
    width_x,
    width_y,
    width_z,
    width_lbrace,
    width_vbar,
    width_rbrace,
    width_tilde,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_cursor,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_solidbox,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_space,
    width_tick,
    width_space,
    width_space,
    width_space,
    width_space
};
// clang-format on

br_font _FontProp4x6 = {
    .flags     = BR_FONTF_PROPORTIONAL,
    .glyph_x   = 4,
    .glyph_y   = 6,
    .spacing_x = 5,
    .spacing_y = 7,
    .width     = widths,
    .encoding  = encoding,
    .glyphs    = (br_uint_8 *)&glyphs,
};

#pragma pack(pop)