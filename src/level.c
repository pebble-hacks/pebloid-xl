//
//  level.c
//
//
//  Created by Dirk Mika on 23.04.13.
//
//

#include <pebble.h>

#include "level.h"
#include "config.h"
#include "block.h"


void level_load(Level *level, uint8_t nr)
{
    uint8_t levelData[LEVEL_NUM_ROWS][LEVEL_DATA_BYTES_PER_ROW];
    ResHandle levelResource = resource_get_handle(RESOURCE_ID_RAW_LEVEL_DATA);
    resource_load_byte_range(levelResource, (nr - 1) * sizeof(levelData), (uint8_t *)levelData, sizeof(levelData));
    
    level->nr = nr;
	level->numBlocksLeft = 0;
	level->numBlocksVisible = 0;
    
    register int row, col;
    register uint8_t dataValue = 0;
	register Block currentBlock;

    for (row = 0; row < LEVEL_NUM_ROWS; row++)
    {
        for (col = 0; col < LEVEL_NUM_COLS; col++)
        {
			if (col % 2 == 0)
			{
	        	dataValue = levelData[row][col >> 1];
			}
			//currentBlock = (dataValue & 0xF0) >> 4;
			currentBlock = dataValue >> 4;
            level->blocks[row][col] = currentBlock;
            if (currentBlock != BlockStateNoBlock)
            {
                level->numBlocksVisible++;
                if (currentBlock != BlockStateUndestroyable)
                {
                    level->numBlocksLeft++;
                }
            }

            //dataValue = (dataValue & 0x0F) << 4;
            dataValue = dataValue << 4;
        }
    }
}
