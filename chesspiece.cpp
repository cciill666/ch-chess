#include "chesspiece.h"

ChessPiece::ChessPiece() {}

ChessPiece::ChessPiece(int id, Camp camp, PieceType type, QObject *parent) {}

int ChessPiece::pieceId() const {}

Camp ChessPiece::camp() const {}

PieceType ChessPiece::pieceType() const {}

bool ChessPiece::isAlive() const {}

int ChessPiece::x() const {}

int ChessPiece::y() const {}

int ChessPiece::width() const {}

int ChessPiece::height() const {}

int ChessPiece::zValue() const {}

QPoint ChessPiece::logicPos() const {}

void ChessPiece::setLogicPos(int col, int row) {}

void ChessPiece::setX(int val) {}

void ChessPiece::setY(int val) {}

void ChessPiece::setAlive(bool ok) {}
