#include "chessai.h"
#include <QtMath>
#include <QList>
#include <algorithm>
#include <climits>

ChessAI::ChessAI(QObject *parent)
    : QObject(parent)
{
}

void ChessAI::setAiCamp(ChessPiece::Camp camp)
{
    m_aiCamp = camp;
}

ChessPiece::Camp ChessAI::aiCamp() const
{
    return m_aiCamp;
}

// 设定棋子分值，优先吃价值高的棋子

int ChessAI::pieceValue(ChessPiece::PieceType type)
{
    switch (type) {
    case ChessPiece::PieceType::Jiang:
    case ChessPiece::PieceType::Shuai:
        return 10000;
    case ChessPiece::PieceType::Ju:
        return 900;
    case ChessPiece::PieceType::Ma:
        return 400;
    case ChessPiece::PieceType::Pao:
        return 450;
    case ChessPiece::PieceType::Xiang:
    case ChessPiece::PieceType::Shi:
        return 200;
    case ChessPiece::PieceType::Bing:
    case ChessPiece::PieceType::Zu:
        return 100;
    default:
        return 0;
    }
}

// 设置棋子应占据有利位置
int ChessAI::positionBonus(ChessPiece::PieceType type, ChessPiece::Camp camp, int col, int row)
{
    Q_UNUSED(camp)
    int bonus = 0;

    switch (type) {
    case ChessPiece::PieceType::Ju: {
        // 车在中路和列
        bonus += (col >= 3 && col <= 5) ? 10 : 0;
        // 车在对方区域
        if (camp == ChessPiece::Camp::Red && row <= 4) bonus += 20;
        if (camp == ChessPiece::Camp::Black && row >= 5) bonus += 20;
        break;
    }
    case ChessPiece::PieceType::Ma: {
        // 马在中心
        int centerDist = qAbs(col - 4) + qAbs(row - 4);
        bonus += qMax(0, 15 - centerDist * 2);
        break;
    }
    case ChessPiece::PieceType::Pao: {
        // 炮在中间列
        bonus += (col >= 3 && col <= 5) ? 8 : 0;
        break;
    }
    case ChessPiece::PieceType::Bing:
    case ChessPiece::PieceType::Zu: {
        // 兵/卒过河后价值增加
        if (camp == ChessPiece::Camp::Red && row <= 4) bonus += 50;
        if (camp == ChessPiece::Camp::Black && row >= 5) bonus += 50;
        // 兵在中间列价值更高
        if (col >= 3 && col <= 5) bonus += 10;
        break;
    }
    default:
        break;
    }

    return bonus;
}

// 获取某位置的棋子
static ChessPiece *getPieceByPosFromList(const QList<ChessPiece*>& pieces, int col, int row)
{
    for (ChessPiece *piece : pieces) {
        if (piece == nullptr || !piece->isAlive()) continue;
        const QPoint pos = piece->getLogicPos();
        if (pos.x() == col && pos.y() == row) return piece;
    }
    return nullptr;
}

bool ChessAI::checkRuleStatic(ChessPiece *piece, int dstCol, int dstRow,
                              const QList<ChessPiece*>& pieces)
{
    if (piece == nullptr || !piece->isAlive()) return false;
    if (dstCol < 0 || dstCol >= COL_COUNT || dstRow < 0 || dstRow >= ROW_COUNT) return false;

    const QPoint srcPos = piece->getLogicPos();
    const int srcCol = srcPos.x();
    const int srcRow = srcPos.y();
    if (srcCol == dstCol && srcRow == dstRow) return false;

    ChessPiece *target = getPieceByPosFromList(pieces, dstCol, dstRow);
    if (target != nullptr && target->camp() == piece->camp()) return false;

    const int dx = dstCol - srcCol;
    const int dy = dstRow - srcRow;
    const int adx = qAbs(dx);
    const int ady = qAbs(dy);
    const bool isRed = piece->camp() == ChessPiece::Camp::Red;

    switch (piece->pieceType()) {
    case ChessPiece::PieceType::Jiang:
    case ChessPiece::PieceType::Shuai:
        if (dstCol < 3 || dstCol > 5) return false;
        if (isRed && (dstRow < 7 || dstRow > 9)) return false;
        if (!isRed && (dstRow < 0 || dstRow > 2)) return false;
        return (adx == 1 && dy == 0) || (ady == 1 && dx == 0);

    case ChessPiece::PieceType::Shi:
        if (dstCol < 3 || dstCol > 5) return false;
        if (isRed && (dstRow < 7 || dstRow > 9)) return false;
        if (!isRed && (dstRow < 0 || dstRow > 2)) return false;
        return adx == 1 && ady == 1;

    case ChessPiece::PieceType::Xiang: {
        if (adx != 2 || ady != 2) return false;
        if (isRed && dstRow < 5) return false;
        if (!isRed && dstRow > 4) return false;
        const int eyeCol = (srcCol + dstCol) / 2;
        const int eyeRow = (srcRow + dstRow) / 2;
        return getPieceByPosFromList(pieces, eyeCol, eyeRow) == nullptr;
    }

    case ChessPiece::PieceType::Ma: {
        const bool validStep = (adx == 2 && ady == 1) || (adx == 1 && ady == 2);
        if (!validStep) return false;
        const int legCol = srcCol + (adx == 2 ? (dx > 0 ? 1 : -1) : 0);
        const int legRow = srcRow + (ady == 2 ? (dy > 0 ? 1 : -1) : 0);
        return getPieceByPosFromList(pieces, legCol, legRow) == nullptr;
    }

    case ChessPiece::PieceType::Ju: {
        if (dx != 0 && dy != 0) return false;
        const int stepCol = dx == 0 ? 0 : (dx > 0 ? 1 : -1);
        const int stepRow = dy == 0 ? 0 : (dy > 0 ? 1 : -1);
        int c = srcCol + stepCol;
        int r = srcRow + stepRow;
        while (c != dstCol || r != dstRow) {
            if (getPieceByPosFromList(pieces, c, r) != nullptr) return false;
            c += stepCol;
            r += stepRow;
        }
        return true;
    }

    case ChessPiece::PieceType::Pao: {
        if (dx != 0 && dy != 0) return false;
        const int stepCol = dx == 0 ? 0 : (dx > 0 ? 1 : -1);
        const int stepRow = dy == 0 ? 0 : (dy > 0 ? 1 : -1);
        int c = srcCol + stepCol;
        int r = srcRow + stepRow;
        int blockCount = 0;
        while (c != dstCol || r != dstRow) {
            if (getPieceByPosFromList(pieces, c, r) != nullptr) ++blockCount;
            c += stepCol;
            r += stepRow;
        }
        return target == nullptr ? blockCount == 0 : blockCount == 1;
    }

    case ChessPiece::PieceType::Bing:
        if (piece->camp() != ChessPiece::Camp::Red) return false;
        if (srcRow >= 5) return dx == 0 && dy == -1;
        return (dx == 0 && dy == -1) || (ady == 0 && adx == 1);

    case ChessPiece::PieceType::Zu:
        if (piece->camp() != ChessPiece::Camp::Black) return false;
        if (srcRow <= 4) return dx == 0 && dy == 1;
        return (dx == 0 && dy == 1) || (ady == 0 && adx == 1);

    default:
        return false;
    }
}

//走法生成

QList<AiMove> ChessAI::generateAllMoves(const QList<ChessPiece*>& pieces,
                                        ChessPiece::Camp camp,
                                        bool checkRuleFn(ChessPiece*, int, int, const QList<ChessPiece*>&))
{
    QList<AiMove> moves;
    for (int dstCol = 0; dstCol < COL_COUNT; ++dstCol) {
        for (int dstRow = 0; dstRow < ROW_COUNT; ++dstRow) {
            for (ChessPiece *piece : pieces) {
                if (piece == nullptr || !piece->isAlive()) continue;
                if (piece->camp() != camp) continue;

                const QPoint srcPos = piece->getLogicPos();
                if (srcPos.x() == dstCol && srcPos.y() == dstRow) continue;

                if (checkRuleFn(piece, dstCol, dstRow, pieces)) {
                    AiMove move;
                    move.fromId = piece->pieceId();
                    move.toCol = dstCol;
                    move.toRow = dstRow;
                    // 优先考虑吃子走法
                    ChessPiece *target = getPieceByPosFromList(pieces, dstCol, dstRow);
                    move.score = target ? pieceValue(target->pieceType()) : 0;
                    moves.append(move);
                }
            }
        }
    }
    return moves;
}

//走法执行

void ChessAI::applyMove(QList<ChessPiece*>& pieces, const AiMove& move)
{
    ChessPiece *mover = nullptr;
    for (ChessPiece *p : pieces) {
        if (p && p->isAlive() && p->pieceId() == move.fromId) {
            mover = p;
            break;
        }
    }
    if (mover) {
        mover->setLogicPos(move.toCol, move.toRow);
    }

    // 处理吃子
    ChessPiece *target = getPieceByPosFromList(pieces, move.toCol, move.toRow);
    if (target && target != mover) {
        target->setAlive(false);
    }
}

void ChessAI::undoMove(QList<ChessPiece*>& pieces, const AiMove& move,
                       ChessPiece::Camp capturedCamp, ChessPiece::PieceType capturedType,
                       int capturedId, bool capturedAlive)
{
    // 恢复被吃的棋子
    if (!capturedAlive) {
        for (ChessPiece *p : pieces) {
            if (p && p->pieceId() == capturedId) {
                p->setAlive(true);
                break;
            }
        }
    }

    // 恢复移动棋子的位置
    ChessPiece *mover = nullptr;

}

//评估函数

int ChessAI::evaluateBoard(const QList<ChessPiece*>& pieces)
{
    int score = 0;
    bool redGeneralAlive = false;
    bool blackGeneralAlive = false;

    for (ChessPiece *piece : pieces) {
        if (piece == nullptr || !piece->isAlive()) continue;

        const ChessPiece::PieceType type = piece->pieceType();
        const ChessPiece::Camp camp = piece->camp();
        const QPoint pos = piece->getLogicPos();
        int val = pieceValue(type) + positionBonus(type, camp, pos.x(), pos.y());

        if (type == ChessPiece::PieceType::Jiang) redGeneralAlive = true;
        if (type == ChessPiece::PieceType::Shuai) blackGeneralAlive = true;

        if (camp == ChessPiece::Camp::Red) {
            score += val;
        } else {
            score -= val;
        }
    }

    // 将/帅被吃则直接返回
    if (!redGeneralAlive) return -100000;
    if (!blackGeneralAlive) return 100000;

    return score;
}

//Minimax搜索

int ChessAI::minimax(QList<ChessPiece*>& pieces, int depth, int alpha, int beta, bool maximizing)
{
    // 终止条件
    if (depth == 0) {
        return evaluateBoard(pieces);
    }

    ChessPiece::Camp camp = maximizing ? ChessPiece::Camp::Red : ChessPiece::Camp::Black;
    QList<AiMove> moves = generateAllMoves(pieces, camp, checkRuleStatic);

    if (moves.isEmpty()) {
        return evaluateBoard(pieces);
    }

    // 走法排序：吃子走法优先
    std::sort(moves.begin(), moves.end(), [](const AiMove& a, const AiMove& b) {
        return a.score > b.score;
    });

    if (maximizing) {
        int maxEval = INT_MIN;
        for (const AiMove& move : moves) {
            // 记录状态用于撤销
            ChessPiece *mover = nullptr;
            QPoint origPos;
            ChessPiece *captured = nullptr;
            ChessPiece::Camp capturedCamp = ChessPiece::Camp::Red;
            ChessPiece::PieceType capturedType = ChessPiece::PieceType::Ju;
            int capturedId = -1;
            bool hadCaptured = false;

            for (ChessPiece *p : pieces) {
                if (p && p->isAlive() && p->pieceId() == move.fromId) {
                    mover = p;
                    origPos = p->getLogicPos();
                    break;
                }
            }

            captured = getPieceByPosFromList(pieces, move.toCol, move.toRow);
            if (captured && captured != mover) {
                hadCaptured = true;
                capturedCamp = captured->camp();
                capturedType = captured->pieceType();
                capturedId = captured->pieceId();
            }

            // 执行
            applyMove(pieces, move);

            int eval = minimax(pieces, depth - 1, alpha, beta, false);
            maxEval = qMax(maxEval, eval);
            alpha = qMax(alpha, eval);

            // 撤销
            mover->setLogicPos(origPos.x(), origPos.y());
            if (hadCaptured) {
                for (ChessPiece *p : pieces) {
                    if (p && p->pieceId() == capturedId) {
                        p->setAlive(true);
                        break;
                    }
                }
            }

            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (const AiMove& move : moves) {
            ChessPiece *mover = nullptr;
            QPoint origPos;
            ChessPiece *captured = nullptr;
            ChessPiece::Camp capturedCamp = ChessPiece::Camp::Red;
            ChessPiece::PieceType capturedType = ChessPiece::PieceType::Ju;
            int capturedId = -1;
            bool hadCaptured = false;

            for (ChessPiece *p : pieces) {
                if (p && p->isAlive() && p->pieceId() == move.fromId) {
                    mover = p;
                    origPos = p->getLogicPos();
                    break;
                }
            }

            captured = getPieceByPosFromList(pieces, move.toCol, move.toRow);
            if (captured && captured != mover) {
                hadCaptured = true;
                capturedCamp = captured->camp();
                capturedType = captured->pieceType();
                capturedId = captured->pieceId();
            }

            applyMove(pieces, move);

            int eval = minimax(pieces, depth - 1, alpha, beta, true);
            minEval = qMin(minEval, eval);
            beta = qMin(beta, eval);

            mover->setLogicPos(origPos.x(), origPos.y());
            if (hadCaptured) {
                for (ChessPiece *p : pieces) {
                    if (p && p->pieceId() == capturedId) {
                        p->setAlive(true);
                        break;
                    }
                }
            }

            if (beta <= alpha) break;
        }
        return minEval;
    }
}

//寻找最佳走法

AiMove ChessAI::findBestMove(const QList<ChessPiece*>& pieces, ChessPiece::Camp currentTurn)
{
    QList<AiMove> moves = generateAllMoves(pieces, currentTurn, checkRuleStatic);

    if (moves.isEmpty()) {
        return AiMove();
    }

    // 走法排序
    std::sort(moves.begin(), moves.end(), [](const AiMove& a, const AiMove& b) {
        return a.score > b.score;
    });

    bool maximizing = (m_aiCamp == ChessPiece::Camp::Red);
    int bestScore = maximizing ? INT_MIN : INT_MAX;
    AiMove bestMove = moves.first();

    for (const AiMove& move : moves) {
        // 复制pieces列表用于模拟
        QList<ChessPiece*> simPieces;
        for (ChessPiece *p : pieces) {
            simPieces.append(p);
        }

        ChessPiece *mover = nullptr;
        QPoint origPos;
        ChessPiece *captured = nullptr;
        bool hadCaptured = false;
        int capturedId = -1;

        for (ChessPiece *p : simPieces) {
            if (p && p->isAlive() && p->pieceId() == move.fromId) {
                mover = p;
                origPos = p->getLogicPos();
                break;
            }
        }

        captured = getPieceByPosFromList(simPieces, move.toCol, move.toRow);
        if (captured && captured != mover) {
            hadCaptured = true;
            capturedId = captured->pieceId();
        }

        applyMove(simPieces, move);

        int score = minimax(simPieces, SEARCH_DEPTH - 1, INT_MIN, INT_MAX, !maximizing);

        mover->setLogicPos(origPos.x(), origPos.y());
        if (hadCaptured) {
            for (ChessPiece *p : simPieces) {
                if (p && p->pieceId() == capturedId) {
                    p->setAlive(true);
                    break;
                }
            }
        }

        if (maximizing) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }

    bestMove.score = bestScore;
    return bestMove;
}
