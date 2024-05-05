#pragma once

#include "../nnue/Network.h"
#include "Bitboard.h"
#include "Color.h"
#include "Move.h"
#include "Piece.h"
#include "Square.h"
#include "Zobrist.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

namespace pali {

constexpr char const *STARTPOS =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class Position {
  std::array<Bitboard, 6> Pieces;
  std::array<Bitboard, 2> Colors;

  Color Stm;
  Square EpSq = Square::None;
  uint8_t Rights = 0;

  uint64_t Hash = 0;

  uint8_t Hmc;

  std::array<Accumulator, 2> Acc;

  std::vector<uint64_t> OccuredPos;

public:
  /// Position constructor: accepts full FEN as input and
  /// parse it into the position
  /// Undefined behavior when given invalid FEN
  Position(const std::string &Fen);

  [[nodiscard]] Bitboard getBB(Piece Pc) const { return Pieces[Pc]; }

  [[nodiscard]] Bitboard getBB(Color Col) const { return Colors[Col]; }

  [[nodiscard]] Bitboard getBB(Piece Pc, Color Col) const {
    return Pieces[Pc] & Colors[Col];
  }

  [[nodiscard]] Bitboard allBB() const {
    return Colors[Color::White] | Colors[Color::Black];
  }

  [[nodiscard]] Color stm() const { return Stm; }

  [[nodiscard]] Square epSq() const { return EpSq; }

  [[nodiscard]] uint8_t rights() const { return Rights; }

  [[nodiscard]] uint64_t hash() const { return Hash; }

  [[nodiscard]] int hmc() const { return Hmc; }

  /// Return a bitboard containing every piece targeting the given Square
  [[nodiscard]] Bitboard attacksAt(Square Sq) const;
  [[nodiscard]] Bitboard attacksAt(Square Sq, Bitboard Occ) const;

  [[nodiscard]] bool isInCheck() const {
    return attacksAt(getBB(Piece::King, Stm).lsb()) & getBB(Stm.inverse());
  }

  /// What piece is on the given square
  [[nodiscard]] Piece pieceAt(Square Sq) const {
    for (int Pc = Piece::Pawn; Pc <= Piece::King; ++Pc) {
      if (Pieces[Pc].getBit(Sq))
        return static_cast<Piece::Type>(Pc);
    }

    return Piece::None;
  }

  /// Add noisy psuedolegal moves to move list
  void genNoisy(MoveList &Ml) const;

  /// Add quiet psuedolegal moves to move list
  void genQuiet(MoveList &Ml) const;

  /// Make move on the board regardless of legality
  /// return false if the move is illegal
  bool makeMove(Move Mv);

  /// Switch side to move and remove existing en passant square
  void changeSide() {
    // En passant square expired
    if (EpSq.exists()) {
      updateHash(getEPKey(EpSq));
      EpSq = Square::None;
    }

    ++Hmc;

    Stm = Stm.inverse();
    updateHash(getStmKey());
  }

  [[nodiscard]] int evaluate() const;

  [[nodiscard]] bool isDraw() const {
    if (Hmc >= 100)
      return true;

    for (int i = static_cast<int>(OccuredPos.size()) - 2; i >= 0; i -= 2) {
      if (i + Hmc < OccuredPos.size())
        return false;
  
      if (OccuredPos[i] == Hash)
        return true;
    }

    return false;
  }

private:
  void updateHash(uint64_t Key) { Hash ^= Key; }

  void nnueAdd(Piece Pc, Color Col, Square Sq) {
    const auto [WhiteIx, BlackIx] = nnueIdx(Col, Pc, Sq);
    const auto &WhiteAdd = NNUE.InputWeights[WhiteIx].Data;
    const auto &BlackAdd = NNUE.InputWeights[BlackIx].Data;
    for (int i = 0; i < HIDDEN_SIZE; i++) {
      Acc[0].Data[i] += WhiteAdd[i];
      Acc[1].Data[i] += BlackAdd[i];
    }
  }

  void nnueSub(Piece Pc, Color Col, Square Sq) {
    const auto [WhiteIx, BlackIx] = nnueIdx(Col, Pc, Sq);
    const auto &WhiteSub = NNUE.InputWeights[WhiteIx].Data;
    const auto &BlackSub = NNUE.InputWeights[BlackIx].Data;
    for (int i = 0; i < HIDDEN_SIZE; i++) {
      Acc[0].Data[i] -= WhiteSub[i];
      Acc[1].Data[i] -= BlackSub[i];
    }
  }

  void addPiece(Piece Pc, Color Col, Square Sq) {
    Pieces[Pc].set(Sq);
    Colors[Col].set(Sq);

    updateHash(getPieceKey(Pc, Sq));
    updateHash(getColorKey(Col, Sq));

    nnueAdd(Pc, Col, Sq);
  }

  void clearPiece(Piece Pc, Color Col, Square Sq) {
    Pieces[Pc].pop(Sq);
    Colors[Col].pop(Sq);

    updateHash(getPieceKey(Pc, Sq));
    updateHash(getColorKey(Col, Sq));

    nnueSub(Pc, Col, Sq);
  }

  void movePiece(Piece Pc, Color Col, Square From, Square To) {
    addPiece(Pc, Col, To);
    clearPiece(Pc, Col, From);
  }
};

} // namespace pali
