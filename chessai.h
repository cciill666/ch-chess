#pragma once

#include <QObject>
#include <QPoint>
#include <QList>
#include "chesspiece.h"
#include "chessboard.h"

// AI走棋结果
struct AiMove {
    int fromId = -1;
    int toCol = -1;
    int toRow = -1;
    int score = 0;
};

class ChessAI : public QObject
{
    Q_OBJECT

public:
    explicit ChessAI(QObject *parent = nullptr);

    // 设置AI阵营（默认黑方）
    void setAiCamp(ChessPiece::Camp camp);
    ChessPiece::Camp aiCamp() const;

    // 计算最佳走法，返回走棋结果
    AiMove findBestMove(const QList<ChessPiece*>& pieces, ChessPiece::Camp currentTurn);

    // 获取某个阵营的所有合法走法
    static QList<AiMove> generateAllMoves(const QList<ChessPiece*>& pieces,
                                          ChessPiece::Camp camp,
                                          bool checkRuleFn(ChessPiece*, int, int, const QList<ChessPiece*>&));

    // 评估函数
    static int evaluateBoard(const QList<ChessPiece*>& pieces);

    // 检查走棋规则
    static bool checkRuleStatic(ChessPiece* piece, int dstCol, int dstRow,
                                const QList<ChessPiece*>& pieces);

private:
    // minimax + alpha-beta 剪枝
    int minimax(QList<ChessPiece*>& pieces, int depth, int alpha, int beta, bool maximizing);

    // 模拟执行一步走棋
    static void applyMove(QList<ChessPiece*>& pieces, const AiMove& move);

    // 模拟撤销一步走棋
    static void undoMove(QList<ChessPiece*>& pieces, const AiMove& move,
                         ChessPiece::Camp capturedCamp, ChessPiece::PieceType capturedType,
                         int capturedId, bool capturedAlive);

    // 棋子基础分值
    static int pieceValue(ChessPiece::PieceType type);

    // 棋子位置加成
    static int positionBonus(ChessPiece::PieceType type, ChessPiece::Camp camp, int col, int row);

    ChessPiece::Camp m_aiCamp = ChessPiece::Camp::Black;
    static const int SEARCH_DEPTH = 3;
};
