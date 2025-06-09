#ifndef GAME_RUNNER_INCLUDE_H
#define GAME_RUNNER_INCLUDE_H

#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "game_types.h"
#include "door_types.h"
#include "game.h"
#include "sound.h"
#include "resource_types.h"
#include "sound.h"
#include "drops_manager.h"
#include "drops_types.h"
#include "pickup_types.h"
#include "rooms\chambers.h"
#include "dl_rand.h"
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
          m_cursorSprite(&m_cursor1bppSprite, 8, 1, 1),
          m_regenSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 8),
          m_characterFont(resources->characterFont, 8, 7, 39),
          m_doorSprite(resources->sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1),
          m_splatSprite(resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS),
          m_regenSpriteIndex(0),
          m_layerInitialized(false),
          m_roomChanged(false),
          m_currentRoom(-1)
    {
        m_drawRoomFunctions = { &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawChamber,
                                &GameRunner::drawTitleScreen,
                                &GameRunner::drawTransition,
                                &GameRunner::drawWipeTransition,
                                &GameRunner::drawGetReadyScreen };

        m_pickUpSprites[0] = &m_diamondSprite;
        m_pickUpSprites[1] = &m_moneyBagSprite;
        m_pickUpSprites[2] = &m_keySprite;

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

    void handleRoomChange()
    {
        SRL::VDP2::NBG0::ScrollDisable();//enable display of NBG0 

        u8* frameBuffer8bpp = new u8[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

        ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(m_gameData.cleanBackground,
                                                                         frameBuffer8bpp,
                                                                         FRAMEBUFFER_WIDTH,
                                                                         FRAMEBUFFER_HEIGHT,
                                                                         ImageUtils::ImageConverter::CrtColor::Blue);

        BitmapUtils::InMemoryBitmap* framebufferBitmap = new BitmapUtils::InMemoryBitmap(frameBuffer8bpp, 
                                                                                         FRAMEBUFFER_WIDTH, 
                                                                                         FRAMEBUFFER_HEIGHT, 
                                                                                         PaletteUtils::g_downlandPalette, 
                                                                                         PaletteUtils::g_downlandPaletteColorsCount);

        SRL::Tilemap::Interfaces::Bmp2Tile* tileSet = new SRL::Tilemap::Interfaces::Bmp2Tile(*framebufferBitmap);

        if (!m_layerInitialized)
        {
            // init the whole layer
            SRL::VDP2::NBG0::LoadTilemap(*tileSet);
            m_layerInitialized = true;

            SRL::VDP2::NBG0::SetPriority(SRL::VDP2::Priority::Layer2);//set NBG0 priority
        }
        else
        {
            // we can't easily reset just the layer, so
            // just replace the tiles and map directly
            Cell2VRAM((uint8_t*)tileSet->GetCellData(), 
                      SRL::VDP2::NBG0::CellAddress, 
                      SRL::VDP2::NBG0::Info.CellByteSize);

            Map2VRAM(SRL::VDP2::NBG0::Info,
                     (uint16_t*)tileSet->GetMapData(),
                     SRL::VDP2::NBG0::MapAddress,
                     SRL::VDP2::NBG0::TilePalette.GetId(),
                     SRL::VDP2::NBG0::GetCellOffset(SRL::VDP2::NBG0::Info, SRL::VDP2::NBG0::CellAddress));
        }


        delete tileSet;//free work RAM
        delete [] frameBuffer8bpp;
        delete framebufferBitmap;

        SRL::VDP2::NBG0::ScrollEnable();//enable display of NBG0 
    }

    void draw()
    {
        if (m_roomChanged)
        {
            handleRoomChange();
            m_roomChanged = false;
        }
        else
        {
            (this->*m_drawRoomFunctions[m_gameData.currentRoom->roomNumber])();
        }
    }

    inline void Cell2VRAM(uint8_t* cellData, 
                          void* cellAdr, 
                          uint32_t size)
    {
        uint8_t* VRAM = (uint8_t*)cellAdr;

        // Note: Consider DMA
        for (uint32_t i = 0; i < size; i++) *(VRAM++) = *(cellData++);
    }

    inline void Map2VRAM(SRL::Tilemap::TilemapInfo& info, 
                         uint16_t* mapData, 
                         void* mapAdr, 
                         uint8_t paloff, 
                         uint32_t mapoff)
    {
        uint16_t* VRAM = (uint16_t*)mapAdr;
        uint32_t* VRAM32 = (uint32_t*)mapAdr;
        uint32_t* Data32 = (uint32_t*)mapData;

        for (uint16_t i = 0; i < info.MapHeight; i++)
        {
            for (uint16_t j = 0; j < info.MapWidth; j++)
            {
                if (info.MapMode) *VRAM++ = ((*mapData++) + mapoff) | (paloff << 12); // 1WORD data
                else *VRAM32++ = ((*Data32++) + mapoff) | (paloff << 20); // 2WORD data
            }
        }
    }

    void roomChanged(const GameData* gameData, u8 roomNumber, s8 transitionType)
    {
        if (m_currentRoom != roomNumber)
        {
            m_roomChanged = true;
            m_currentRoom = roomNumber;
        }
    }

private:

    void drawChamber()
    {
        /*
        // Update texture from gameFramebuffer
        SDLUtils_convert1bppImageTo32bppCrtEffectImage(m_gameData.cleanBackground,
                                                       framebuffer,
                                                       FRAMEBUFFER_WIDTH,
                                                       FRAMEBUFFER_HEIGHT,
                                                       CrtColor::Blue);
        */

        // draw drops


        drawDrops(&m_gameData);

        const PlayerData* playerData = m_gameData.currentPlayerData;

        u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

        const Pickup* pickups = playerData->gamePickups[m_gameData.currentRoom->roomNumber];
        for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
        {
		    if ((pickups->state & playerData->playerMask))
		    {
                m_pickUpSprites[pickups->type]->draw(pickups->x << 1, 
                                                     pickups->y, 
                                                     0);

                /*
                drawSprite(framebuffer, 
                            FRAMEBUFFER_WIDTH, 
                            FRAMEBUFFER_HEIGHT,
                            pickups->x << 1,
                            pickups->y,
                            0,
                            m_pickUpSprites[pickups->type]);
                            */
            }

            pickups++;
        }


        switch (playerData->state)
        {
        case PLAYER_STATE_SPLAT: 
            m_splatSprite.draw((playerData->x >> 8) << 1,
                               (playerData->y >> 8) + 7,
                               playerData->splatFrameNumber);

            /*
            drawSprite(framebuffer, 
                        FRAMEBUFFER_WIDTH, 
                        FRAMEBUFFER_HEIGHT,
                        (playerData->x >> 8) << 1,
                        (playerData->y >> 8) + 7,
                        playerData->splatFrameNumber,
                        &m_playerSplatSprite);
            */
            break;
        case PLAYER_STATE_REGENERATION: 

            if (!m_gameData.paused)
            {
                m_regenSpriteIndex = dl_rand() % m_regenSprite.m_numFrames;
            }

            m_regenSprite.draw((playerData->x >> 8) << 1,
                                playerData->y >> 8,
                                m_regenSpriteIndex + (playerData->facingDirection ? 0 : m_regenSprite.m_numFrames)); // PLAYER_SPRITE_LEFT_STAND
            break;
        default: 
            /*
            drawSprite(framebuffer, 
                        FRAMEBUFFER_WIDTH, 
                        FRAMEBUFFER_HEIGHT,
                        (playerData->x >> 8) << 1,
                        playerData->y >> 8,
                        playerData->currentSpriteNumber,
                        &m_playerSprite);
            */

            m_playerSprite.draw((playerData->x >> 8) << 1,
                                playerData->y >> 8,
                                playerData->currentSpriteNumber);
        }

        // draw ball
        if (m_gameData.ballData.enabled)
        {
            const BallData* ballData = &m_gameData.ballData;

            m_ballSprite.draw((ballData->x >> 8) << 1,
                              ballData->y >> 8,
                              ((s8)ballData->fallStateCounter < 0));

            /*
            drawSprite(framebuffer, 
                        FRAMEBUFFER_WIDTH, 
                        FRAMEBUFFER_HEIGHT,
                        (ballData->x >> 8) << 1,
                        ballData->y >> 8,
                        ((s8)ballData->fallStateCounter < 0),
                        &m_ballSprite);
            */

        }

        // draw bird
        if (m_gameData.birdData.state && currentTimer == 0)
        {
            const BirdData* birdData = &m_gameData.birdData;

            m_birdSprite.draw((birdData->x >> 8) << 1,
                              birdData->y >> 8,
                              birdData->animationFrame);

            /*
            drawSprite(framebuffer, 
                        FRAMEBUFFER_WIDTH, 
                        FRAMEBUFFER_HEIGHT,
                        (birdData->x >> 8) << 1,
                        birdData->y >> 8,
                        birdData->animationFrame,
                        &m_birdSprite);
            */
        }

        // draw doors
        int roomNumber = m_gameData.currentRoom->roomNumber;
		DoorInfoData* doorInfoData = &m_resources->roomResources[roomNumber].doorInfoData;
		const DoorInfo* doorInfoRunner = doorInfoData->doorInfos;

		for (u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
		{
            if (!playerData->doorStateData[doorInfoRunner->globalDoorIndex] & playerData->playerMask)
                continue;

            int xPosition = doorInfoRunner->x;
	        // adjust the door position, as per the original game.
	        if (xPosition > 40) 
		        xPosition += 7;
	        else
		        xPosition -= 4;

            m_doorSprite.draw(xPosition << 1, 
                              doorInfoRunner->y,
                              0);

			doorInfoRunner++;
		}



        /*
        // draw player lives icons
        drawPlayerLives(m_gameData.currentPlayerData->lives,
                        playerData->currentSpriteNumber,
                        &m_playerSprite,
                        &m_regenSprite,
                        playerData->state == PLAYER_STATE_REGENERATION,
                        framebuffer);



        // draw timer
	    drawText(framebuffer,
                    FRAMEBUFFER_WIDTH,
                    FRAMEBUFFER_HEIGHT,
                    m_gameData.string_timer, 
			        &m_characterFont, 
                    TIMER_DRAW_LOCATION);

        // draw player text
	    drawText(framebuffer,
                    FRAMEBUFFER_WIDTH,
                    FRAMEBUFFER_HEIGHT,
                    m_resources->text_pl1, 
			        &m_characterFont, 
                    PLAYERLIVES_TEXT_DRAW_LOCATION);

        // draw chamber text
	    drawText(framebuffer,
                    FRAMEBUFFER_WIDTH,
                    FRAMEBUFFER_HEIGHT,
                    m_resources->text_chamber, 
			        &m_characterFont, 
			        CHAMBER_TEXT_DRAW_LOCATION);

        // room number chamber text
	    drawText(framebuffer,
                    FRAMEBUFFER_WIDTH,
                    FRAMEBUFFER_HEIGHT,
                    m_gameData.string_roomNumber, 
			        &m_characterFont, 
			        CHAMBER_NUMBER_TEXT_DRAW_LOCATION);

        // draw score
	    drawText(framebuffer,
                    FRAMEBUFFER_WIDTH,
                    FRAMEBUFFER_HEIGHT,
                    playerData->scoreString, 
			        &m_characterFont, 
			        SCORE_DRAW_LOCATION);
        */
    }

    void drawTitleScreen()
    {
        /*
        SDLUtils_convert1bppImageTo32bppCrtEffectImage(m_gameData.cleanBackground,
                                                       framebuffer,
                                                       FRAMEBUFFER_WIDTH,
                                                       FRAMEBUFFER_HEIGHT,
                                                       CrtColor::Blue);
        // draw drops
        drawDrops(gameData, framebuffer);
        */

        drawDrops(&m_gameData);

	    u16 cursorX = m_gameData.numPlayers == 1 ? 32 :128;  // hardcoded locations in the frambuffer

        m_cursorSprite.draw(cursorX, 123, 0);

    }

    void drawTransition()
    {
	    //memset(framebuffer, 0, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(u32));

        /*
        u8* frameBuffer8bpp = new u8[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

        ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(m_gameData.cleanBackground,
                                                                         frameBuffer8bpp,
                                                                         FRAMEBUFFER_WIDTH,
                                                                         FRAMEBUFFER_HEIGHT,
                                                                         ImageUtils::ImageConverter::CrtColor::Blue);

        BitmapUtils::InMemoryBitmap* framebufferBitmap = new BitmapUtils::InMemoryBitmap(frameBuffer8bpp, 
                                                                                         FRAMEBUFFER_WIDTH, 
                                                                                         FRAMEBUFFER_HEIGHT, 
                                                                                         PaletteUtils::g_downlandPalette, 
                                                                                         PaletteUtils::g_downlandPaletteColorsCount);

        SRL::Tilemap::Interfaces::Bmp2Tile* tileSet = new SRL::Tilemap::Interfaces::Bmp2Tile(*framebufferBitmap);
        SRL::VDP2::NBG0::LoadTilemap(*tileSet);    

        delete tileSet;//free work RAM
        delete [] frameBuffer8bpp;
        delete framebufferBitmap;
        */
    }



    void drawWipeTransition()
    {
        /*
        // not the most efficient as it updates the whole framebuffer
        // instead of what changed per frame during the wipe, but at the 
        // moment we're not worried about performance. Different platforms
        // will have to handle this in different ways.
	    for (int sectionCounter = 0; sectionCounter < 6; sectionCounter++)
	    {
            u32 offset = (sectionCounter * 32 * FRAMEBUFFER_WIDTH);
	        u32* framebufferRunner = framebuffer + offset;
	        u32* wipeFramebufferRunner = m_wipeFramebuffer + offset;

            for (int lineCounter = 0; lineCounter < m_gameData.transitionCurrentLine; lineCounter++)
            {
		        for (int innerLoop = 0; innerLoop < FRAMEBUFFER_WIDTH; innerLoop++)
		        {
			        *framebufferRunner = *wipeFramebufferRunner;

			        // draw a solid line underneath the pixel
				    *(framebufferRunner + FRAMEBUFFER_WIDTH) = 0x000000ff;

			        framebufferRunner++;
			        wipeFramebufferRunner++;
		        }
            }
	    }
        */
    }

    void drawGetReadyScreen()
    {
        /*
        SDLUtils_convert1bppImageTo32bppCrtEffectImage(m_gameData.cleanBackground,
                                                       framebuffer,
                                                       FRAMEBUFFER_WIDTH,
                                                       FRAMEBUFFER_HEIGHT,
                                                       CrtColor::Blue);
        */

        drawDrops(&m_gameData);
    }


public:
    GameData m_gameData;

private:
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
	RegenSprite m_regenSprite;
	Sprite m_characterFont;
    Sprite m_doorSprite;
    SplatSprite m_splatSprite;

    Sprite* m_pickUpSprites[3];

    int m_regenSpriteIndex;

	typedef void (GameRunner::*DrawRoomFunction)();
	std::vector<DrawRoomFunction> m_drawRoomFunctions;

    u8 m_regenSpriteBuffer[(PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS];

    bool m_layerInitialized;
    bool m_roomChanged;
    u8 m_currentRoom;
};

#endif