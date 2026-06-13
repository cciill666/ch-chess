#include "chessmanager.h"

ChessManager::ChessManager() {}

ChessManager::ChessManager(QObject *parent) {}

ChessManager::~ChessManager() {}

bool ChessManager::isOnline() const {}

ChessBoard *ChessManager::boardObj() const {}

QList<ChessPiece *> &ChessManager::pieceList() {}

int ChessManager::currentTurn() const {}

ChessPiece *ChessManager::getPieceAt(int col, int row) {}

bool ChessManager::checkMove(ChessPiece *piece, int dstCol, int dstRow) {}

void ChessManager::initChessLayout() {}

void ChessManager::movePiece(ChessPiece *piece, int dstX, int dstY) {}

void ChessManager::enterGame() {}

void ChessManager::startServer(quint16 port) {}

void ChessManager::connectServer(const QString &ip, quint16 port) {}

void ChessManager::sendPieceMove(int id, int x, int y) {}

void ChessManager::syncPieceMove(int id, int x, int y) {}
