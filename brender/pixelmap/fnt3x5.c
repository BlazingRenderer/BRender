#include <stddef.h>
#include "brender.h"

#pragma pack(push, 1)

typedef struct {
    br_uint_8 glyph_space[5];
    br_uint_8 glyph_exclam[5];
    br_uint_8 glyph_dquote[5];
    br_uint_8 glyph_pound[5];
    br_uint_8 glyph_hash[5];
    br_uint_8 glyph_percent[5];
    br_uint_8 glyph_ampersand[5];
    br_uint_8 glyph_quote[5];
    br_uint_8 glyph_lparen[5];
    br_uint_8 glyph_rparen[5];
    br_uint_8 glyph_asterisk[5];
    br_uint_8 glyph_plus[5];
    br_uint_8 glyph_comma[5];
    br_uint_8 glyph_minus[5];
    br_uint_8 glyph_period[5];
    br_uint_8 glyph_slash[5];
    br_uint_8 glyph_0[5];
    br_uint_8 glyph_1[5];
    br_uint_8 glyph_2[5];
    br_uint_8 glyph_3[5];
    br_uint_8 glyph_4[5];
    br_uint_8 glyph_5[5];
    br_uint_8 glyph_6[5];
    br_uint_8 glyph_7[5];
    br_uint_8 glyph_8[5];
    br_uint_8 glyph_9[5];
    br_uint_8 glyph_colon[5];
    br_uint_8 glyph_semicolon[5];
    br_uint_8 glyph_langle[5];
    br_uint_8 glyph_equal[5];
    br_uint_8 glyph_rangle[5];
    br_uint_8 glyph_query[5];
    br_uint_8 glyph_comat[5];
    br_uint_8 glyph_A[5];
    br_uint_8 glyph_B[5];
    br_uint_8 glyph_C[5];
    br_uint_8 glyph_D[5];
    br_uint_8 glyph_E[5];
    br_uint_8 glyph_F[5];
    br_uint_8 glyph_G[5];
    br_uint_8 glyph_H[5];
    br_uint_8 glyph_I[5];
    br_uint_8 glyph_J[5];
    br_uint_8 glyph_K[5];
    br_uint_8 glyph_L[5];
    br_uint_8 glyph_M[5];
    br_uint_8 glyph_N[5];
    br_uint_8 glyph_O[5];
    br_uint_8 glyph_P[5];
    br_uint_8 glyph_Q[5];
    br_uint_8 glyph_R[5];
    br_uint_8 glyph_S[5];
    br_uint_8 glyph_T[5];
    br_uint_8 glyph_U[5];
    br_uint_8 glyph_V[5];
    br_uint_8 glyph_W[5];
    br_uint_8 glyph_X[5];
    br_uint_8 glyph_Y[5];
    br_uint_8 glyph_Z[5];
    br_uint_8 glyph_lsquare[5];
    br_uint_8 glyph_bslash[5];
    br_uint_8 glyph_rsquare[5];
    br_uint_8 glyph_caret[5];
    br_uint_8 glyph_underbar[5];
    br_uint_8 glyph_bquote[5];
    br_uint_8 glyph_lbrace[5];
    br_uint_8 glyph_vbar[5];
    br_uint_8 glyph_rbrace[5];
    br_uint_8 glyph_tilde[5];
    br_uint_8 glyph_openbox[5];
    br_uint_8 glyph_alpha[5];
    br_uint_8 glyph_sup2[5];
    br_uint_8 glyph_degree[5];
    br_uint_8 glyph_a[5];
    br_uint_8 glyph_b[5];
    br_uint_8 glyph_c[5];
    br_uint_8 glyph_d[5];
    br_uint_8 glyph_e[5];
    br_uint_8 glyph_f[5];
    br_uint_8 glyph_g[5];
    br_uint_8 glyph_h[5];
    br_uint_8 glyph_i[5];
    br_uint_8 glyph_j[5];
    br_uint_8 glyph_k[5];
    br_uint_8 glyph_l[5];
    br_uint_8 glyph_m[5];
    br_uint_8 glyph_n[5];
    br_uint_8 glyph_o[5];
    br_uint_8 glyph_p[5];
    br_uint_8 glyph_q[5];
    br_uint_8 glyph_r[5];
    br_uint_8 glyph_s[5];
    br_uint_8 glyph_t[5];
    br_uint_8 glyph_u[5];
    br_uint_8 glyph_v[5];
    br_uint_8 glyph_w[5];
    br_uint_8 glyph_x[5];
    br_uint_8 glyph_y[5];
    br_uint_8 glyph_z[5];
    br_uint_8 glyph_ld_tb[5];
    br_uint_8 glyph_ld_tbl[5];
    br_uint_8 glyph_ld_tbL[5];
    br_uint_8 glyph_ld_TBl[5];
    br_uint_8 glyph_ld_Bl[5];
    br_uint_8 glyph_ld_bL[5];
    br_uint_8 glyph_ld_TBL[5];
    br_uint_8 glyph_ld_TB[5];
    br_uint_8 glyph_ld_BL[5];
    br_uint_8 glyph_ld_TL[5];
    br_uint_8 glyph_ld_Tl[5];
    br_uint_8 glyph_ld_tL[5];
    br_uint_8 glyph_ld_bl[5];
    br_uint_8 glyph_ld_tr[5];
    br_uint_8 glyph_ld_tlr[5];
    br_uint_8 glyph_ld_blr[5];
    br_uint_8 glyph_ld_tbr[5];
    br_uint_8 glyph_ld_lr[5];
    br_uint_8 glyph_ld_tblr[5];
    br_uint_8 glyph_ld_tbR[5];
    br_uint_8 glyph_ld_TBr[5];
    br_uint_8 glyph_ld_TR[5];
    br_uint_8 glyph_ld_BR[5];
    br_uint_8 glyph_ld_TLR[5];
    br_uint_8 glyph_ld_BLR[5];
    br_uint_8 glyph_ld_TBR[5];
    br_uint_8 glyph_ld_LR[5];
    br_uint_8 glyph_ld_TBLR[5];
    br_uint_8 glyph_ld_tLR[5];
    br_uint_8 glyph_ld_Tlr[5];
    br_uint_8 glyph_ld_bLR[5];
    br_uint_8 glyph_ld_Blr[5];
    br_uint_8 glyph_ld_Tr[5];
    br_uint_8 glyph_ld_tR[5];
    br_uint_8 glyph_ld_bR[5];
    br_uint_8 glyph_ld_Br[5];
    br_uint_8 glyph_ld_TBlr[5];
    br_uint_8 glyph_ld_tbLR[5];
    br_uint_8 glyph_ld_tl[5];
    br_uint_8 glyph_ld_br[5];
    br_uint_8 glyph_arrow[5];
    br_uint_8 glyph_uparrow[5];
    br_uint_8 glyph_downarrow[5];
    br_uint_8 glyph_leftarrow[5];
    br_uint_8 glyph_rightarrow[5];
    br_uint_8 glyph_tick[5];
    br_uint_8 glyph_bullet[5];
} br_glyphs_t;

static br_glyphs_t glyphs = {
    {0,   0,   0,   0,   0  }, /* glyph_space */
    {64,  64,  64,  0,   64 }, /* glyph_exclam */
    {160, 160, 0,   0,   0  }, /* glyph_dquote */
    {96,  64,  224, 64,  224}, /* glyph_pound */
    {64,  224, 64,  224, 64 }, /* glyph_hash */
    {160, 32,  64,  128, 160}, /* glyph_percent */
    {64,  160, 160, 64,  160}, /* glyph_ampersand */
    {64,  128, 0,   0,   0  }, /* glyph_quote */
    {32,  64,  64,  64,  32 }, /* glyph_lparen */
    {128, 64,  64,  64,  128}, /* glyph_rparen */
    {0,   160, 64,  160, 0  }, /* glyph_asterisk */
    {0,   64,  224, 64,  0  }, /* glyph_plus */
    {0,   0,   0,   64,  128}, /* glyph_comma */
    {0,   0,   224, 0,   0  }, /* glyph_minus */
    {0,   0,   0,   0,   64 }, /* glyph_period */
    {32,  32,  64,  128, 128}, /* glyph_slash */
    {224, 160, 160, 160, 224}, /* glyph_0 */
    {192, 64,  64,  64,  224}, /* glyph_1 */
    {224, 32,  224, 128, 224}, /* glyph_2 */
    {224, 32,  96,  32,  224}, /* glyph_3 */
    {128, 128, 160, 224, 32 }, /* glyph_4 */
    {224, 128, 224, 32,  224}, /* glyph_5 */
    {224, 128, 224, 160, 224}, /* glyph_6 */
    {224, 32,  32,  32,  32 }, /* glyph_7 */
    {224, 160, 224, 160, 224}, /* glyph_8 */
    {224, 160, 224, 32,  224}, /* glyph_9 */
    {0,   64,  0,   64,  0  }, /* glyph_colon */
    {0,   64,  0,   64,  128}, /* glyph_semicolon */
    {32,  64,  128, 64,  32 }, /* glyph_langle */
    {0,   224, 0,   224, 0  }, /* glyph_equal */
    {128, 64,  32,  64,  128}, /* glyph_rangle */
    {224, 32,  96,  0,   64 }, /* glyph_query */
    {224, 160, 160, 128, 224}, /* glyph_comat */
    {64,  160, 160, 224, 160}, /* glyph_A */
    {192, 160, 192, 160, 192}, /* glyph_B */
    {96,  128, 128, 128, 96 }, /* glyph_C */
    {192, 160, 160, 160, 192}, /* glyph_D */
    {224, 128, 192, 128, 224}, /* glyph_E */
    {224, 128, 192, 128, 128}, /* glyph_F */
    {96,  128, 160, 160, 96 }, /* glyph_G */
    {160, 160, 224, 160, 160}, /* glyph_H */
    {224, 64,  64,  64,  224}, /* glyph_I */
    {224, 64,  64,  64,  192}, /* glyph_J */
    {160, 224, 192, 224, 160}, /* glyph_K */
    {128, 128, 128, 128, 224}, /* glyph_L */
    {160, 224, 224, 160, 160}, /* glyph_M */
    {160, 224, 224, 224, 160}, /* glyph_N */
    {64,  160, 160, 160, 64 }, /* glyph_O */
    {192, 160, 192, 128, 128}, /* glyph_P */
    {64,  160, 160, 160, 96 }, /* glyph_Q */
    {192, 160, 192, 160, 160}, /* glyph_R */
    {96,  128, 64,  32,  192}, /* glyph_S */
    {224, 64,  64,  64,  64 }, /* glyph_T */
    {160, 160, 160, 160, 224}, /* glyph_U */
    {160, 160, 160, 160, 64 }, /* glyph_V */
    {160, 160, 224, 224, 160}, /* glyph_W */
    {160, 160, 64,  160, 160}, /* glyph_X */
    {160, 160, 224, 64,  64 }, /* glyph_Y */
    {224, 32,  64,  128, 224}, /* glyph_Z */
    {224, 128, 128, 128, 224}, /* glyph_lsquare */
    {128, 128, 64,  32,  32 }, /* glyph_bslash */
    {224, 32,  32,  32,  224}, /* glyph_rsquare */
    {64,  160, 0,   0,   0  }, /* glyph_caret */
    {0,   0,   0,   0,   224}, /* glyph_underbar */
    {64,  32,  0,   0,   0  }, /* glyph_bquote */
    {96,  64,  192, 64,  96 }, /* glyph_lbrace */
    {128, 128, 128, 128, 128}, /* glyph_vbar */
    {192, 64,  96,  64,  192}, /* glyph_rbrace */
    {80,  160, 0,   0,   0  }, /* glyph_tilde */
    {240, 144, 144, 144, 240}, /* glyph_openbox */
    {0,   80,  160, 160, 80 }, /* glyph_alpha */
    {192, 32,  64,  224, 0  }, /* glyph_sup2 */
    {224, 160, 224, 0,   0  }, /* glyph_degree */
    {0,   64,  160, 224, 160}, /* glyph_a */
    {0,   192, 192, 160, 192}, /* glyph_b */
    {0,   96,  128, 128, 96 }, /* glyph_c */
    {0,   192, 160, 160, 192}, /* glyph_d */
    {0,   224, 192, 128, 224}, /* glyph_e */
    {0,   224, 128, 192, 128}, /* glyph_f */
    {0,   96,  128, 160, 96 }, /* glyph_g */
    {0,   160, 160, 224, 160}, /* glyph_h */
    {0,   224, 64,  64,  224}, /* glyph_i */
    {0,   224, 64,  64,  192}, /* glyph_j */
    {0,   160, 192, 192, 160}, /* glyph_k */
    {0,   128, 128, 128, 224}, /* glyph_l */
    {0,   160, 224, 160, 160}, /* glyph_m */
    {0,   160, 224, 224, 160}, /* glyph_n */
    {0,   64,  160, 160, 64 }, /* glyph_o */
    {0,   192, 160, 192, 128}, /* glyph_p */
    {0,   64,  160, 160, 96 }, /* glyph_q */
    {0,   192, 160, 192, 160}, /* glyph_r */
    {0,   96,  128, 32,  192}, /* glyph_s */
    {0,   224, 64,  64,  64 }, /* glyph_t */
    {0,   160, 160, 160, 224}, /* glyph_u */
    {0,   160, 160, 160, 64 }, /* glyph_v */
    {0,   160, 160, 224, 160}, /* glyph_w */
    {0,   160, 64,  64,  160}, /* glyph_x */
    {0,   160, 160, 64,  64 }, /* glyph_y */
    {0,   224, 32,  192, 224}, /* glyph_z */
    {64,  64,  64,  64,  64 }, /* glyph_ld_tb */
    {64,  64,  192, 64,  64 }, /* glyph_ld_tbl */
    {64,  192, 64,  192, 64 }, /* glyph_ld_tbL */
    {96,  96,  224, 96,  96 }, /* glyph_ld_TBl */
    {0,   0,   224, 96,  96 }, /* glyph_ld_Bl */
    {0,   224, 32,  224, 32 }, /* glyph_ld_bL */
    {96,  224, 96,  224, 96 }, /* glyph_ld_TBL */
    {96,  96,  96,  96,  96 }, /* glyph_ld_TB */
    {0,   112, 96,  112, 96 }, /* glyph_ld_BL */
    {96,  112, 96,  112, 0  }, /* glyph_ld_TL */
    {96,  96,  224, 0,   0  }, /* glyph_ld_Tl */
    {64,  192, 64,  192, 0  }, /* glyph_ld_tL */
    {0,   0,   192, 64,  64 }, /* glyph_ld_bl */
    {64,  64,  112, 0,   0  }, /* glyph_ld_tr */
    {64,  64,  240, 0,   0  }, /* glyph_ld_tlr */
    {0,   0,   240, 64,  64 }, /* glyph_ld_blr */
    {0,   0,   0,   0,   0  }, /* glyph_ld_tbr */
    {0,   0,   240, 0,   0  }, /* glyph_ld_lr */
    {64,  64,  240, 64,  64 }, /* glyph_ld_tblr */
    {64,  112, 64,  112, 64 }, /* glyph_ld_tbR */
    {96,  96,  112, 96,  96 }, /* glyph_ld_TBr */
    {96,  112, 96,  112, 0  }, /* glyph_ld_TR */
    {0,   112, 96,  112, 96 }, /* glyph_ld_BR */
    {96,  240, 96,  240, 0  }, /* glyph_ld_TLR */
    {0,   240, 96,  240, 96 }, /* glyph_ld_BLR */
    {96,  112, 96,  112, 96 }, /* glyph_ld_TBR */
    {0,   240, 0,   240, 0  }, /* glyph_ld_LR */
    {96,  240, 96,  240, 96 }, /* glyph_ld_TBLR */
    {64,  240, 64,  240, 0  }, /* glyph_ld_tLR */
    {96,  96,  240, 0,   0  }, /* glyph_ld_Tlr */
    {0,   240, 64,  240, 64 }, /* glyph_ld_bLR */
    {0,   0,   240, 96,  96 }, /* glyph_ld_Blr */
    {96,  96,  112, 0,   0  }, /* glyph_ld_Tr */
    {64,  112, 64,  112, 0  }, /* glyph_ld_tR */
    {0,   112, 64,  112, 64 }, /* glyph_ld_bR */
    {0,   0,   112, 96,  96 }, /* glyph_ld_Br */
    {96,  96,  240, 96,  96 }, /* glyph_ld_TBlr */
    {64,  240, 64,  240, 64 }, /* glyph_ld_tbLR */
    {64,  64,  192, 0,   0  }, /* glyph_ld_tl */
    {0,   0,   112, 64,  64 }, /* glyph_ld_br */
    {248, 240, 240, 200, 132}, /* glyph_arrow */
    {64,  224, 0,   0,   0  }, /* glyph_uparrow */
    {0,   0,   0,   224, 64 }, /* glyph_downarrow */
    {0,   64,  192, 64,  0  }, /* glyph_leftarrow */
    {0,   64,  96,  64,  0  }, /* glyph_rightarrow */
    {0,   96,  64,  192, 64 }, /* glyph_tick */
    {0,   0,   64,  0,   0  }, /* glyph_bullet */
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
    offsetof(br_glyphs_t, glyph_uparrow),
    offsetof(br_glyphs_t, glyph_downarrow),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_exclam),
    offsetof(br_glyphs_t, glyph_dquote),
    offsetof(br_glyphs_t, glyph_hash),
    offsetof(br_glyphs_t, glyph_pound),
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
    offsetof(br_glyphs_t, glyph_arrow),
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
    offsetof(br_glyphs_t, glyph_ld_tb),
    offsetof(br_glyphs_t, glyph_ld_tbl),
    offsetof(br_glyphs_t, glyph_ld_tbL),
    offsetof(br_glyphs_t, glyph_ld_TBl),
    offsetof(br_glyphs_t, glyph_ld_Bl),
    offsetof(br_glyphs_t, glyph_ld_bL),
    offsetof(br_glyphs_t, glyph_ld_TBL),
    offsetof(br_glyphs_t, glyph_ld_TB),
    offsetof(br_glyphs_t, glyph_ld_BL),
    offsetof(br_glyphs_t, glyph_ld_TL),
    offsetof(br_glyphs_t, glyph_ld_Tl),
    offsetof(br_glyphs_t, glyph_ld_tL),
    offsetof(br_glyphs_t, glyph_ld_bl),
    offsetof(br_glyphs_t, glyph_ld_tr),
    offsetof(br_glyphs_t, glyph_ld_tlr),
    offsetof(br_glyphs_t, glyph_ld_blr),
    offsetof(br_glyphs_t, glyph_ld_tbr),
    offsetof(br_glyphs_t, glyph_ld_lr),
    offsetof(br_glyphs_t, glyph_ld_tblr),
    offsetof(br_glyphs_t, glyph_ld_tbR),
    offsetof(br_glyphs_t, glyph_ld_TBr),
    offsetof(br_glyphs_t, glyph_ld_TR),
    offsetof(br_glyphs_t, glyph_ld_BR),
    offsetof(br_glyphs_t, glyph_ld_TLR),
    offsetof(br_glyphs_t, glyph_ld_BLR),
    offsetof(br_glyphs_t, glyph_ld_TBR),
    offsetof(br_glyphs_t, glyph_ld_LR),
    offsetof(br_glyphs_t, glyph_ld_TBLR),
    offsetof(br_glyphs_t, glyph_ld_tLR),
    offsetof(br_glyphs_t, glyph_ld_Tlr),
    offsetof(br_glyphs_t, glyph_ld_bLR),
    offsetof(br_glyphs_t, glyph_ld_Blr),
    offsetof(br_glyphs_t, glyph_ld_Tr),
    offsetof(br_glyphs_t, glyph_ld_tR),
    offsetof(br_glyphs_t, glyph_ld_bR),
    offsetof(br_glyphs_t, glyph_ld_Br),
    offsetof(br_glyphs_t, glyph_ld_TBlr),
    offsetof(br_glyphs_t, glyph_ld_tbLR),
    offsetof(br_glyphs_t, glyph_ld_tl),
    offsetof(br_glyphs_t, glyph_ld_br),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_alpha),
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
    offsetof(br_glyphs_t, glyph_degree),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_tick),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_sup2),
    offsetof(br_glyphs_t, glyph_space),
    offsetof(br_glyphs_t, glyph_space),
};
// clang-format on

br_font _FontFixed3x5 = {
    .flags     = 0,
    .glyph_x   = 3,
    .glyph_y   = 5,
    .spacing_x = 4,
    .spacing_y = 6,
    .width     = NULL,
    .encoding  = encoding,
    .glyphs    = (br_uint_8 *)&glyphs,
};

#pragma pack(pop)
