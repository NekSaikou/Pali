#include "Position.h"

constexpr uint8_t CASTLING_UPDATE[64] {
  7,  15, 15, 15, 3,  15, 15, 11, 
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14
};

void Position::addPiece(Color col, Piece pc, Square sq) {
  setBit(this->pieces[pc], sq);
  setBit(this->colors[col], sq);
}

void Position::clearPiece(Color col, Piece pc, Square sq) {
  popBit(this->pieces[pc], sq);
  popBit(this->colors[col], sq);
}

void Position::movePiece(Color col, Piece pc, Square from, Square to) {
  clearPiece(col, pc, from);
  addPiece(col, pc, to);
}

void Position::updateRights(Square from, Square to) {
  this->rights &= CASTLING_UPDATE[from];
  this->rights &= CASTLING_UPDATE[to];
}

void Position::makeMove(Move mv) {
  Color stm = this->sideToMove();
  Color xstm = this->oppSideToMove();
  Square from = mv.getFrom();
  Square to = mv.getTo();

  Piece target = pieceOnSQ(mv.getTo());

  this->hmc++; // Increment this first thing

  // We will clear pawn captured by en passant
  // and update this->epSQ in case of double push on this square
  Square epSQUpdate = to - (this->sideToMove() ? South : North);

  // We reset half move clock either when a pawn moves
  // or a capture happens
  if (mv.getPiece() == Pawn) this->hmc = 0;

  // Remember to clear out captured piece
  if (mv.isEP()) {
    this->clearPiece(xstm, Pawn, epSQUpdate);
  }
  else if (mv.isCapture()) {
    this->clearPiece(xstm, target, to);
    this->hmc = 0;
  }

  // En passant expired
  if (this->epSQ != NO_SQ) {
    this->epSQ = NO_SQ;
  }

  // Move the piece to a the destination and remove it from the old square
  Piece addedPiece = mv.isPromo() ? mv.promoType() : mv.getPiece();
  this->clearPiece(stm, mv.getPiece(), from);
  this->addPiece(stm, addedPiece, to);

  // Add new en passant target in case of double push
  if (mv.isDP()) {
    this->epSQ = epSQUpdate;
  }

  // Move the rook when castling
  if (mv.isCastle()) {
    switch (to) {
      case g1: movePiece(stm, Rook, h1, f1); break;
      case c1: movePiece(stm, Rook, a1, d1); break;
      case g8: movePiece(stm, Rook, h8, f8); break;
      case c8: movePiece(stm, Rook, a8, d8); break;
    }
  }

  // Update castling rights
  this->updateRights(from, to);

  // End the turn and change side
  this->changeSide();
}

// Return the position of each piece attacking a square
Bitboard Position::sqAttackers(Square sq, Bitboard occ) {
  return (getPawnAttack(White, sq) & this->getPieceBB(Pawn) & this->getColorBB(Black))
       | (getPawnAttack(Black, sq) & this->getPieceBB(Pawn) & this->getColorBB(White))
       | (getKnightAttack(sq) & this->getPieceBB(Knight))
       | (getKingAttack(sq) & this->getPieceBB(King))
       | (getBishopAttack(sq, occ) & this->getPieceBB(Bishop))
       | (getRookAttack(sq, occ) & this->getPieceBB(Rook))
       | (getQueenAttack(sq, occ) & this->getPieceBB(Queen));
}

Position::Position(const std::string &fen) {
  // Trim the FEN then break it into tokens
  std::vector<std::string> tokens = splitWS(trim(fen));

  // Parse position of pieces
  Rank rank = 0;
  File file = 0;
  for (char c : tokens[0]) {
    // Go to next rank
    if (c == '/') {
        file = 0;
        rank++;
        continue;
    }
    // Empty square(s)
    if (isdigit(c)) {
      file += (c - '0');
      continue;
    } 
    // Set piece down
    Color col = isupper(c) ? White : Black;
    Square sq = squareIx(rank, file);
    switch (toupper(c)) {
        case 'P': addPiece(col, Pawn, sq); break;
        case 'N': addPiece(col, Knight, sq); break;
        case 'B': addPiece(col, Bishop, sq); break;
        case 'R': addPiece(col, Rook, sq); break;
        case 'Q': addPiece(col, Queen, sq); break;
        case 'K': addPiece(col, King, sq); break;
    }

    // Go to the next file
    file++;
  }

  // Parse side to move
  // This token should always contain only either 'w' or 'b'
  this->stm = tokens[1][0] == 'w' ? White : Black;

  // Parse castling rights
  for (char c : tokens[2]) {
    switch (c) {
      case 'K': this->rights |= C_WK; break;
      case 'Q': this->rights |= C_WQ; break;
      case 'k': this->rights |= C_BK; break;
      case 'q': this->rights |= C_BQ; break;
    }
  }

  // Parse en passant
  if (tokens[3][0] == '-') this->epSQ = NO_SQ;
  else this->epSQ = squareIx(7 - (tokens[3][1] - '1'), tokens[3][0] - 'a');

  // Parse half move clock
  this->hmc = std::stoi(tokens[4]);
}
