#ifndef STONE_H
#define STONE_H
#include <vector>
#include "WorldConstants.h"
#include "Point.h" 

/*
 *  A Stone consists of 4 subStones, represented in Points.
 *  The Stones have an own inner coordinate system:
 *          -2
 *      ....|....
 *      ....|....
 *    -4----+----+4
 *      ....|....
 *      ....|.... 
 *          +2
 *  The "+" is the midpoint relative it is always (0|0),
 *  but the absolute position depence from the Stone position in the
 *  Play field.
 */

class Stone
{
private:
	// The position of all substones which result in the whole Stone
	Point m_subStones[4];
	// The absolute position of the midpoint
	Point m_position;
	// The char which is drawn for a point
	const char m_Shape = '#';
public:

	Stone();

	Stone(Point position, Point subStone1, Point subStone2, 
	      Point subStone3, Point subStone4);

	Stone(int midPointX, int midPointY,
	      int subStone1X, int subStone1Y,
	      int subStone2X, int subStone2Y,
	      int subStone3X, int subStone3Y,
	      int subStone4X, int subStone4Y);
		
	void moveDown();
	void moveLeft();
	void moveRight();
	// Store the Stone in the buffer so we can later draw it with
	// all the other Stones
	void fillFieldBuffer(char fieldBuffer[world_constants::FIELD_ROW][world_constants::FIELD_COLUMN]);
	
};

#endif // !STONE_H
