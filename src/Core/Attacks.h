#pragma once

#include "BasicTypes.h"
#include "Bitboard.h"

constexpr Bitboard PAWN_ATTACKS[2][64] {
  {
    0x0ULL,                0x0ULL,                0x0ULL,                0x0ULL,
    0x0ULL,                0x0ULL,                0x0ULL,                0x0ULL,
    0x2ULL,                0x5ULL,                0xaULL,                0x14ULL,
    0x28ULL,               0x50ULL,               0xa0ULL,               0x40ULL,
    0x200ULL,              0x500ULL,              0xa00ULL,              0x1400ULL,
    0x2800ULL,             0x5000ULL,             0xa000ULL,             0x4000ULL,
    0x20000ULL,            0x50000ULL,            0xa0000ULL,            0x140000ULL,
    0x280000ULL,           0x500000ULL,           0xa00000ULL,           0x400000ULL,
    0x2000000ULL,          0x5000000ULL,          0xa000000ULL,          0x14000000ULL,
    0x28000000ULL,         0x50000000ULL,         0xa0000000ULL,         0x40000000ULL,
    0x200000000ULL,        0x500000000ULL,        0xa00000000ULL,        0x1400000000ULL,
    0x2800000000ULL,       0x5000000000ULL,       0xa000000000ULL,       0x4000000000ULL,
    0x20000000000ULL,      0x50000000000ULL,      0xa0000000000ULL,      0x140000000000ULL,
    0x280000000000ULL,     0x500000000000ULL,     0xa00000000000ULL,     0x400000000000ULL,
    0x2000000000000ULL,    0x5000000000000ULL,    0xa000000000000ULL,    0x14000000000000ULL,
    0x28000000000000ULL,   0x50000000000000ULL,   0xa0000000000000ULL,   0x40000000000000ULL
  },
  {
    0x200ULL,              0x500ULL,              0xa00ULL,              0x1400ULL,
    0x2800ULL,             0x5000ULL,             0xa000ULL,             0x4000ULL,
    0x20000ULL,            0x50000ULL,            0xa0000ULL,            0x140000ULL,
    0x280000ULL,           0x500000ULL,           0xa00000ULL,           0x400000ULL,
    0x2000000ULL,          0x5000000ULL,          0xa000000ULL,          0x14000000ULL,
    0x28000000ULL,         0x50000000ULL,         0xa0000000ULL,         0x40000000ULL,
    0x200000000ULL,        0x500000000ULL,        0xa00000000ULL,        0x1400000000ULL,
    0x2800000000ULL,       0x5000000000ULL,       0xa000000000ULL,       0x4000000000ULL,
    0x20000000000ULL,      0x50000000000ULL,      0xa0000000000ULL,      0x140000000000ULL,
    0x280000000000ULL,     0x500000000000ULL,     0xa00000000000ULL,     0x400000000000ULL,
    0x2000000000000ULL,    0x5000000000000ULL,    0xa000000000000ULL,    0x14000000000000ULL,
    0x28000000000000ULL,   0x50000000000000ULL,   0xa0000000000000ULL,   0x40000000000000ULL,
    0x200000000000000ULL,  0x500000000000000ULL,  0xa00000000000000ULL,  0x1400000000000000ULL,
    0x2800000000000000ULL, 0x5000000000000000ULL, 0xa000000000000000ULL, 0x4000000000000000ULL,
    0x0ULL,                0x0ULL,                0x0ULL,                0x0ULL,
    0x0ULL,                0x0ULL,                0x0ULL,                0x0ULL
  }
};

constexpr Bitboard KNIGHT_ATTACKS[64] {
    0x20400ULL,            0x50800ULL,            0xa1100ULL,            0x142200ULL,
    0x284400ULL,           0x508800ULL,           0xa01000ULL,           0x402000ULL,
    0x2040004ULL,          0x5080008ULL,          0xa110011ULL,          0x14220022ULL,
    0x28440044ULL,         0x50880088ULL,         0xa0100010ULL,         0x40200020ULL,
    0x204000402ULL,        0x508000805ULL,        0xa1100110aULL,        0x1422002214ULL,
    0x2844004428ULL,       0x5088008850ULL,       0xa0100010a0ULL,       0x4020002040ULL,
    0x20400040200ULL,      0x50800080500ULL,      0xa1100110a00ULL,      0x142200221400ULL,
    0x284400442800ULL,     0x508800885000ULL,     0xa0100010a000ULL,     0x402000204000ULL,
    0x2040004020000ULL,    0x5080008050000ULL,    0xa1100110a0000ULL,    0x14220022140000ULL,
    0x28440044280000ULL,   0x50880088500000ULL,   0xa0100010a00000ULL,   0x40200020400000ULL,
    0x204000402000000ULL,  0x508000805000000ULL,  0xa1100110a000000ULL,  0x1422002214000000ULL,
    0x2844004428000000ULL, 0x5088008850000000ULL, 0xa0100010a0000000ULL, 0x4020002040000000ULL,
    0x400040200000000ULL,  0x800080500000000ULL,  0x1100110a00000000ULL, 0x2200221400000000ULL,
    0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010a000000000ULL, 0x2000204000000000ULL,
    0x4020000000000ULL,    0x8050000000000ULL,    0x110a0000000000ULL,   0x22140000000000ULL,
    0x44280000000000ULL,   0x0088500000000000ULL, 0x0010a00000000000ULL, 0x20400000000000ULL
};

constexpr Bitboard KING_ATTACKS[64] { 
    0x302ULL,              0x705ULL,              0xe0aULL,              0x1c14ULL,
    0x3828ULL,             0x7050ULL,             0xe0a0ULL,             0xc040ULL,
    0x30203ULL,            0x70507ULL,            0xe0a0eULL,            0x1c141cULL,
    0x382838ULL,           0x705070ULL,           0xe0a0e0ULL,           0xc040c0ULL,
    0x3020300ULL,          0x7050700ULL,          0xe0a0e00ULL,          0x1c141c00ULL,
    0x38283800ULL,         0x70507000ULL,         0xe0a0e000ULL,         0xc040c000ULL,
    0x302030000ULL,        0x705070000ULL,        0xe0a0e0000ULL,        0x1c141c0000ULL,
    0x3828380000ULL,       0x7050700000ULL,       0xe0a0e00000ULL,       0xc040c00000ULL,
    0x30203000000ULL,      0x70507000000ULL,      0xe0a0e000000ULL,      0x1c141c000000ULL,
    0x382838000000ULL,     0x705070000000ULL,     0xe0a0e0000000ULL,     0xc040c0000000ULL,
    0x3020300000000ULL,    0x7050700000000ULL,    0xe0a0e00000000ULL,    0x1c141c00000000ULL,
    0x38283800000000ULL,   0x70507000000000ULL,   0xe0a0e000000000ULL,   0xc040c000000000ULL,
    0x302030000000000ULL,  0x705070000000000ULL,  0xe0a0e0000000000ULL,  0x1c141c0000000000ULL,
    0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
    0x203000000000000ULL,  0x507000000000000ULL,  0xa0e000000000000ULL,  0x141c000000000000ULL,
    0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL
};

constexpr Bitboard BISHOP_MASKS[64] {
    0x40201008040200ULL,   0x402010080400ULL,     0x4020100a00ULL,       0x40221400ULL, 
    0x2442800ULL,          0x204085000ULL,        0x20408102000ULL,      0x2040810204000ULL, 
    0x20100804020000ULL,   0x40201008040000ULL,   0x4020100a0000ULL,     0x4022140000ULL, 
    0x244280000ULL,        0x20408500000ULL,      0x2040810200000ULL,    0x4081020400000ULL, 
    0x10080402000200ULL,   0x20100804000400ULL,   0x4020100a000a00ULL,   0x402214001400ULL, 
    0x24428002800ULL,      0x2040850005000ULL,    0x4081020002000ULL,    0x8102040004000ULL, 
    0x8040200020400ULL,    0x10080400040800ULL,   0x20100a000a1000ULL,   0x40221400142200ULL, 
    0x2442800284400ULL,    0x4085000500800ULL,    0x8102000201000ULL,    0x10204000402000ULL, 
    0x4020002040800ULL,    0x8040004081000ULL,    0x100a000a102000ULL,   0x22140014224000ULL, 
    0x44280028440200ULL,   0x8500050080400ULL,    0x10200020100800ULL,   0x20400040201000ULL, 
    0x2000204081000ULL,    0x4000408102000ULL,    0xa000a10204000ULL,    0x14001422400000ULL, 
    0x28002844020000ULL,   0x50005008040200ULL,   0x20002010080400ULL,   0x40004020100800ULL, 
    0x20408102000ULL,      0x40810204000ULL,      0xa1020400000ULL,      0x142240000000ULL, 
    0x284402000000ULL,     0x500804020000ULL,     0x201008040200ULL,     0x402010080400ULL, 
    0x2040810204000ULL,    0x4081020400000ULL,    0xa102040000000ULL,    0x14224000000000ULL, 
    0x28440200000000ULL,   0x50080402000000ULL,   0x20100804020000ULL,   0x40201008040200ULL
};

constexpr Bitboard ROOK_MASKS[64] {
    0x101010101017e,    0x202020202027c,    0x404040404047a,    0x8080808080876, 
    0x1010101010106e,   0x2020202020205e,   0x4040404040403e,   0x8080808080807e, 
    0x1010101017e00,    0x2020202027c00,    0x4040404047a00,    0x8080808087600, 
    0x10101010106e00,   0x20202020205e00,   0x40404040403e00,   0x80808080807e00, 
    0x10101017e0100,    0x20202027c0200,    0x40404047a0400,    0x8080808760800, 
    0x101010106e1000,   0x202020205e2000,   0x404040403e4000,   0x808080807e8000, 
    0x101017e010100,    0x202027c020200,    0x404047a040400,    0x8080876080800, 
    0x1010106e101000,   0x2020205e202000,   0x4040403e404000,   0x8080807e808000, 
    0x1017e01010100,    0x2027c02020200,    0x4047a04040400,    0x8087608080800, 
    0x10106e10101000,   0x20205e20202000,   0x40403e40404000,   0x80807e80808000, 
    0x17e0101010100,    0x27c0202020200,    0x47a0404040400,    0x8760808080800, 
    0x106e1010101000,   0x205e2020202000,   0x403e4040404000,   0x807e8080808000, 
    0x7e010101010100,   0x7c020202020200,   0x7a040404040400,   0x76080808080800, 
    0x6e101010101000,   0x5e202020202000,   0x3e404040404000,   0x7e808080808000, 
    0x7e01010101010100, 0x7c02020202020200, 0x7a04040404040400, 0x7608080808080800, 
    0x6e10101010101000, 0x5e20202020202000, 0x3e40404040404000, 0x7e80808080808000
};

constexpr int BISHOP_SHIFTS[] {
    58, 59, 59, 59, 59, 59, 59, 58,
    59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59,
    58, 59, 59, 59, 59, 59, 59, 58
};

constexpr int ROOK_SHIFTS[] {
    52, 53, 53, 53, 53, 53, 53, 52,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    52, 53, 53, 53, 53, 53, 53, 52
};

constexpr Bitboard DIAG_MASKS[] {
    0x80,               0x8040,             0x804020,
    0x80402010,         0x8040201008,       0x804020100804,
    0x80402010080402,   0x8040201008040201, 0x4020100804020100,
    0x2010080402010000, 0x1008040201000000, 0x804020100000000,
    0x402010000000000,  0x201000000000000,  0x100000000000000
};

constexpr Bitboard ANTI_DIAG_MASKS[] {
    0x1,                0x102,              0x10204,
    0x1020408,          0x102040810,        0x10204081020,
    0x1020408102040,    0x102040810204080,  0x204081020408000,
    0x408102040800000,  0x810204080000000,  0x1020408000000000,
    0x2040800000000000, 0x4080000000000000, 0x8000000000000000
};

constexpr HashKey BISHOP_MAGIC[] {
    0x89a1121896040240, 0x2004844802002010, 0x2068080051921000, 0x62880a0220200808,
    0x4042004000000,    0x100822020200011,  0xc00444222012000a, 0x28808801216001,   
    0x400492088408100,  0x201c401040c0084,  0x840800910a0010,   0x82080240060,      
    0x2000840504006000, 0x30010c4108405004, 0x1008005410080802, 0x8144042209100900,
    0x208081020014400,  0x4800201208ca00,   0xf18140408012008,  0x1004002802102001,
    0x841000820080811,  0x40200200a42008,   0x800054042000,     0x88010400410c9000,
    0x520040470104290,  0x1004040051500081, 0x2002081833080021, 0x400c00c010142,
    0x941408200c002000, 0x658810000806011,  0x188071040440a00,  0x4800404002011c00,
    0x104442040404200,  0x511080202091021,  0x4022401120400,    0x80c0040400080120,
    0x8040010040820802, 0x480810700020090,  0x102008e00040242,  0x809005202050100,
    0x8002024220104080, 0x431008804142000,  0x19001802081400,   0x200014208040080,
    0x3308082008200100, 0x41010500040c020,  0x4012020c04210308, 0x208220a202004080, 
    0x111040120082000,  0x6803040141280a00, 0x2101004202410000, 0x8200000041108022,
    0x21082088000,      0x2410204010040,    0x40100400809000,   0x822088220820214,
    0x40808090012004,   0x910224040218c9,   0x402814422015008,  0x90014004842410,
    0x1000042304105,    0x10008830412a00,   0x2520081090008908, 0x40102000a0a60140
};

constexpr HashKey ROOK_MAGIC[] {
    0xa8002c000108020,  0x6c00049b0002001,  0x100200010090040,  0x2480041000800801,
    0x280028004000800,  0x900410008040022,  0x280020001001080,  0x2880002041000080,
    0xa000800080400034, 0x4808020004000,    0x2290802004801000, 0x411000d00100020,
    0x402800800040080,  0xb000401004208,    0x2409000100040200, 0x1002100004082,
    0x22878001e24000,   0x1090810021004010, 0x801030040200012,  0x500808008001000,
    0xa08018014000880,  0x8000808004000200, 0x201008080010200,  0x801020000441091,
    0x800080204005,     0x1040200040100048, 0x120200402082,     0xd14880480100080,
    0x12040280080080,   0x100040080020080,  0x9020010080800200, 0x813241200148449,
    0x491604001800080,  0x100401000402001,  0x4820010021001040, 0x400402202000812,
    0x209009005000802,  0x810800601800400,  0x4301083214000150, 0x204026458e001401,
    0x40204000808000,   0x8001008040010020, 0x8410820820420010, 0x1003001000090020,
    0x804040008008080,  0x12000810020004,   0x1000100200040208, 0x430000a044020001,
    0x280009023410300,  0xe0100040002240,   0x200100401700,     0x2244100408008080,
    0x8000400801980,    0x2000810040200,    0x8010100228810400, 0x2000009044210200,
    0x4080008040102101, 0x40002080411d01,   0x2005524060000901, 0x502001008400422,
    0x489a000810200402, 0x1004400080a13,    0x4000011008020084, 0x26002114058042
};

extern Bitboard BISHOP_ATTACKS[64][512] ;
extern Bitboard ROOK_ATTACKS[64][4096];

void initSliders();

[[nodiscard]] inline Bitboard getPawnAttack(Color col, Square sq) {
  return PAWN_ATTACKS[col][sq];
}

[[nodiscard]] inline Bitboard getKnightAttack(Square sq) {
  return KNIGHT_ATTACKS[sq];
}


[[nodiscard]] inline Bitboard getKingAttack(Square sq) {
  return KING_ATTACKS[sq];
}

[[nodiscard]] inline Bitboard getBishopAttack(Square sq, Bitboard occ) {
  occ &= BISHOP_MASKS[sq];
  occ *= BISHOP_MAGIC[sq];
  occ >>= BISHOP_SHIFTS[sq];

  return BISHOP_ATTACKS[sq][occ];
}

[[nodiscard]] inline Bitboard getRookAttack(Square sq, Bitboard occ) {
  occ &= ROOK_MASKS[sq];
  occ *= ROOK_MAGIC[sq];
  occ >>= ROOK_SHIFTS[sq];

  return ROOK_ATTACKS[sq][occ];
}

[[nodiscard]] inline Bitboard getQueenAttack(Square sq, Bitboard occ) {
  return getBishopAttack(sq, occ) 
       | getRookAttack(sq, occ);
}