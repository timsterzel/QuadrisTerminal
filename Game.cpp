#ifndef GAME_CPP
#define GAME_CPP
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "Game.h"
#include "WorldConstants.h"
#include "InputHelper.h"

using namespace std;
using namespace world_constants;

Game::Game():
m_running(true),
m_draw(true),
m_standardStepTime(1100),
m_currentStepTime(m_standardStepTime),
m_stepTimeFast(50),
m_currentStone(),
m_nextStone(),
m_command('\0'),
m_removedLinesLevel(0),
m_removedLinesTotal(0),
m_level(9),
m_score(0)
{
	m_standardStepTime = (1000 - (m_level * 100)) + 100;
}

void Game::run()
{
	CLOCK::time_point timeStart = CLOCK::now();
	while (m_running)
	{
		/* 
		 * Set the current step time back to its standard so its only
		 * faster as long the player press the specific key
		 */
		m_currentStepTime = m_standardStepTime;
		checkInput();
		if (!m_paused)
		{
			commandReaction();	
			removeFullRows();
			if (isStepTimeLeft(timeStart))
			{
				updateTimeAffected();
				m_draw = true;
				timeStart = CLOCK::now();	
			}	
			if (m_draw)
			{
				draw();	
				m_draw = false;
			}	
		
		}
	}
}

bool Game::isCurrentStoneColliding() const
{
	if (m_currentStone.getLeft() < 0)
		return true;
	if (m_currentStone.getRight() >= (world_constants::FIELD_COLUMN))
		return true;
	if (m_currentStone.getBottom() == world_constants::FIELD_ROW)
		return true;
	bool collidingWithFallen = false;
	for (FallenStone fallenStone : m_fallenStones)
	{
		if(m_currentStone.isCollidingWithPoint(fallenStone.getPosition()))
		{
			collidingWithFallen = true;
		}
		/*
		if (stone.isCollidingWithStone(m_currentStone))
		{
			collidingWithStone = true;
		}	
		*/
	}
	return collidingWithFallen;
}

bool Game::isStepTimeLeft(CLOCK::time_point timeStart) const
{
	CLOCK::time_point now = CLOCK::now();
	chrono::duration<float> timeSpan = (now - timeStart);
	auto milli = chrono::duration_cast<chrono::milliseconds>(timeSpan);
	auto time = milli.count();
	if (time >= m_currentStepTime)
	{
		return true;
	}
	return false;
}

void Game::checkInput()
{
	if (InputHelper::kbhit())
	{
		m_command = InputHelper::getch();		
		if (m_command == 'c')
		{
			m_running = false;
		}
		else if (m_command == '0')
		{
			m_paused = !m_paused;
		}
	}
}

void Game::cleanFullRow(const int row)
{
	
	auto remove_st = remove_if(m_fallenStones.begin(), m_fallenStones.end(),
	[row](FallenStone &fallenStone)
	{
		return fallenStone.getPosition().getIntY() == row; 
	});	
	m_fallenStones.erase(remove_st, m_fallenStones.end());
	// Now move the Stone down which are over the deleted line
	for (FallenStone &fallenStone : m_fallenStones)
	{
		if (fallenStone.getPosition().getIntY() < row)
		{
			fallenStone.moveDown();		
		}			
	}
}

void Game::removeFullRows()
{
	// Count the remove lines so we can later calculate the score
	unsigned int removedLines = 0;
	bool rowDeletion = true;
	while (rowDeletion)
	{
		rowDeletion = false;
		// The Amount of fallen stones in the rows
		unsigned int fallenStonesRow[world_constants::FIELD_ROW] = { 0 };
		for (FallenStone fallenStone : m_fallenStones)
		{
			// Increase the number of the row where the fallen stone is
			(fallenStonesRow[fallenStone.getPosition().getIntY()])++;	
		}
		for (int i = 0; i != world_constants::FIELD_ROW; i++)
		{
		
			int actualRow = i;
			// Remove the stones which are in a full row
			if (fallenStonesRow[actualRow] == world_constants::FIELD_COLUMN)
			{
				cleanFullRow(actualRow);
				// Set the row Deleted to true and break the loop so we can restart
				// checking if a row is full, because by moving down
				// now there can be new full lines
				rowDeletion = true;
				removedLines++;
				break;
			}	
		}	
	}
	if (removedLines > 0)
	{
		m_removedLinesLevel += removedLines;
		m_removedLinesTotal += removedLines;
		if (m_removedLinesLevel >= 10 && m_level <= 10)
		{
			m_level++;
			m_standardStepTime = (1000 - (m_level * 100)) + 100;
			m_currentStepTime = m_standardStepTime;
			// Add the rest lines to the actual level lines
			m_removedLinesLevel = m_removedLinesLevel % 10;
		}
		m_score += (40 * (m_level + 1) * pow(2 , removedLines)); 
	}
	
}

bool Game::isGameOver()
{
	return m_currentStone.getBottom() <= 0;
}

void Game::spawnStone()
{
	m_currentStone.respawn();
}

void Game::updateTimeAffected()
{
	/*
	 * Move down the last added stone, because its
	 * the actual stone which the user can control
	 */
	 m_currentStone.moveDown();
	/*
	 * If the Stone collides with the ground "freeze" the Stone
	 * (Add the Stone to the m_stones container so it is not longer
	 * under the users control.
	  */
	if (isCurrentStoneColliding()) 
	{
		m_currentStone.restoreOldPosition();
		if (isGameOver())
		{
			m_running = false;
		}
		PointF subStones[4];
		m_currentStone.fillWithGlobalPoints(subStones);
		for (PointF subStone : subStones)
		{
			FallenStone fallenStone(subStone, m_currentStone.getShape());
			m_fallenStones.push_back(fallenStone);		
		}
		spawnStone();
		removeFullRows();
	}
}

void Game::commandReaction()
{
	// First check if there was an input
	if (m_command != '\0')
	{
		if (m_command == 'a')
		{
			m_currentStone.moveLeft(); 
		}
		else if (m_command == 'd')
		{
			m_currentStone.moveRight();
		}
		else if (m_command == 's')
		{
			// The stone should fall faster as long the button is pressed
			m_currentStepTime = m_stepTimeFast;
		}
		else if (m_command == 'o')
		{
			m_currentStone.rotateLeft();
		}
		else if (m_command == 'p')
		{
			m_currentStone.rotateRight();
		}
		// Debuging commands
		else if (m_command == '1')
		{
			spawnStone();
		}
		m_command = '\0';	
		// Restore the Stones old Position if it is colliding with something
		if (isCurrentStoneColliding())
		{		
			m_currentStone.restoreOldPosition();
			if (isGameOver())
			{
				m_running = false;
			}
		}
		
		// Something has changed so the field should redraw
		m_draw = true;
	}
}

void Game::clearScreen()
{
	for (int i = 0; i != 50; i++)
	{
		cout << endl;
	}
}

void Game::draw() 
{
	clearScreen();
	cout << "########################################" << endl;
	cout << "1234567890123456789012345678901234567890" << endl;


	for (int i = 0; i != world_constants::SCREEN_HEIGHT; i++)
	{
		for (int j = 0; j != world_constants::SCREEN_WIDTH; j++)
		{
			m_fieldBuffer[i][j] = '*';
		
		}
	}
	
	// Draw the Game field with borders and a ground
	for (int i = FIELD_START_Y; i != FIELD_START_Y + FIELD_WHOLE_HEIGHT; i++)
	{
		for (int j = FIELD_START_X; j != FIELD_START_X + FIELD_WHOLE_WIDTH; j++)
		{	
			// Draw the ground
			if (i == FIELD_START_Y + FIELD_WHOLE_HEIGHT - 1)
			{
				 m_fieldBuffer[i][j] = '#';
			}
			// Draw the borders left and right in the field
			else if (j == FIELD_START_X || j == FIELD_START_X + FIELD_WHOLE_WIDTH - 1)
			{
				m_fieldBuffer[i][j] = '#';			
			}
			// Draw the empty field
			else
			{
				m_fieldBuffer[i][j] =  '.';
			}
		}
		
	}
	// FIELD_START_X + 1 because we have a border with a with of one, 
	// so start after the left border
	m_currentStone.fillFieldBuffer(FIELD_START_X + 1, FIELD_START_Y, m_fieldBuffer);
	

	
	// Draw the fallen Stones
	for (FallenStone fallenStone : m_fallenStones)
	{
		fallenStone.fillFieldBuffer
			(FIELD_START_X + 1, FIELD_START_Y, 
			 m_fieldBuffer);
	}
	/*
	for (int i = 0; i != world_constants::FIELD_ROW; i++)
	{	
		cout << "#";		
		for (int j = 0; j != world_constants::FIELD_COLUMN; j++)
		{
			cout << m_fieldBuffer[i][j];			
		}
		cout << "#";
		cout << endl;
	}
	cout << "############" << endl;
	cout << " Score: " << m_score << endl;  
	cout << " Level: " << m_level << endl;
	cout << " Lines: " << m_removedLinesTotal << endl;
	*/
	// Draw all
	for (int i = 0; i != world_constants::SCREEN_HEIGHT; i++)
	{
		for (int j = 0; j != world_constants::SCREEN_WIDTH; j++)
		{
			cout << m_fieldBuffer[i][j];		
		}
		cout << endl;
	}


	

}


#endif // !GAME_CPP



