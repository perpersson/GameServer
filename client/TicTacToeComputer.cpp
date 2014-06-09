#include <stdlib.h>
#include <limits.h>
#include <algorithm>
#include "TicTacToeComputer.h"

TicTacToeComputer::TicTacToeComputer(SkillLevel skillLevel) :
  skillLevel(skillLevel), crossToMove(true),
  crossPositions(0), noughtPositions(0)
{
}

bool TicTacToeComputer::makeMove(int position)
{
  // Empty position?
  unsigned int positionValue = 1 << position;
  if ((positionValue & (crossPositions | noughtPositions)) == 0)
  {
    if (crossToMove)
      crossPositions |= positionValue;
    else
      noughtPositions |= positionValue;
    crossToMove = !crossToMove;
    return true;
  }
  else
    return false;
}

int TicTacToeComputer::makeComputerMove()
{
  unsigned int emptyPositions[9];
  unsigned int emptyCount = getPossiblePositions(emptyPositions);
  int position;
  switch (skillLevel)
  {
    default:
    case IdiotSkill:
      position = getRandomMove(emptyCount, emptyPositions);
      break;
    case WeakSkill:
      position = getWeakMove(emptyCount, emptyPositions);
      break;
    case NormalSkill:
      position = getNormalMove(emptyCount, emptyPositions);
      break;
    case UnbeatableSkill:
      position = getBestMove(emptyCount, emptyPositions);
      break;
  }
  makeMove(position);
  return position;
}

int TicTacToeComputer::getRandomMove(unsigned int emptyCount,
                                     unsigned int* emptyPositions)
{
  // Make a random move.
  return emptyPositions[rand() % emptyCount];
}

int TicTacToeComputer::getWeakMove(unsigned int emptyCount,
                                   unsigned int* emptyPositions)
{
  int position = getForcedMove(emptyCount, emptyPositions);
  if (position != -1)
    return position;
  else
    return getRandomMove(emptyCount, emptyPositions);
}

int TicTacToeComputer::getNormalMove(unsigned int emptyCount,
                                     unsigned int* emptyPositions)
{
  // First check for any forced move.
  int position = getForcedMove(emptyCount, emptyPositions);
  if (position != -1)
    return position;
  else
  {
    // Prefer center, corner, side in this order.
    unsigned int cornerCount = 0;
    unsigned int corners[4];
    for (unsigned int i=0; i<emptyCount; ++i)
    {
      int position = emptyPositions[i];
      switch (position)
      {
        case 4: return 4;   // Center.
        case 0: case 2: case 6: case 8:
          corners[cornerCount++] = position;
          break;
      }
    }
    // Any corner?
    if (cornerCount != 0)
      return getRandomMove(cornerCount, corners);
    else
      return getRandomMove(emptyCount, emptyPositions);
  }
}

int TicTacToeComputer::getBestMove(unsigned int emptyCount,
                                   unsigned int* emptyPositions)
{
  int bestScore = INT_MIN;
  int bestPosition = 0;
  unsigned int& positionsToUpdate =
    (crossToMove ? crossPositions : noughtPositions);
  for (unsigned int i=0; i<emptyCount; ++i)
  {
    // Make move and check if any winner.
    int position = emptyPositions[i];
    unsigned int positionValue = 1 << position;

    // Win move or search deeper?
    if (isWinningPosition(positionsToUpdate | positionValue))
      return position;

    positionsToUpdate |= positionValue;
    int score = -getNegaMaxScore(!crossToMove);
    if (score > bestScore)
    {
      bestPosition = position;
      bestScore = score;
    }

    // Undo move.
    positionsToUpdate &= ~positionValue;
  }
  return bestPosition;
}

int TicTacToeComputer::getNegaMaxScore(bool crossToMove)
{
  int bestScore = INT_MIN;
  unsigned int& positionsToUpdate =
    (crossToMove ? crossPositions : noughtPositions);
  bool anyPossibleMove = false;
  for (unsigned int position=0; position<9; ++position)
  {
    // Empty position?
    unsigned int positionValue = 1 << position;
    if ((positionValue & (crossPositions | noughtPositions)) == 0)
    {
      anyPossibleMove = true;
      if (isWinningPosition(positionsToUpdate | positionValue))
        return 1;
      positionsToUpdate |= positionValue;
      bestScore = std::max(bestScore, -getNegaMaxScore(!crossToMove));
      positionsToUpdate &= ~positionValue;
    }
  }
  return (anyPossibleMove ? bestScore : 0);
}

unsigned int TicTacToeComputer::getPossiblePositions(unsigned int* positions)
{
  unsigned int count = 0;
  unsigned int usedPositions = crossPositions | noughtPositions;
  for (unsigned int position=0; position<9; ++position)
  {
    unsigned int positionValue = 1 << position;
    if ((usedPositions & positionValue) == 0)
      positions[count++] = position;
  }
  return count;
}

int TicTacToeComputer::getDirectWinningMove(bool crossToMove,
                                            unsigned int emptyCount,
                                            unsigned int* emptyPositions)
{
  // Check if there's a direct win move.
  unsigned int& positionsToCheck =
    (crossToMove ? crossPositions : noughtPositions);
  for (unsigned int i=0; i<emptyCount; ++i)
  {
    int position = emptyPositions[i];
    unsigned int positionValue = 1 << position;
    if (isWinningPosition(positionsToCheck | positionValue))
      return position;
  }
  return -1;   // No direct winning move.
}

bool TicTacToeComputer::isWinningPosition(unsigned int positions) const
{
  // Winning bit patterns:
  // 012, 345, 678, 036, 147, 258, 048, 246
  const unsigned int winningPattern1 = 1 << 0 | 1 << 1 | 1 << 2;
  const unsigned int winningPattern2 = 1 << 3 | 1 << 4 | 1 << 5;
  const unsigned int winningPattern3 = 1 << 6 | 1 << 7 | 1 << 8;
  const unsigned int winningPattern4 = 1 << 0 | 1 << 3 | 1 << 6;
  const unsigned int winningPattern5 = 1 << 1 | 1 << 4 | 1 << 7;
  const unsigned int winningPattern6 = 1 << 2 | 1 << 5 | 1 << 8;
  const unsigned int winningPattern7 = 1 << 0 | 1 << 4 | 1 << 8;
  const unsigned int winningPattern8 = 1 << 2 | 1 << 4 | 1 << 6;
  return ((positions & winningPattern1) == winningPattern1 ||
          (positions & winningPattern2) == winningPattern2 ||
          (positions & winningPattern3) == winningPattern3 ||
          (positions & winningPattern4) == winningPattern4 ||
          (positions & winningPattern5) == winningPattern5 ||
          (positions & winningPattern6) == winningPattern6 ||
          (positions & winningPattern7) == winningPattern7 ||
          (positions & winningPattern8) == winningPattern8);
}

int TicTacToeComputer::getForcedMove(unsigned int emptyCount,
                                     unsigned int* emptyPositions)
{
  // Check if there's a direct winning move.
  int position = getDirectWinningMove(crossToMove, emptyCount, emptyPositions);
  if (position != -1)
    return position;
  else
    // Check if we have to stop opponent from winning.
    return getDirectWinningMove(!crossToMove, emptyCount, emptyPositions);
}
