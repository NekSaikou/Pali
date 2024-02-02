#pragma once

#include "BasicTypes.h"
#include "Bitboard.h"
#include <string>

constexpr int MAX_MOVE = 218;

enum MoveFlag : uint8_t 
    { Normal
    , DoublePush
    , KSCastle
    , QSCastle
    , Capture
    , EnPassant
    , KnightPromo = 8 
    , BishopPromo
    , RookPromo
    , QueenPromo
    , KnightPromoCapture
    , BishopPromoCapture
    , RookPromoCapture
    , QueenPromoCapture
    };

using MoveScore = int32_t;

class Move { 
private:
  Square from;
  Square to;
  MoveFlag flag;
  Piece pc;

public:
  [[nodiscard]] inline Move() {
    this->from = 0;
    this->to = 0;
    this->flag = Normal;
    this->pc = NO_PC;
  }

  [[nodiscard]] inline Move(Square from, Square to, MoveFlag flag, Piece pc) {
    this->from = from;
    this->to = to;
    this->flag = flag;
    this->pc = pc;
  };

  [[nodiscard]] inline Square getFrom() {
    return this->from;
  }

  [[nodiscard]] inline Square getTo() {
    return this->to;
  }

  [[nodiscard]] inline Piece getPiece() {
    return this->pc;
  }

  [[nodiscard]] inline bool isDP() {
    return this->flag == DoublePush;
  }

  [[nodiscard]] inline bool isEP() {
    return this->flag == EnPassant;
  }

  [[nodiscard]] inline bool isCastle() {
    return this->flag == KSCastle
        || this->flag == QSCastle;
  }

  [[nodiscard]] inline bool isCapture() {
    return this->flag & 4;
  }

  [[nodiscard]] inline bool isPromo() {
    return this->flag & 8;
  }

  [[nodiscard]] inline Piece promoType() {
    return static_cast<Piece>((this->flag & 3) + 1);
  }

  [[nodiscard]] inline bool isQuiet() {
    return !isCapture() && this->flag != QueenPromo;
  }

  [[nodiscard]] inline uint16_t compress() {
    return (this->from) | 
           (this->to << 6) | 
           (this->flag << 12);
  }

  [[nodiscard]] inline std::string string() {
    std::string str;
    str += SQUARE_NAME[this->from]; 
    str += SQUARE_NAME[this->to];
    str += this->isPromo() ? PIECE_SYMBOL[this->promoType()] : "";

    return str;
  }
};

class MoveList {
private:
  Move moves[MAX_MOVE];
  MoveScore scores[MAX_MOVE] = {};
  int length = 0;

public:
  [[nodiscard]] inline MoveList() {}

  [[nodiscard]] inline Move getMove(int ix) const {
    return this->moves[ix];
  };

  [[nodiscard]] inline MoveScore getScore(int ix) const {
    return this->scores[ix];
  };

  [[nodiscard]] inline int getLength() const {
    return this->length;
  };

  void inline push(Move mv) {
    this->moves[this->length] = mv;
    this->length++;
  };

  [[nodiscard]] Move inline takeLast() {
    return this->moves[--this->length];
  };

  void inline clear() {
    this->length = 0;
  };

  void inline swap(int ix1, int ix2) {
    Move m1 = this->moves[ix1];
    this->moves[ix1] = this->moves[ix2];
    this->moves[ix2] = m1;
    
    MoveScore s1 = this->scores[ix1];
    this->scores[ix1] = this->scores[ix2];
    this->scores[ix2] = s1;
  };

  void inline scoreMove(int ix, MoveScore score) {
    this->scores[ix] = score;
  };
};
