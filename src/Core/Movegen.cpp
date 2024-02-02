#include "Position.h"
#include <cassert>

template void Position::genLegal<false>(MoveList& ml);
template void Position::genLegal<true>(MoveList& ml);

constexpr Bitboard NO_CHECK = 0xffffffffffffffffULL;
Bitboard BETWEEN_SQ[64][64];

void initBetweenSQ() {
  for (int sq1 = 0; sq1 < 64; sq1++) {
    for (int sq2 = 0; sq2 < 64; sq2++) {
      Bitboard sqs = sqToBB(sq1) | sqToBB(sq2);
      Rank rank1 = rankIx(sq1);
      Rank rank2 = rankIx(sq2);
      File file1 = fileIx(sq1);
      File file2 = fileIx(sq2);
      Diag diag1 = 7 + rank1 - file1;
      Diag diag2 = 7 + rank2 - file2;
      Diag antiDiag1 = rank1 + file1;
      Diag antiDiag2 = rank2 + file2;

      // generate attack between the squares if they are aligned
      if (diag1 == diag2 || antiDiag1 == antiDiag2) {
        BETWEEN_SQ[sq1][sq2] = getBishopAttack(sq1, sqs) & getBishopAttack(sq2, sqs);
      }
      if (file1 == file2 || rank1 == rank2) {
        BETWEEN_SQ[sq1][sq2] = getRookAttack(sq1, sqs) & getRookAttack(sq2, sqs);
      }
    }
  }
}

std::pair<Bitboard, int> Position::genCheckMask() {
  Color stm = this->sideToMove();
  Color xstm = this->oppSideToMove();
  Square kingSQ = lsb(this->getColoredPieceBB(stm, King));
  Bitboard occ = this->all();

  Bitboard checkMask = 0;
  int checksCount = 0;

  Bitboard pawns = getPawnAttack(stm, kingSQ) & getColoredPieceBB(xstm, Pawn);
  Bitboard knights = getKnightAttack(kingSQ) & getColoredPieceBB(xstm, Knight);
  Bitboard bishops = getBishopAttack(kingSQ, occ) 
                   & (getColoredPieceBB(xstm, Bishop) 
                   |  getColoredPieceBB(xstm, Queen));
  Bitboard rooks = getRookAttack(kingSQ, occ) 
                 & (getColoredPieceBB(xstm, Rook) 
                 |  getColoredPieceBB(xstm, Queen));

  this->isInCheck = false;

  if (pawns != 0) {
    checkMask |= pawns;
    checksCount++;
    this->isInCheck = true;
  }

  if (knights != 0) {
    checkMask |= knights;
    checksCount++;
    this->isInCheck = true;
  }

  if (bishops != 0) {
    checkMask |= BETWEEN_SQ[kingSQ][lsb(bishops)] | sqToBB(lsb(bishops));
    checksCount++;
    if (popcnt(bishops) > 1) checksCount++;
    this->isInCheck = true;
  }

  if (rooks != 0) {
    checkMask |= BETWEEN_SQ[kingSQ][lsb(rooks)] | sqToBB(lsb(rooks));
    checksCount++;
    if (popcnt(rooks) > 1) checksCount++;
    this->isInCheck = true;
  }
  // If there's no check, return every square.
  // This way every move will "address" the inexistent check.
  if (checksCount == 0) checkMask = NO_CHECK;
  return {checkMask, checksCount};
}

std::pair<Bitboard, Bitboard> Position::genPinMask() {
  Color stm = this->sideToMove();
  Color xstm = this->oppSideToMove();
  Square kingSQ = lsb(this->getColoredPieceBB(stm, King));

  // Pretend as if we don't have any piece.
  // This way, we get all pieces staring at the king
  // regardless of any blockers in the way.
  Bitboard opp = this->colors[xstm];
  Bitboard bishops = getBishopAttack(kingSQ, opp) 
                   & (getColoredPieceBB(xstm, Bishop) 
                   |  getColoredPieceBB(xstm, Queen));
  Bitboard rooks = getRookAttack(kingSQ, opp) 
                 & (getColoredPieceBB(xstm, Rook) 
                 |  getColoredPieceBB(xstm, Queen));

  Bitboard diagPin = 0ULL;
  Bitboard orthPin = 0ULL;

  // Check the attacking ray of each attacker.
  // There's a pin if exactly one of our pieces lies on the ray.
  while (bishops) {
    Square attackerSq = lsb(bishops);
    popBit(bishops, attackerSq);

    Bitboard possiblePin = BETWEEN_SQ[kingSQ][attackerSq] | sqToBB(attackerSq);

    if (popcnt(possiblePin & this->colors[stm]) == 1) diagPin |= possiblePin;
  }
  // Do the same for rooks.
  while (rooks) {
    Square attackerSq = lsb(rooks);
    popBit(rooks, attackerSq);

    Bitboard possiblePin = BETWEEN_SQ[kingSQ][attackerSq] | sqToBB(attackerSq);

    if (popcnt(possiblePin & this->colors[stm]) == 1) orthPin |= possiblePin;
  }

  return {diagPin, orthPin};
}

// Helper function for calculating pawn push
std::pair<Square, Square> Position::pawnPush(Square from) {
  // White => 0, Black => 1
  Direction shift = this->sideToMove() ? South : North;
  Bitboard occ = this->all();

  Square sp = from + shift;

  // Pawn is blocked
  if (getBit(occ, sp)) return {NO_SQ, NO_SQ};

  Square dp = sp + shift;
  bool onSecondRank = getBit(SECOND_RANK[this->sideToMove()], from);

  // Pawn not on second rank or blocked on double push
  if (!onSecondRank || getBit(occ, dp)) return {sp, NO_SQ};

  return {sp, dp};
}

template<bool NoisyOnly>
void Position::genLegal(MoveList &ml) {
  Color stm = this->sideToMove();
  Color xstm = this->oppSideToMove();
  Bitboard occ = this->all();
  Square kingSQ = lsb(this->getColoredPieceBB(stm, King));

  auto [checkMask, checksCount] = genCheckMask(); 

  { // Generate king moves
    // Only one king can exist for each side.
    Square from = kingSQ;
    Bitboard attacks = getKingAttack(from) & ~this->getColorBB(stm);

    // Temporarily remove occupancy to check for x-ray
    popBit(this->colors[stm], from);

    while (attacks) {
      Square to = lsb(attacks);
      popBit(attacks, to);

      // If the move puts the king in check, skip
      if (sqAttackers(to, this->all()) & this->getColorBB(xstm)) continue;

      // Only push capture moves in quiescence search
      bool isCapture = getBit(this->getColorBB(xstm), to);
      if constexpr (NoisyOnly) {
        if (!isCapture) continue;
        else ml.push(Move(from, to, Capture, King));
      } else ml.push(Move(from, to, isCapture ? Capture : Normal, King));
    }

    // Put the occupancy bit back
    setBit(this->colors[stm], from);
  }

  // Only the king can move during double check
  if (checksCount > 1) return;

  if constexpr (!NoisyOnly) { // Generate castling moves
    // A castling move may never be a capture
    // Only one king can exist for each side.
    Square from = kingSQ;
    if (stm == White) {
      // White's kingside castling
      if (!isInCheck // Not in check
      && this->rights & C_WK // Allowed to castle kingside
      && !getBit(occ, f1) // f1 is vacant
      && !getBit(occ, g1) // g1 is vacant
      && !(sqAttackers(f1, occ) & this->getColorBB(xstm)) // f1 is not attacked
      && !(sqAttackers(g1, occ) & this->getColorBB(xstm))) // g1 is not attacked
      {
        ml.push(Move(from, g1, KSCastle, King));
      }
      // White's queenside castling
      if (!isInCheck // Not in check
      && this->rights & C_WQ // Allowed to castle queenside
      && !getBit(occ, b1) // b1 is vacant
      && !getBit(occ, c1) // c1 is vacant
      && !getBit(occ, d1) // d1 is vacant
      && !(sqAttackers(c1, occ) & this->getColorBB(xstm)) // f1 is not attacked
      && !(sqAttackers(d1, occ) & this->getColorBB(xstm))) // g1 is not attacked
      {
        ml.push(Move(from, c1, QSCastle, King));
      }
    } else {
      // Black's kingside castling
      if (!isInCheck // Not in check
      && this->rights & C_BK // Allowed to castle kingside
      && !getBit(occ, f8) // f1 is vacant
      && !getBit(occ, g8) // g1 is vacant
      && !(sqAttackers(f8, occ) & this->getColorBB(xstm)) // f1 is not attacked
      && !(sqAttackers(g8, occ) & this->getColorBB(xstm))) // g1 is not attacked
      {
        ml.push(Move(from, g8, KSCastle, King));
      }
      // Black's queenside castling
      if (!isInCheck // Not in check
      && this->rights & C_BQ // Allowed to castle queenside
      && !getBit(occ, b8) // b8 is vacant
      && !getBit(occ, c8) // c8 is vacant
      && !getBit(occ, d8) // d8 is vacant
      && !(sqAttackers(c8, occ) & this->getColorBB(xstm)) // f8 is not attacked
      && !(sqAttackers(d8, occ) & this->getColorBB(xstm))) // g8 is not attacked
      {
        ml.push(Move(from, c8, QSCastle, King));
      }
    }
  }

  auto [diagPin, orthPin] = genPinMask();

  { // Generate knight moves
    Bitboard pieces = this->getPieceBB(Knight) & this->getColorBB(stm);
    while (pieces) { // Generate for each piece until none left
      Square from = lsb(pieces);
      popBit(pieces, from);

      // A pinned knight can never move
      if (getBit(diagPin | orthPin, from)) continue;

      Bitboard attacks = getKnightAttack(from) & ~this->getColorBB(stm);
      while (attacks) {
        Square to = lsb(attacks);
        popBit(attacks, to);

        // The move doesn't address check, skip
        if (!getBit(checkMask, to)) continue;

        // Only push capture moves in quiescence search
        bool isCapture = getBit(this->getColorBB(xstm), to);
        if constexpr (NoisyOnly) {
          if (!isCapture) continue;
          else ml.push(Move(from, to, Capture, Knight));
        } else ml.push(Move(from, to, isCapture ? Capture : Normal, Knight));
      }
    }
  }

  { // Generate bishop moves and diagonal queen moves
    Bitboard pieces = (this->getPieceBB(Bishop) | this->getPieceBB(Queen)) & this->getColorBB(stm);
    while (pieces) { // Generate for each piece until none left
      Square from = lsb(pieces);
      popBit(pieces, from);

      // An orthogonally pinned bishop can never move
      if (getBit(orthPin, from)) continue;

      Bitboard attacks = getBishopAttack(from, occ) & ~this->getColorBB(stm);
      while (attacks) {
        Square to = lsb(attacks);
        popBit(attacks, to);

        if (!getBit(checkMask, to) // The move doesn't address check
        || (getBit(diagPin, from) && !getBit(diagPin, to))) continue; // The move exposes the king

        // We could be generating queen move here
        Piece pc = getBit(this->getPieceBB(Queen), from) ? Queen : Bishop;

        // Only push capture moves in quiescence search
        bool isCapture = getBit(this->getColorBB(xstm), to);
        if constexpr (NoisyOnly) {
          if (!isCapture) continue;
          else ml.push(Move(from, to, Capture, pc));
        } else ml.push(Move(from, to, isCapture ? Capture : Normal, pc));
      }
    }
  }

  { // Generate rook moves and orthogonal queen moves
    Bitboard pieces = (this->getPieceBB(Rook) | this->getPieceBB(Queen)) & this->getColorBB(stm);
    while (pieces) { // Generate for each piece until none left
      Square from = lsb(pieces);
      popBit(pieces, from);

      // A diagonally pinned rook can never move
      if (getBit(diagPin, from)) continue;

      Bitboard attacks = getRookAttack(from, occ) & ~this->getColorBB(stm);
      while (attacks) {
        Square to = lsb(attacks);
        popBit(attacks, to);

        if (!getBit(checkMask, to) // The move doesn't address check
        || (getBit(orthPin, from) && !getBit(orthPin, to))) continue; // The move exposes the king

        // We could be generating queen move here
        Piece pc = getBit(this->getPieceBB(Queen), from) ? Queen : Rook;

        // Only push capture moves in quiescence search
        bool isCapture = getBit(this->getColorBB(xstm), to);
        if constexpr (NoisyOnly) {
          if (!isCapture) continue;
          else ml.push(Move(from, to, Capture, pc));
        } else ml.push(Move(from, to, isCapture ? Capture : Normal, pc));
      }
    }
  }

  { // Generate pawn moves
    Bitboard pieces = this->getPieceBB(Pawn) & this->getColorBB(stm);
    while (pieces) {
      Square from = lsb(pieces);
      popBit(pieces, from);

      // Single/double push
      auto [sp, dp] = pawnPush(from);

      // When a pawn is pinned vertically, we can add the
      // available push along the pin and skip the rest.
      // Promotion is not a concern because a pawn
      // can never move to 8th rank while being pinned.
      if constexpr (!NoisyOnly) { // No capture or promotion here
        if (getBit(orthPin, from)) {
          if (sp == NO_SQ) continue;

          // Exposes the king to horizontal check, skip
          if (!getBit(orthPin, sp)) continue;

          if (getBit(checkMask, sp)) 
            ml.push(Move(from, sp, Normal, Pawn));

          if (dp != NO_SQ && getBit(checkMask, dp)) 
            ml.push(Move(from, dp, DoublePush, Pawn));

          continue; // Skip the rest of the generation
        } 
      } else continue; // Skip anyway without pushing anything

      bool onPromoRank = getBit(SEVENTH_RANK[this->sideToMove()], from);

      // We have to include en passant square
      Bitboard attacks = getPawnAttack(stm, from) 
                       & (this->getColorBB(xstm) | (this->epSQ == NO_SQ ? 0 : sqToBB(this->epSQ)));

      while (attacks) { // Pawn capture generation
        Square to = lsb(attacks);
        popBit(attacks, to);

        bool isEnpassant = to == this->epSQ;

        // The move doesn't address check, skip
        Square epCaptureSQ = to - (this->sideToMove() ? South : North);

        if (!getBit(checkMask, to) 
        && (isEnpassant ? !getBit(checkMask, epCaptureSQ) : true)) 
          continue;


        if (isEnpassant) { // Filter out illegal en passant
          bool illegal = false;

          // Pretend as if we made the capture
          popBit(this->colors[stm], from);
          setBit(this->colors[stm], to);
          popBit(this->colors[xstm], epCaptureSQ);

          // If the king is exposed, the capture is illegal
          if (getRookAttack(kingSQ, this->all())
           &  (this->getColoredPieceBB(xstm, Rook)
           |   this->getColoredPieceBB(xstm, Queen))) illegal = true;

          // Undo the capture
          setBit(this->colors[stm], from);
          popBit(this->colors[stm], to);
          setBit(this->colors[xstm], epCaptureSQ);

          if (illegal) continue;
        }

        // When a pawn is pinned diagonally, we add the capture
        // along the pinned diagonal and the rest of the moves
        // can be safely skipped.
        if (getBit(diagPin, from)) {
          // Wrong capture, skip
          if (!getBit(diagPin, to)) continue;

          if (onPromoRank) {
            ml.push(Move(from, to, QueenPromoCapture, Pawn));
            ml.push(Move(from, to, KnightPromoCapture, Pawn));
            ml.push(Move(from, to, RookPromoCapture, Pawn));
            ml.push(Move(from, to, BishopPromoCapture, Pawn));
          } else ml.push(Move(from, to, isEnpassant ? EnPassant : Capture, Pawn));

          // Skip the rest
          continue;
        }

        // Non-pin captures
        if (onPromoRank) {
          ml.push(Move(from, to, QueenPromoCapture, Pawn));
          ml.push(Move(from, to, KnightPromoCapture, Pawn));
          ml.push(Move(from, to, BishopPromoCapture, Pawn));
          ml.push(Move(from, to, RookPromoCapture, Pawn));
        } else ml.push(Move(from, to, isEnpassant ? EnPassant : Capture, Pawn));
      } // End of pawn attack generation
      // Generate non-capture pawn moves
      // If the pawn is blocked or pinned diagonally, skip
      if (sp == NO_SQ || getBit(diagPin, from)) continue;

      if (getBit(checkMask, sp)) {
        if (onPromoRank) {
          ml.push(Move(from, sp, QueenPromo, Pawn));
          ml.push(Move(from, sp, KnightPromo, Pawn));
          ml.push(Move(from, sp, BishopPromo, Pawn));
          ml.push(Move(from, sp, RookPromo, Pawn));
        } else if constexpr (!NoisyOnly) { 
          ml.push(Move(from, sp, Normal, Pawn));
        }
      }

      // Double push isn't noisy
      if constexpr (!NoisyOnly)
        if (dp != NO_SQ && getBit(checkMask, dp)) 
          ml.push(Move(from, dp, DoublePush, Pawn));
    }
  }
}
