#pragma once

#include "../Core/Position.h"
#include "../Core/Moves.h"

using EvalScore = int16_t;

constexpr EvalScore SEE_VAL[6] = 
    { 100 // Pawn
    , 300 // Knight
    , 300 // Bishop
    , 450 // Rook
    , 850 // Queen
    , 0   // King (should never get captured)
    };

inline bool staticExchangeEval(Position &pos, Move &mv, EvalScore threshold) {
  if (mv.isCastle() // Castling will never result in capture
  ||  mv.isPromo()  // Promotion will probably beat whatever threshold there is
  ) return true;

  Square sq = mv.getTo();
  Piece attackingPiece = mv.getPiece();
  Piece target = pos.pieceOnSQ(sq);
  EvalScore targetValue = target == NO_PC 
    ? 0
    : SEE_VAL[target];

  // Assumes we lose the moved piece in the exchange
  EvalScore exchangeScore = targetValue - SEE_VAL[attackingPiece] - threshold;

  // If the exchange beats the threshold anyway then it will always pass
  if (exchangeScore >= 0) return true;

  // Since we already moved the first piece, we must remove it from the board
  Bitboard occ = pos.all();
  popBit(occ, mv.getFrom());
  popBit(occ, sq); // Remove the piece occupying target square

  Bitboard attackers = pos.sqAttackers(sq, occ);

  // Sliders
  Bitboard bishops = pos.getPieceBB(Bishop) | pos.getPieceBB(Queen);
  Bitboard rooks = pos.getPieceBB(Rook) | pos.getPieceBB(Queen);

  // We already moved our piece, so we start from the opponent's turn
  Color sideToFail = pos.oppSideToMove();

  // Make capture until either side run out of piece or fail the threshold
  do {
    attackers &= occ; // Remove used pieces

    // The attacking side runs out of piece
    Bitboard ourAttackers = attackers & pos.getColorBB(sideToFail);
    if (ourAttackers == 0)
      return sideToFail != pos.sideToMove();

    // Pick the least valuable piece for the next move
    for (int pc = Pawn; pc <= King; pc++) {
      if (pos.getPieceBB(static_cast<Piece>(pc)) & ourAttackers) {
        attackingPiece = static_cast<Piece>(pc);
        // Remove the moved piece from the board
        occ ^= sqToBB(lsb(ourAttackers & 
                          pos.getColoredPieceBB(sideToFail, attackingPiece)
                          ));
        break;
      }
    }

    // Check if new line of attacks opens up
    if (attackingPiece == Pawn
    ||  attackingPiece == Bishop
    ||  attackingPiece == Queen)
      attackers |= getBishopAttack(sq, occ) & bishops;

    if (attackingPiece == Rook
    ||  attackingPiece == Queen)
      attackers |= getRookAttack(sq, occ) & rooks;

    exchangeScore = -exchangeScore - 1 - SEE_VAL[attackingPiece];
    
    // Switch side then run the next iteration
    sideToFail = static_cast<Color>(sideToFail ^ 1);
  } while (exchangeScore <= 0);

  // If the move fails the threshold but results leaves opponent king
  // in check, the failing side is the opponent
  if (attackingPiece == King && attackers & pos.getColorBB(sideToFail))
    sideToFail = static_cast<Color>(sideToFail ^ 1);

  return sideToFail != pos.sideToMove();
}
