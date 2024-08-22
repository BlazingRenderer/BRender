/*
 * Automatically generated from fntp4x6.asm, do not edit.
 */
#include <brender.h>

enum {
    width_space = 2,
    width_exclam = 1,
    width_dquote = 3,
    width_hash = 5,
    width_dollar = 3,
    width_percent = 5,
    width_ampersand = 3,
    width_quote = 2,
    width_lparen = 2,
    width_rparen = 2,
    width_asterisk = 3,
    width_plus = 3,
    width_comma = 2,
    width_minus = 3,
    width_period = 2,
    width_slash = 5,
    width_0 = 4,
    width_1 = 4,
    width_2 = 4,
    width_3 = 4,
    width_4 = 4,
    width_5 = 4,
    width_6 = 4,
    width_7 = 4,
    width_8 = 4,
    width_9 = 4,
    width_colon = 1,
    width_semicolon = 2,
    width_langle = 3,
    width_equal = 3,
    width_rangle = 3,
    width_query = 3,
    width_comat = 4,
    width_A = 4,
    width_B = 4,
    width_C = 4,
    width_D = 4,
    width_E = 4,
    width_F = 4,
    width_G = 4,
    width_H = 4,
    width_I = 1,
    width_J = 4,
    width_K = 4,
    width_L = 4,
    width_M = 5,
    width_N = 4,
    width_O = 4,
    width_P = 4,
    width_Q = 4,
    width_R = 4,
    width_S = 4,
    width_T = 3,
    width_U = 4,
    width_V = 5,
    width_W = 5,
    width_X = 5,
    width_Y = 5,
    width_Z = 5,
    width_lsquare = 2,
    width_bslash = 5,
    width_rsquare = 2,
    width_caret = 3,
    width_underbar = 4,
    width_bquote = 2,
    width_a = 3,
    width_b = 3,
    width_c = 3,
    width_d = 3,
    width_e = 3,
    width_f = 2,
    width_g = 3,
    width_h = 3,
    width_i = 1,
    width_j = 2,
    width_k = 3,
    width_l = 1,
    width_m = 5,
    width_n = 3,
    width_o = 3,
    width_p = 3,
    width_q = 3,
    width_r = 3,
    width_s = 3,
    width_t = 3,
    width_u = 3,
    width_v = 3,
    width_w = 5,
    width_x = 3,
    width_y = 3,
    width_z = 3,
    width_lbrace = 3,
    width_vbar = 1,
    width_rbrace = 3,
    width_tilde = 4,
    width_bullet = 3,
    width_cross = 3,
    width_tick = 5,
    width_uparrow = 3,
    width_downarrow = 3,
    width_rightarrow = 4,
    width_leftarrow = 4,
    width_solidbox = 4,
    width_cursor = 3,
};

const static br_int_8 widths[256] = {
    [0] = width_space,
    [1] = width_space,
    [2] = width_space,
    [3] = width_space,
    [4] = width_space,
    [5] = width_space,
    [6] = width_space,
    [7] = width_bullet,
    [8] = width_space,
    [9] = width_space,
    [10] = width_space,
    [11] = width_space,
    [12] = width_space,
    [13] = width_space,
    [14] = width_space,
    [15] = width_space,
    [16] = width_space,
    [17] = width_space,
    [18] = width_space,
    [19] = width_space,
    [20] = width_space,
    [21] = width_space,
    [22] = width_space,
    [23] = width_space,
    [24] = width_uparrow,
    [25] = width_downarrow,
    [26] = width_rightarrow,
    [27] = width_leftarrow,
    [28] = width_space,
    [29] = width_space,
    [30] = width_space,
    [31] = width_space,
    [32] = width_space,
    [33] = width_exclam,
    [34] = width_dquote,
    [35] = width_hash,
    [36] = width_dollar,
    [37] = width_percent,
    [38] = width_ampersand,
    [39] = width_quote,
    [40] = width_lparen,
    [41] = width_rparen,
    [42] = width_asterisk,
    [43] = width_plus,
    [44] = width_comma,
    [45] = width_minus,
    [46] = width_period,
    [47] = width_slash,
    [48] = width_0,
    [49] = width_1,
    [50] = width_2,
    [51] = width_3,
    [52] = width_4,
    [53] = width_5,
    [54] = width_6,
    [55] = width_7,
    [56] = width_8,
    [57] = width_9,
    [58] = width_colon,
    [59] = width_semicolon,
    [60] = width_langle,
    [61] = width_equal,
    [62] = width_rangle,
    [63] = width_query,
    [64] = width_comat,
    [65] = width_A,
    [66] = width_B,
    [67] = width_C,
    [68] = width_D,
    [69] = width_E,
    [70] = width_F,
    [71] = width_G,
    [72] = width_H,
    [73] = width_I,
    [74] = width_J,
    [75] = width_K,
    [76] = width_L,
    [77] = width_M,
    [78] = width_N,
    [79] = width_O,
    [80] = width_P,
    [81] = width_Q,
    [82] = width_R,
    [83] = width_S,
    [84] = width_T,
    [85] = width_U,
    [86] = width_V,
    [87] = width_W,
    [88] = width_X,
    [89] = width_Y,
    [90] = width_Z,
    [91] = width_lsquare,
    [92] = width_bslash,
    [93] = width_rsquare,
    [94] = width_caret,
    [95] = width_underbar,
    [96] = width_bquote,
    [97] = width_a,
    [98] = width_b,
    [99] = width_c,
    [100] = width_d,
    [101] = width_e,
    [102] = width_f,
    [103] = width_g,
    [104] = width_h,
    [105] = width_i,
    [106] = width_j,
    [107] = width_k,
    [108] = width_l,
    [109] = width_m,
    [110] = width_n,
    [111] = width_o,
    [112] = width_p,
    [113] = width_q,
    [114] = width_r,
    [115] = width_s,
    [116] = width_t,
    [117] = width_u,
    [118] = width_v,
    [119] = width_w,
    [120] = width_x,
    [121] = width_y,
    [122] = width_z,
    [123] = width_lbrace,
    [124] = width_vbar,
    [125] = width_rbrace,
    [126] = width_tilde,
    [127] = width_space,
    [128] = width_space,
    [129] = width_space,
    [130] = width_space,
    [131] = width_space,
    [132] = width_space,
    [133] = width_space,
    [134] = width_space,
    [135] = width_space,
    [136] = width_space,
    [137] = width_space,
    [138] = width_space,
    [139] = width_space,
    [140] = width_space,
    [141] = width_space,
    [142] = width_space,
    [143] = width_space,
    [144] = width_space,
    [145] = width_space,
    [146] = width_space,
    [147] = width_space,
    [148] = width_space,
    [149] = width_space,
    [150] = width_space,
    [151] = width_space,
    [152] = width_space,
    [153] = width_space,
    [154] = width_space,
    [155] = width_space,
    [156] = width_space,
    [157] = width_space,
    [158] = width_space,
    [159] = width_space,
    [160] = width_space,
    [161] = width_space,
    [162] = width_space,
    [163] = width_space,
    [164] = width_space,
    [165] = width_space,
    [166] = width_space,
    [167] = width_space,
    [168] = width_space,
    [169] = width_space,
    [170] = width_space,
    [171] = width_space,
    [172] = width_space,
    [173] = width_space,
    [174] = width_space,
    [175] = width_space,
    [176] = width_space,
    [177] = width_space,
    [178] = width_space,
    [179] = width_cursor,
    [180] = width_space,
    [181] = width_space,
    [182] = width_space,
    [183] = width_space,
    [184] = width_space,
    [185] = width_space,
    [186] = width_space,
    [187] = width_space,
    [188] = width_space,
    [189] = width_space,
    [190] = width_space,
    [191] = width_space,
    [192] = width_space,
    [193] = width_space,
    [194] = width_space,
    [195] = width_space,
    [196] = width_space,
    [197] = width_space,
    [198] = width_space,
    [199] = width_space,
    [200] = width_space,
    [201] = width_space,
    [202] = width_space,
    [203] = width_space,
    [204] = width_space,
    [205] = width_space,
    [206] = width_space,
    [207] = width_space,
    [208] = width_space,
    [209] = width_space,
    [210] = width_space,
    [211] = width_space,
    [212] = width_space,
    [213] = width_space,
    [214] = width_space,
    [215] = width_space,
    [216] = width_space,
    [217] = width_space,
    [218] = width_space,
    [219] = width_solidbox,
    [220] = width_space,
    [221] = width_space,
    [222] = width_space,
    [223] = width_space,
    [224] = width_space,
    [225] = width_space,
    [226] = width_space,
    [227] = width_space,
    [228] = width_space,
    [229] = width_space,
    [230] = width_space,
    [231] = width_space,
    [232] = width_space,
    [233] = width_space,
    [234] = width_space,
    [235] = width_space,
    [236] = width_space,
    [237] = width_space,
    [238] = width_space,
    [239] = width_space,
    [240] = width_space,
    [241] = width_space,
    [242] = width_space,
    [243] = width_space,
    [244] = width_space,
    [245] = width_space,
    [246] = width_space,
    [247] = width_space,
    [248] = width_space,
    [249] = width_space,
    [250] = width_space,
    [251] = width_tick,
    [252] = width_space,
    [253] = width_space,
    [254] = width_space,
    [255] = width_space,
};

const static br_uint_8 glyph_space[6] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_exclam[6] = {
    0b10000000,
    0b10000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_dquote[6] = {
    0b10100000,
    0b10100000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_hash[6] = {
    0b01010000,
    0b11111000,
    0b01010000,
    0b11111000,
    0b01010000,
    0b00000000,
};

const static br_uint_8 glyph_dollar[6] = {
    0b01000000,
    0b01100000,
    0b11000000,
    0b00100000,
    0b11000000,
    0b01000000,
};

const static br_uint_8 glyph_percent[6] = {
    0b11001000,
    0b11010000,
    0b00100000,
    0b01011000,
    0b10011000,
    0b00000000,
};

const static br_uint_8 glyph_ampersand[6] = {
    0b01000000,
    0b10100000,
    0b10100000,
    0b01000000,
    0b10100000,
    0b00000000,
};

const static br_uint_8 glyph_quote[6] = {
    0b01000000,
    0b10000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_lparen[6] = {
    0b01000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_rparen[6] = {
    0b10000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_asterisk[6] = {
    0b00000000,
    0b10100000,
    0b01000000,
    0b10100000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_plus[6] = {
    0b00000000,
    0b01000000,
    0b11100000,
    0b01000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_comma[6] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b01000000,
    0b10000000,
};

const static br_uint_8 glyph_minus[6] = {
    0b00000000,
    0b00000000,
    0b11100000,
    0b00000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_period[6] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_slash[6] = {
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_0[6] = {
    0b01100000,
    0b10010000,
    0b10010000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_1[6] = {
    0b00100000,
    0b01100000,
    0b00100000,
    0b00100000,
    0b01110000,
    0b00000000,
};

const static br_uint_8 glyph_2[6] = {
    0b11100000,
    0b00010000,
    0b01100000,
    0b10000000,
    0b11110000,
    0b00000000,
};

const static br_uint_8 glyph_3[6] = {
    0b11100000,
    0b00010000,
    0b01100000,
    0b00010000,
    0b11100000,
    0b00000000,
};

const static br_uint_8 glyph_4[6] = {
    0b10000000,
    0b10000000,
    0b10100000,
    0b11110000,
    0b00100000,
    0b00000000,
};

const static br_uint_8 glyph_5[6] = {
    0b11110000,
    0b10000000,
    0b11100000,
    0b00010000,
    0b11100000,
    0b00000000,
};

const static br_uint_8 glyph_6[6] = {
    0b01100000,
    0b10000000,
    0b11100000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_7[6] = {
    0b11110000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_8[6] = {
    0b01100000,
    0b10010000,
    0b01100000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_9[6] = {
    0b01100000,
    0b10010000,
    0b01110000,
    0b00010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_colon[6] = {
    0b00000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_semicolon[6] = {
    0b00000000,
    0b01000000,
    0b00000000,
    0b01000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_langle[6] = {
    0b00100000,
    0b01000000,
    0b10000000,
    0b01000000,
    0b00100000,
    0b00000000,
};

const static br_uint_8 glyph_equal[6] = {
    0b00000000,
    0b11100000,
    0b00000000,
    0b11100000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_rangle[6] = {
    0b10000000,
    0b01000000,
    0b00100000,
    0b01000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_query[6] = {
    0b11000000,
    0b00100000,
    0b01000000,
    0b00000000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_comat[6] = {
    0b01100000,
    0b10110000,
    0b10110000,
    0b10000000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_A[6] = {
    0b01100000,
    0b10010000,
    0b11110000,
    0b10010000,
    0b10010000,
    0b00000000,
};

const static br_uint_8 glyph_B[6] = {
    0b11100000,
    0b10010000,
    0b11100000,
    0b10010000,
    0b11100000,
    0b00000000,
};

const static br_uint_8 glyph_C[6] = {
    0b01100000,
    0b10010000,
    0b10000000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_D[6] = {
    0b11100000,
    0b10010000,
    0b10010000,
    0b10010000,
    0b11100000,
    0b00000000,
};

const static br_uint_8 glyph_E[6] = {
    0b11110000,
    0b10000000,
    0b11100000,
    0b10000000,
    0b11110000,
    0b00000000,
};

const static br_uint_8 glyph_F[6] = {
    0b11110000,
    0b10000000,
    0b11100000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_G[6] = {
    0b01100000,
    0b10000000,
    0b10110000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_H[6] = {
    0b10010000,
    0b10010000,
    0b11110000,
    0b10010000,
    0b10010000,
    0b00000000,
};

const static br_uint_8 glyph_I[6] = {
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_J[6] = {
    0b00010000,
    0b00010000,
    0b00010000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_K[6] = {
    0b10010000,
    0b10100000,
    0b11000000,
    0b10100000,
    0b10010000,
    0b00000000,
};

const static br_uint_8 glyph_L[6] = {
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b11110000,
    0b00000000,
};

const static br_uint_8 glyph_M[6] = {
    0b10001000,
    0b11011000,
    0b10101000,
    0b10001000,
    0b10001000,
    0b00000000,
};

const static br_uint_8 glyph_N[6] = {
    0b10010000,
    0b11010000,
    0b10110000,
    0b10010000,
    0b10010000,
    0b00000000,
};

const static br_uint_8 glyph_O[6] = {
    0b01100000,
    0b10010000,
    0b10010000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_P[6] = {
    0b11100000,
    0b10010000,
    0b11100000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_Q[6] = {
    0b01100000,
    0b10010000,
    0b10010000,
    0b10110000,
    0b01110000,
    0b00000000,
};

const static br_uint_8 glyph_R[6] = {
    0b11100000,
    0b10010000,
    0b11100000,
    0b10010000,
    0b10010000,
    0b00000000,
};

const static br_uint_8 glyph_S[6] = {
    0b01110000,
    0b10000000,
    0b01100000,
    0b00010000,
    0b11100000,
    0b00000000,
};

const static br_uint_8 glyph_T[6] = {
    0b11100000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_U[6] = {
    0b10010000,
    0b10010000,
    0b10010000,
    0b10010000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_V[6] = {
    0b10001000,
    0b10001000,
    0b01010000,
    0b01010000,
    0b00100000,
    0b00000000,
};

const static br_uint_8 glyph_W[6] = {
    0b10001000,
    0b10001000,
    0b10101000,
    0b10101000,
    0b01010000,
    0b00000000,
};

const static br_uint_8 glyph_X[6] = {
    0b10001000,
    0b01010000,
    0b00100000,
    0b01010000,
    0b10001000,
    0b00000000,
};

const static br_uint_8 glyph_Y[6] = {
    0b10001000,
    0b01010000,
    0b00100000,
    0b00100000,
    0b00100000,
    0b00000000,
};

const static br_uint_8 glyph_Z[6] = {
    0b11111000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b11111000,
    0b00000000,
};

const static br_uint_8 glyph_lsquare[6] = {
    0b11000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b11000000,
    0b00000000,
};

const static br_uint_8 glyph_bslash[6] = {
    0b10000000,
    0b01000000,
    0b00100000,
    0b00010000,
    0b00001000,
    0b00000000,
};

const static br_uint_8 glyph_rsquare[6] = {
    0b11000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b11000000,
    0b00000000,
};

const static br_uint_8 glyph_caret[6] = {
    0b01000000,
    0b10100000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_underbar[6] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b11110000,
};

const static br_uint_8 glyph_bquote[6] = {
    0b10000000,
    0b01000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_a[6] = {
    0b00000000,
    0b01100000,
    0b10100000,
    0b10100000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_b[6] = {
    0b10000000,
    0b11000000,
    0b10100000,
    0b10100000,
    0b11000000,
    0b00000000,
};

const static br_uint_8 glyph_c[6] = {
    0b00000000,
    0b01100000,
    0b10000000,
    0b10000000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_d[6] = {
    0b00100000,
    0b01100000,
    0b10100000,
    0b10100000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_e[6] = {
    0b00000000,
    0b01000000,
    0b10100000,
    0b11000000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_f[6] = {
    0b01000000,
    0b10000000,
    0b11000000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_g[6] = {
    0b00000000,
    0b01100000,
    0b10100000,
    0b01100000,
    0b00100000,
    0b01000000,
};

const static br_uint_8 glyph_h[6] = {
    0b10000000,
    0b11000000,
    0b10100000,
    0b10100000,
    0b10100000,
    0b00000000,
};

const static br_uint_8 glyph_i[6] = {
    0b10000000,
    0b00000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_j[6] = {
    0b01000000,
    0b00000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b10000000,
};

const static br_uint_8 glyph_k[6] = {
    0b10000000,
    0b10100000,
    0b11000000,
    0b11000000,
    0b10100000,
    0b00000000,
};

const static br_uint_8 glyph_l[6] = {
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_m[6] = {
    0b00000000,
    0b11010000,
    0b10101000,
    0b10101000,
    0b10101000,
    0b00000000,
};

const static br_uint_8 glyph_n[6] = {
    0b00000000,
    0b11000000,
    0b10100000,
    0b10100000,
    0b10100000,
    0b00000000,
};

const static br_uint_8 glyph_o[6] = {
    0b00000000,
    0b01000000,
    0b10100000,
    0b10100000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_p[6] = {
    0b00000000,
    0b11000000,
    0b10100000,
    0b10100000,
    0b11000000,
    0b10000000,
};

const static br_uint_8 glyph_q[6] = {
    0b00000000,
    0b01100000,
    0b10100000,
    0b10100000,
    0b01100000,
    0b00100000,
};

const static br_uint_8 glyph_r[6] = {
    0b00000000,
    0b01100000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_s[6] = {
    0b00000000,
    0b01100000,
    0b11000000,
    0b00100000,
    0b11000000,
    0b00000000,
};

const static br_uint_8 glyph_t[6] = {
    0b01000000,
    0b11100000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_u[6] = {
    0b00000000,
    0b10100000,
    0b10100000,
    0b10100000,
    0b01100000,
    0b00000000,
};

const static br_uint_8 glyph_v[6] = {
    0b00000000,
    0b10100000,
    0b10100000,
    0b01000000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_w[6] = {
    0b00000000,
    0b10101000,
    0b10101000,
    0b10101000,
    0b01010000,
    0b00000000,
};

const static br_uint_8 glyph_x[6] = {
    0b00000000,
    0b10100000,
    0b01000000,
    0b01000000,
    0b10100000,
    0b00000000,
};

const static br_uint_8 glyph_y[6] = {
    0b00000000,
    0b10100000,
    0b10100000,
    0b01100000,
    0b00100000,
    0b01000000,
};

const static br_uint_8 glyph_z[6] = {
    0b00000000,
    0b11100000,
    0b00100000,
    0b11000000,
    0b11100000,
    0b00000000,
};

const static br_uint_8 glyph_lbrace[6] = {
    0b00100000,
    0b01000000,
    0b11000000,
    0b01000000,
    0b00100000,
    0b00000000,
};

const static br_uint_8 glyph_vbar[6] = {
    0b10000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_rbrace[6] = {
    0b10000000,
    0b01000000,
    0b01100000,
    0b01000000,
    0b10000000,
    0b00000000,
};

const static br_uint_8 glyph_tilde[6] = {
    0b01010000,
    0b10100000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_bullet[6] = {
    0b00000000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_cross[6] = {
    0b00000000,
    0b10100000,
    0b01000000,
    0b10100000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_tick[6] = {
    0b00000000,
    0b00001000,
    0b00010000,
    0b10100000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_uparrow[6] = {
    0b00000000,
    0b01000000,
    0b11100000,
    0b01000000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_downarrow[6] = {
    0b00000000,
    0b01000000,
    0b01000000,
    0b11100000,
    0b01000000,
    0b00000000,
};

const static br_uint_8 glyph_rightarrow[6] = {
    0b00000000,
    0b00100000,
    0b11110000,
    0b00100000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_leftarrow[6] = {
    0b00000000,
    0b01000000,
    0b11110000,
    0b01000000,
    0b00000000,
    0b00000000,
};

const static br_uint_8 glyph_solidbox[6] = {
    0b11110000,
    0b11110000,
    0b11110000,
    0b11110000,
    0b11110000,
    0b11110000,
};

const static br_uint_8 glyph_cursor[6] = {
    0b10100000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b10100000,
};

const static br_uint_8 *encodings[256] = {
    [0] = glyph_space,
    [1] = glyph_space,
    [2] = glyph_space,
    [3] = glyph_space,
    [4] = glyph_space,
    [5] = glyph_space,
    [6] = glyph_space,
    [7] = glyph_bullet,
    [8] = glyph_space,
    [9] = glyph_space,
    [10] = glyph_space,
    [11] = glyph_space,
    [12] = glyph_space,
    [13] = glyph_space,
    [14] = glyph_space,
    [15] = glyph_space,
    [16] = glyph_space,
    [17] = glyph_space,
    [18] = glyph_space,
    [19] = glyph_space,
    [20] = glyph_space,
    [21] = glyph_space,
    [22] = glyph_space,
    [23] = glyph_space,
    [24] = glyph_uparrow,
    [25] = glyph_downarrow,
    [26] = glyph_rightarrow,
    [27] = glyph_leftarrow,
    [28] = glyph_space,
    [29] = glyph_space,
    [30] = glyph_space,
    [31] = glyph_space,
    [32] = glyph_space,
    [33] = glyph_exclam,
    [34] = glyph_dquote,
    [35] = glyph_hash,
    [36] = glyph_dollar,
    [37] = glyph_percent,
    [38] = glyph_ampersand,
    [39] = glyph_quote,
    [40] = glyph_lparen,
    [41] = glyph_rparen,
    [42] = glyph_asterisk,
    [43] = glyph_plus,
    [44] = glyph_comma,
    [45] = glyph_minus,
    [46] = glyph_period,
    [47] = glyph_slash,
    [48] = glyph_0,
    [49] = glyph_1,
    [50] = glyph_2,
    [51] = glyph_3,
    [52] = glyph_4,
    [53] = glyph_5,
    [54] = glyph_6,
    [55] = glyph_7,
    [56] = glyph_8,
    [57] = glyph_9,
    [58] = glyph_colon,
    [59] = glyph_semicolon,
    [60] = glyph_langle,
    [61] = glyph_equal,
    [62] = glyph_rangle,
    [63] = glyph_query,
    [64] = glyph_comat,
    [65] = glyph_A,
    [66] = glyph_B,
    [67] = glyph_C,
    [68] = glyph_D,
    [69] = glyph_E,
    [70] = glyph_F,
    [71] = glyph_G,
    [72] = glyph_H,
    [73] = glyph_I,
    [74] = glyph_J,
    [75] = glyph_K,
    [76] = glyph_L,
    [77] = glyph_M,
    [78] = glyph_N,
    [79] = glyph_O,
    [80] = glyph_P,
    [81] = glyph_Q,
    [82] = glyph_R,
    [83] = glyph_S,
    [84] = glyph_T,
    [85] = glyph_U,
    [86] = glyph_V,
    [87] = glyph_W,
    [88] = glyph_X,
    [89] = glyph_Y,
    [90] = glyph_Z,
    [91] = glyph_lsquare,
    [92] = glyph_bslash,
    [93] = glyph_rsquare,
    [94] = glyph_caret,
    [95] = glyph_underbar,
    [96] = glyph_bquote,
    [97] = glyph_a,
    [98] = glyph_b,
    [99] = glyph_c,
    [100] = glyph_d,
    [101] = glyph_e,
    [102] = glyph_f,
    [103] = glyph_g,
    [104] = glyph_h,
    [105] = glyph_i,
    [106] = glyph_j,
    [107] = glyph_k,
    [108] = glyph_l,
    [109] = glyph_m,
    [110] = glyph_n,
    [111] = glyph_o,
    [112] = glyph_p,
    [113] = glyph_q,
    [114] = glyph_r,
    [115] = glyph_s,
    [116] = glyph_t,
    [117] = glyph_u,
    [118] = glyph_v,
    [119] = glyph_w,
    [120] = glyph_x,
    [121] = glyph_y,
    [122] = glyph_z,
    [123] = glyph_lbrace,
    [124] = glyph_vbar,
    [125] = glyph_rbrace,
    [126] = glyph_tilde,
    [127] = glyph_space,
    [128] = glyph_space,
    [129] = glyph_space,
    [130] = glyph_space,
    [131] = glyph_space,
    [132] = glyph_space,
    [133] = glyph_space,
    [134] = glyph_space,
    [135] = glyph_space,
    [136] = glyph_space,
    [137] = glyph_space,
    [138] = glyph_space,
    [139] = glyph_space,
    [140] = glyph_space,
    [141] = glyph_space,
    [142] = glyph_space,
    [143] = glyph_space,
    [144] = glyph_space,
    [145] = glyph_space,
    [146] = glyph_space,
    [147] = glyph_space,
    [148] = glyph_space,
    [149] = glyph_space,
    [150] = glyph_space,
    [151] = glyph_space,
    [152] = glyph_space,
    [153] = glyph_space,
    [154] = glyph_space,
    [155] = glyph_space,
    [156] = glyph_space,
    [157] = glyph_space,
    [158] = glyph_space,
    [159] = glyph_space,
    [160] = glyph_space,
    [161] = glyph_space,
    [162] = glyph_space,
    [163] = glyph_space,
    [164] = glyph_space,
    [165] = glyph_space,
    [166] = glyph_space,
    [167] = glyph_space,
    [168] = glyph_space,
    [169] = glyph_space,
    [170] = glyph_space,
    [171] = glyph_space,
    [172] = glyph_space,
    [173] = glyph_space,
    [174] = glyph_space,
    [175] = glyph_space,
    [176] = glyph_space,
    [177] = glyph_space,
    [178] = glyph_space,
    [179] = glyph_cursor,
    [180] = glyph_space,
    [181] = glyph_space,
    [182] = glyph_space,
    [183] = glyph_space,
    [184] = glyph_space,
    [185] = glyph_space,
    [186] = glyph_space,
    [187] = glyph_space,
    [188] = glyph_space,
    [189] = glyph_space,
    [190] = glyph_space,
    [191] = glyph_space,
    [192] = glyph_space,
    [193] = glyph_space,
    [194] = glyph_space,
    [195] = glyph_space,
    [196] = glyph_space,
    [197] = glyph_space,
    [198] = glyph_space,
    [199] = glyph_space,
    [200] = glyph_space,
    [201] = glyph_space,
    [202] = glyph_space,
    [203] = glyph_space,
    [204] = glyph_space,
    [205] = glyph_space,
    [206] = glyph_space,
    [207] = glyph_space,
    [208] = glyph_space,
    [209] = glyph_space,
    [210] = glyph_space,
    [211] = glyph_space,
    [212] = glyph_space,
    [213] = glyph_space,
    [214] = glyph_space,
    [215] = glyph_space,
    [216] = glyph_space,
    [217] = glyph_space,
    [218] = glyph_space,
    [219] = glyph_solidbox,
    [220] = glyph_space,
    [221] = glyph_space,
    [222] = glyph_space,
    [223] = glyph_space,
    [224] = glyph_space,
    [225] = glyph_space,
    [226] = glyph_space,
    [227] = glyph_space,
    [228] = glyph_space,
    [229] = glyph_space,
    [230] = glyph_space,
    [231] = glyph_space,
    [232] = glyph_space,
    [233] = glyph_space,
    [234] = glyph_space,
    [235] = glyph_space,
    [236] = glyph_space,
    [237] = glyph_space,
    [238] = glyph_space,
    [239] = glyph_space,
    [240] = glyph_space,
    [241] = glyph_space,
    [242] = glyph_space,
    [243] = glyph_space,
    [244] = glyph_space,
    [245] = glyph_space,
    [246] = glyph_space,
    [247] = glyph_space,
    [248] = glyph_space,
    [249] = glyph_space,
    [250] = glyph_space,
    [251] = glyph_tick,
    [252] = glyph_space,
    [253] = glyph_space,
    [254] = glyph_space,
    [255] = glyph_space,
};

br_font _FontProp4x6 = {
	.flags     = BR_FONTF_PROPORTIONAL,
	.glyph_x   = 4,
	.glyph_y   = 6,
	.spacing_x = 5,
	.spacing_y = 7,
	.width     = widths,
	.encoding  = encodings,
};
