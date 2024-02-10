#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <cctype>
#include <iostream>

#include "Attacks.h"
#include "BasicTypes.h"
#include "Bitboard.h"
#include "Moves.h"
#include "Zobrist.h"
#include "../Util.h"
#include "../Search/NNUE/Network.h"

// Helper lookup table for pins and check mask generation
extern Bitboard BETWEEN_SQ[64][64];
void initBetweenSQ();

const inline std::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class Position {
private:
  Bitboard pieces[6] = {};
  Bitboard colors[2] = {};

  Color stm;
  Square epSQ;
  uint8_t rights = 0;

  HashKey hash = 0; // zobrist key

  uint8_t hmc; // half move clock
  int phase = 0; // keep track of game phase (midgame/endgame)

  bool isInCheck = false;

  Accumulator acc[2];
public:
  // The board constructor uses FEN as input
  // all information up to half move clock is required
  Position(const std::string &fen);

  void addPiece(Color col, Piece pc, Square sq);

  void nnueAdd(Color col, Piece pc, Square sq);

  void clearPiece(Color col, Piece pc, Square sq);

  void nnueClear(Color col, Piece pc, Square sq);

  void movePiece(Color col, Piece pc, Square from, Square to);

  void updateRights(Square from, Square to);

  void makeMove(Move mv);

  inline void updateHash(HashKey key) {
    this->hash ^= key;
  }


  [[nodiscard]] Bitboard sqAttackers(Square sq, Bitboard occ);

  [[nodiscard]] inline Piece pieceOnSQ(Square sq) {
    for (int pc = Pawn; pc <= King; pc++) {
      if (getBit(this->pieces[pc], sq)) 
        return static_cast<Piece>(pc);
    }
    return NO_PC;
  }

  [[nodiscard]] inline Bitboard getPieceBB(Piece pc) {
    return this->pieces[pc];
  }

  [[nodiscard]] inline Bitboard getColorBB(Color col) {
    return this->colors[col];
  }

  [[nodiscard]] inline Bitboard getColoredPieceBB(Color col, Piece pc) {
    return this->colors[col] & this->pieces[pc];
  }

  // return a bitboard of all pieces
  [[nodiscard]] inline Bitboard all() {
    return this->colors[White] | this->colors[Black];
  }

  [[nodiscard]] inline Color sideToMove() {
    return this->stm;
  }

  [[nodiscard]] inline Color oppSideToMove() {
    return static_cast<Color>(this->stm ^ 1);
  }

  inline void changeSide() {
    // En passant expired
    if (this->epSQ != NO_SQ) {
      this->updateHash(getEPKey(epSQ));
      this->epSQ = NO_SQ;
    }

    this->stm = static_cast<Color>(this->stm ^ 1);
    this->updateHash(getSTMKey());
  }

  [[nodiscard]] inline HashKey getHash() {
    return this->hash;
  }

  [[nodiscard]] inline uint8_t halfMove() {
    return this->hmc;
  }

  [[nodiscard]] inline bool inCheck() {
    return this->getColorBB(this->oppSideToMove()) 
         & sqAttackers(
            lsb(this->getColoredPieceBB(this->sideToMove(), King)), 
            this->all());
  }

  // Move generation function
  // Defined in Movegen.cpp
  // Quiet only option specifically for quiescence search
  template<bool NoisyOnly>
  void genLegal(MoveList &ml);

  // Evaluation function
  int16_t evaluate();

private:
  // Helper functions for move generation
  // These aren't used anywhere else so they can remain private
  [[nodiscard]] std::pair<Bitboard, int> genCheckMask();
  [[nodiscard]] std::pair<Bitboard, Bitboard> genPinMask();
  [[nodiscard]] std::pair<Square, Square> pawnPush(Square from);
};
