#ifndef GAME_RUNNER_INCLUDE_H
#define GAME_RUNNER_INCLUDE_H

#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "game_types.h"
#include "game.h"
#include "sound.h"
#include "resource_types.h"
#include "sound.h"
#include "drops_manager.h"
#include "drops_types.h"
#include "pickup_types.h"
#include "rooms\chambers.h"
}

#include "downland_resource_loader_saturn.hpp"
#include "sprite.hpp"
#include "image_utils.hpp"
#include "bitmap_utils.hpp"
#include "palette_utils.hpp"

const char* romFileNames[] = 
{
    "DOWNLAND.BIN",
    "DOWNLAND.ROM",
};
int romFileNamesCount = sizeof(romFileNames) / sizeof(romFileNames[0]);

class GameRunner
{
public:
    GameRunner(Resources* resources)
        : m_resources(resources),
          m_cursor1bppSprite(0xff),
          m_dropSprite(resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT),
          m_ballSprite(resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT),
          m_playerSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT),
          m_playerSplatSprite(resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS, PLAYER_SPLAT_SPRITE_COUNT),
          m_birdSprite(resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT),
	      m_keySprite(resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
	      m_diamondSprite(resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
	      m_moneyBagSprite(resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
          m_cursorSprite(&m_cursor1bppSprite, 8, 1, 1)
    {
        Game_Init(&m_gameData, m_resources);
    }


    void update()
    {
        Game_Update(&m_gameData, m_resources);
    }



    void drawDrops(const GameData* gameData)
    {
        // draw drops
        const Drop* dropsRunner = gameData->dropData.drops;

        for (int loop = 0; loop < NUM_DROPS; loop++)
        {
            if ((s8)dropsRunner->wiggleTimer < 0 || // wiggling
                dropsRunner->wiggleTimer > 1)   // falling
            {
                m_dropSprite.draw(dropsRunner->x << 1,
                                  dropsRunner->y >> 8,
                                  0);
            }

            dropsRunner++;
        }
    }

    void draw()
    {
        drawDrops(&m_gameData);

	    u16 cursorX = m_gameData.numPlayers == 1 ? 32 :128;  // hardcoded locations in the frambuffer

        m_cursorSprite.draw(cursorX, 123, 0);
    }

public:

    GameData m_gameData;
    Resources* m_resources;
    u8 m_cursor1bppSprite;
    Sprite m_dropSprite;
    Sprite m_ballSprite;
    Sprite m_playerSprite;
    Sprite m_playerSplatSprite;
    Sprite m_birdSprite;
    Sprite m_keySprite;
    Sprite m_diamondSprite;
    Sprite m_moneyBagSprite;
    Sprite m_cursorSprite;


};

#endif