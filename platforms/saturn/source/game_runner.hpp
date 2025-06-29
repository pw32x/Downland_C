#ifndef GAME_RUNNER_INCLUDE_H
#define GAME_RUNNER_INCLUDE_H

#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "game_types.h"
#include "door_types.h"
#include "game.h"
#include "dl_sound.h"
#include "resource_types.h"
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
#include "logger.hpp"

#define REGEN_SPRITES 5

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
          m_regenSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_ROWS, REGEN_SPRITES),
          m_characterFont(resources->characterFont, 8, 7, 39),
          m_doorSprite(resources->sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1),
          m_splatSprite(resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS),
          m_playerIconSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT),
          m_regenPlayerIconSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, REGEN_SPRITES),
          m_regenSpriteIndex(0),
          m_nbg0Initialized(false),
          m_nbg1Initialized(false),
          m_currentRoom(-1),
          m_isNBG0Active(true)
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

        memset(&m_gameData, 0, sizeof(GameData));
        Game_Init(&m_gameData, m_resources);


        //g_rooms[WIPE_TRANSITION_ROOM_INDEX] = g_rooms[TRANSITION_ROOM_INDEX];
    }


    void update()
    {
//        m_gameData.transitionInitialDelay = 0;
        Game_Update(&m_gameData, m_resources);
    }

    void drawDrops(const GameData* gameData)
    {
        // draw drops
        const Drop* dropsRunner = gameData->dropData.drops;

        for (int loop = 0; loop < NUM_DROPS; loop++)
        {
            if ((dl_s8)dropsRunner->wiggleTimer < 0 || // wiggling
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
        (this->*m_drawRoomFunctions[m_gameData.currentRoom->roomNumber])();
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

    void roomTransitionDone(const GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType)
    {
        // swap the active background for the next 
        // wipe transition.
        m_isNBG0Active = !m_isNBG0Active;
    }

private:

    void drawPlayerLives()
    {
        const PlayerData* playerData = m_gameData.currentPlayerData;

	    dl_u8 x = PLAYERLIVES_ICON_X;
	    dl_u8 y = PLAYERLIVES_ICON_Y;

        for (dl_u8 loop = 0; loop < playerData->lives; loop++)
	    {
            m_playerIconSprite.draw(x << 1, y, playerData->currentSpriteNumber);
		    x += PLAYERLIVES_ICON_SPACING;
        }

	    if (playerData->state == PLAYER_STATE_REGENERATION)
	    {
            m_regenPlayerIconSprite.draw(x << 1, 
                                         y, 
                                         m_regenSpriteIndex + (playerData->facingDirection ? 0 : m_regenPlayerIconSprite.getNumFrames())); // PLAYER_SPRITE_LEFT_STAND
        }
    }

	void drawText(const dl_u8* text, dl_u16 xyLocation)
    {
        dl_u16 x = ((xyLocation % 32) * 8);
        dl_u16 y = (xyLocation / 32);

        // for each character
        while (*text != 0xff)
        {
            m_characterFont.draw(x, y, *text);
            text++;
            x += 8;
        }
    }

    void drawChamber()
    {
        SRL::Math::Types::Vector2D scrollOffset = SRL::Math::Types::Vector2D(-SCREEN_OFFSET_X, -SCREEN_OFFSET_Y);
        SRL::VDP2::NBG0::SetPosition(scrollOffset);
        SRL::VDP2::NBG1::SetPosition(scrollOffset);

        drawDrops(&m_gameData);

        const PlayerData* playerData = m_gameData.currentPlayerData;

        dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

        const Pickup* pickups = playerData->gamePickups[m_gameData.currentRoom->roomNumber];
        for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
        {
		    if ((pickups->state & playerData->playerMask))
		    {
                m_pickUpSprites[pickups->type]->draw(pickups->x << 1, 
                                                     pickups->y, 
                                                     0);
            }

            pickups++;
        }


        switch (playerData->state)
        {
        case PLAYER_STATE_SPLAT: 
            m_splatSprite.draw((playerData->x >> 8) << 1,
                               (playerData->y >> 8) + 7,
                               playerData->splatFrameNumber);
            break;
        case PLAYER_STATE_REGENERATION: 

            if (!m_gameData.paused)
            {
                m_regenSpriteIndex = dl_rand() % m_regenSprite.getNumFrames();
            }

            m_regenSprite.draw((playerData->x >> 8) << 1,
                                playerData->y >> 8,
                                m_regenSpriteIndex + (playerData->facingDirection ? 0 : m_regenSprite.getNumFrames())); // PLAYER_SPRITE_LEFT_STAND
            break;
        default: 
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
                              ((dl_s8)ballData->fallStateCounter < 0));
        }

        // draw bird
        if (m_gameData.birdData.state && currentTimer == 0)
        {
            const BirdData* birdData = &m_gameData.birdData;

            m_birdSprite.draw((birdData->x >> 8) << 1,
                              birdData->y >> 8,
                              birdData->animationFrame);
        }

        // draw doors
        int roomNumber = m_gameData.currentRoom->roomNumber;
		const DoorInfoData* doorInfoData = &m_resources->roomResources[roomNumber].doorInfoData;
		const DoorInfo* doorInfoRunner = doorInfoData->doorInfos;

		for (dl_u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
		{
            if (playerData->doorStateData[doorInfoRunner->globalDoorIndex] & playerData->playerMask &&
                doorInfoRunner->x != 0xff)
            {
                int xPosition = doorInfoRunner->x;
	            // adjust the door position, as per the original game.
	            if (xPosition > 40) 
		            xPosition += 7;
	            else
		            xPosition -= 4;

                m_doorSprite.draw(xPosition << 1, 
                                  doorInfoRunner->y,
                                  0);
            }

			doorInfoRunner++;
		}

        // draw player lives icons
        drawPlayerLives();

        // draw text
	    drawText(m_gameData.string_timer, TIMER_DRAW_LOCATION);
	    drawText(m_resources->text_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
	    drawText(m_resources->text_chamber, CHAMBER_TEXT_DRAW_LOCATION);
	    drawText(m_gameData.string_roomNumber, CHAMBER_NUMBER_TEXT_DRAW_LOCATION);
	    drawText(playerData->scoreString, SCORE_DRAW_LOCATION);        
    }

    void drawTitleScreen()
    {
        SRL::Math::Types::Vector2D scrollOffset = SRL::Math::Types::Vector2D(-SCREEN_OFFSET_X, -SCREEN_OFFSET_Y);
        SRL::VDP2::NBG0::SetPosition(scrollOffset);
        SRL::VDP2::NBG1::SetPosition(scrollOffset);

        drawDrops(&m_gameData);

	    dl_u16 cursorX = m_gameData.numPlayers == 1 ? 32 :128;  // hardcoded locations in the frambuffer

        m_cursorSprite.draw(cursorX, 123, 0);
    }

    void drawTransition()
    {
        if (m_gameData.transitionInitialDelay == 29)
        {
            if (m_isNBG0Active)
                clearBackgroundNBG0();
            else
                clearBackgroundNBG1();
        }
        else if (!m_gameData.transitionInitialDelay)
        {
            if (m_isNBG0Active)
                drawCleanBackgroundToNBG0();
            else
                drawCleanBackgroundToNBG1();
        }
    }

    void clearBackgroundNBG0()
    {
        SRL::VDP2::NBG0::ScrollDisable();//enable display of NBG0 

        dl_u8* frameBuffer8bpp = new dl_u8[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

        memset(frameBuffer8bpp, 0, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);

        BitmapUtils::InMemoryBitmap* framebufferBitmap = new BitmapUtils::InMemoryBitmap(frameBuffer8bpp, 
                                                                                         FRAMEBUFFER_WIDTH, 
                                                                                         FRAMEBUFFER_HEIGHT, 
                                                                                         PaletteUtils::g_downlandPalette, 
                                                                                         PaletteUtils::g_downlandPaletteColorsCount);

        SRL::Tilemap::Interfaces::Bmp2Tile* tileSet = new SRL::Tilemap::Interfaces::Bmp2Tile(*framebufferBitmap);

        if (!m_nbg0Initialized)
        {
            // init the whole layer
            SRL::VDP2::NBG0::LoadTilemap(*tileSet);
            m_nbg0Initialized = true;

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

        SRL::VDP2::NBG0::ScrollEnable();
    }


    void clearBackgroundNBG1()
    {
        SRL::VDP2::NBG0::ScrollDisable();//enable display of NBG0 

        dl_u8* frameBuffer8bpp = new dl_u8[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

        memset(frameBuffer8bpp, 0, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);

        BitmapUtils::InMemoryBitmap* framebufferBitmap = new BitmapUtils::InMemoryBitmap(frameBuffer8bpp, 
                                                                                         FRAMEBUFFER_WIDTH, 
                                                                                         FRAMEBUFFER_HEIGHT, 
                                                                                         PaletteUtils::g_downlandPalette, 
                                                                                         PaletteUtils::g_downlandPaletteColorsCount);

        SRL::Tilemap::Interfaces::Bmp2Tile* tileSet = new SRL::Tilemap::Interfaces::Bmp2Tile(*framebufferBitmap);

        if (!m_nbg1Initialized)
        {
            // init the whole layer
            SRL::VDP2::NBG0::LoadTilemap(*tileSet);
            m_nbg1Initialized = true;

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

        SRL::VDP2::NBG0::ScrollEnable();
    }

    void drawCleanBackgroundToNBG0()
    {
        SRL::VDP2::NBG0::ScrollDisable();//enable display of NBG0 

        dl_u8* frameBuffer8bpp = new dl_u8[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

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

        if (!m_nbg0Initialized)
        {
            // init the whole layer
            SRL::VDP2::NBG0::LoadTilemap(*tileSet);
            m_nbg0Initialized = true;

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

        SRL::VDP2::NBG0::ScrollEnable();
    }


    void drawCleanBackgroundToNBG1()
    {
        SRL::VDP2::NBG1::ScrollDisable();//enable display of NBG1 

        dl_u8* frameBuffer8bpp = new dl_u8[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

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

        if (!m_nbg1Initialized)
        {
            // init the whole layer
            SRL::VDP2::NBG1::LoadTilemap(*tileSet);
            m_nbg1Initialized = true;

            SRL::VDP2::NBG1::SetPriority(SRL::VDP2::Priority::Layer2);//set NBG1 priority
        }
        else
        {
            // we can't easily reset just the layer, so
            // just replace the tiles and map directly
            Cell2VRAM((uint8_t*)tileSet->GetCellData(), 
                      SRL::VDP2::NBG1::CellAddress, 
                      SRL::VDP2::NBG1::Info.CellByteSize);

            Map2VRAM(SRL::VDP2::NBG1::Info,
                     (uint16_t*)tileSet->GetMapData(),
                     SRL::VDP2::NBG1::MapAddress,
                     SRL::VDP2::NBG1::TilePalette.GetId(),
                     SRL::VDP2::NBG1::GetCellOffset(SRL::VDP2::NBG1::Info, SRL::VDP2::NBG1::CellAddress));
        }


        delete tileSet;//free work RAM
        delete [] frameBuffer8bpp;
        delete framebufferBitmap;

        SRL::VDP2::NBG1::ScrollEnable();
    }

    void drawWipeTransition()
    {
	    if (m_gameData.transitionInitialDelay)
	    {
		    return;
	    }

        if (m_gameData.transitionCurrentLine == 0)
        {
            if (m_isNBG0Active)
                drawCleanBackgroundToNBG1();
            else
                drawCleanBackgroundToNBG0();

            // direction to scroll the transitioning background
            const PlayerData* playerData = m_gameData.currentPlayerData;
            m_transitionDirection = playerData->lastDoor->xLocationInNextRoom < 50;

            //m_transitionDirection ? Log::Logger::Print("to Left") : Log::Logger::Print("To right");
        }

        //Log::Logger::Print("draw wipe transition");

        SRL::Math::Types::Vector2D scrollOffset;

        if (m_transitionDirection)
        {
            scrollOffset = SRL::Math::Types::Vector2D(-SCREEN_OFFSET_X, -SCREEN_OFFSET_Y);
            SRL::Math::Types::Fxp pos = (dl_s16)(32 - m_gameData.transitionCurrentLine);
            scrollOffset.X -= pos;
        }
        else
        {
            scrollOffset = SRL::Math::Types::Vector2D(0, -SCREEN_OFFSET_Y);
            SRL::Math::Types::Fxp pos = (dl_s16)(m_gameData.transitionCurrentLine);
            scrollOffset.X -= pos;
        }

        //scrollOffset.Y -= pos;

        if (m_isNBG0Active)
        {
            float opacityNBG0f = (32 - m_gameData.transitionCurrentLine) / 32.0f;
            float opacityNBG1f = m_gameData.transitionCurrentLine / 32.0f;

            SRL::Math::Types::Fxp opacityNBG0 = SRL::Math::Types::Fxp::Convert(opacityNBG0f);
            SRL::Math::Types::Fxp opacityNBG1 = SRL::Math::Types::Fxp::Convert(opacityNBG1f);
            SRL::VDP2::NBG0::SetOpacity(opacityNBG0);
            SRL::VDP2::NBG1::SetOpacity(opacityNBG1);
            SRL::VDP2::NBG1::SetPosition(scrollOffset);
        }
        else
        {
            float opacityNBG1f = (32 - m_gameData.transitionCurrentLine) / 32.0f;
            float opacityNBG0f = m_gameData.transitionCurrentLine / 32.0f;

            SRL::Math::Types::Fxp opacityNBG1 = SRL::Math::Types::Fxp::Convert(opacityNBG1f);
            SRL::Math::Types::Fxp opacityNBG0 = SRL::Math::Types::Fxp::Convert(opacityNBG0f);
            SRL::VDP2::NBG1::SetOpacity(opacityNBG1);
            SRL::VDP2::NBG0::SetOpacity(opacityNBG0);
            SRL::VDP2::NBG0::SetPosition(scrollOffset);
        }



        /*
        if (m_gameData.transitionCurrentLine == 31)
        {
            Log::Logger::Print("Transition done");
            SRL::Math::Types::Fxp opacityNBG0 = SRL::Math::Types::Fxp::Convert(1.0f);
            SRL::Math::Types::Fxp opacityNBG1 = SRL::Math::Types::Fxp::Convert(0.0f);
            SRL::VDP2::NBG1::SetOpacity(opacityNBG1);
            drawCleanBackgroundToNBG0();
            drawCleanBackgroundToNBG1();
            SRL::VDP2::NBG0::SetOpacity(opacityNBG0);

        }
        */
    }

    void drawGetReadyScreen()
    {
        SRL::Math::Types::Vector2D scrollOffset = SRL::Math::Types::Vector2D(-SCREEN_OFFSET_X, -SCREEN_OFFSET_Y);
        SRL::VDP2::NBG0::SetPosition(scrollOffset);
        SRL::VDP2::NBG1::SetPosition(scrollOffset);

        drawDrops(&m_gameData);
    }


public:
    GameData m_gameData;

private:
    Resources* m_resources;
    dl_u8 m_cursor1bppSprite;
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
	FontSprite m_characterFont;
    Sprite m_doorSprite;
    SplatSprite m_splatSprite;
    ClippedSprite m_playerIconSprite;
    RegenSprite m_regenPlayerIconSprite;

    Sprite* m_pickUpSprites[3];

    int m_regenSpriteIndex;

	typedef void (GameRunner::*DrawRoomFunction)();
	std::vector<DrawRoomFunction> m_drawRoomFunctions;

    dl_u8 m_regenSpriteBuffer[(PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS];

    bool m_nbg0Initialized;
    bool m_nbg1Initialized;
    dl_u8 m_currentRoom;
    bool m_isNBG0Active;
    bool m_transitionDirection;
};

#endif