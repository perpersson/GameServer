#ifndef TICTACTOE_COMPUTER_H
#define TICTACTOE_COMPUTER_H

class TicTacToeComputer
{
 public:
  enum SkillLevel
  {
    IdiotSkill,
    WeakSkill,
    NormalSkill,
    UnbeatableSkill
  };

  TicTacToeComputer(SkillLevel skillLevel = UnbeatableSkill);
  bool makeMove(int position);
  int makeComputerMove();

 private:
  SkillLevel skillLevel;
  bool crossToMove;
  unsigned int crossPositions;
  unsigned int noughtPositions;

  int getRandomMove(unsigned int emptyCount, unsigned int* emptyPositions);
  int getWeakMove(unsigned int emptyCount, unsigned int* emptyPositions);
  int getNormalMove(unsigned int emptyCount, unsigned int* emptyPositions);
  int getBestMove(unsigned int emptyCount, unsigned int* emptyPositions);
  int getNegaMaxScore(bool crossToMove);

  unsigned int getPossiblePositions(unsigned int* positions);
  int getDirectWinningMove(bool crossToMove, unsigned int emptyCount,
                           unsigned int* emptyPositions);
  bool isWinningPosition(unsigned int positions) const;
  int getForcedMove(unsigned int emptyCount, unsigned int* emptyPositions);
};

#endif
