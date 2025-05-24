#include "sdl_video_filter_modern.h"
#include "..\sdl_video_filter_utils.h"

#include "..\..\..\game\drops_manager.h"
#include "..\..\..\game\drops_types.h"
#include "..\..\..\game\rooms\chambers.h"

extern "C"
{
#include "..\..\..\game\draw_utils.h"
}

#include <algorithm>

Sprite::Sprite(const u8* originalSprite, 
               u8 width, 
               u8 height, 
               u8 numFrames)
    : m_originalSprite(originalSprite),
      m_numFrames(numFrames),
      m_width(width),
      m_height(height)
{
    const u8* spriteRunner = originalSprite;

    std::vector<u32> spriteFrame;
    spriteFrame.resize(width * height);

    for (int loop = 0; loop < numFrames; loop++)
    {
        SDLUtils_convert1bppImageTo32bppCrtEffectImage(spriteRunner,
                                                       spriteFrame.data(),
                                                       width,
                                                       height,
                                                       CrtColor::Blue);

        m_frames.push_back(spriteFrame);

        spriteRunner += (width / 8) * height;
    }
}

void Sprite::updateSprite(u8 frameNumber, const u8* originalSprite)
{
    std::vector<u32>& spriteFrame = m_frames[frameNumber];

    SDLUtils_convert1bppImageTo32bppCrtEffectImage(originalSprite,
                                                   spriteFrame.data(),
                                                   m_width,
                                                   m_height,
                                                   CrtColor::Blue);
}


void drawSpriteBottomClipped(u32* crtFramebuffer, 
                             u16 framebufferWidth,
                             u16 framebufferHeight,
                             u16 spriteX, 
                             u16 spriteY, 
                             u8 spriteHeight,
                             u16 frameNumber,
                             const Sprite* sprite)
{
    u32 drawLocation = spriteX + (spriteY * framebufferWidth);
    crtFramebuffer += drawLocation;

    const u32* spriteRunner = sprite->m_frames[frameNumber].data();

    for (int loopY = 0; loopY < spriteHeight; loopY++)
    {
        for (int loopX = 0; loopX < sprite->m_width; loopX++)
        {
            u32 color = *spriteRunner;

            if (color != 0)
                *crtFramebuffer = color;

            crtFramebuffer++;
            spriteRunner++;
        }

        crtFramebuffer += (framebufferWidth - sprite->m_width);
    }
}

void drawSprite(u32* crtFramebuffer, 
                u16 framebufferWidth,
                u16 framebufferHeight,
                u16 spriteX, 
                u16 spriteY, 
                u16 frameNumber,
                const Sprite* sprite)
{
    drawSpriteBottomClipped(crtFramebuffer,
                            framebufferWidth,
                            framebufferHeight,
                            spriteX,
                            spriteY,
                            sprite->m_height,
                            frameNumber,
                            sprite);
}

void drawText(u32* crtFramebuffer,
              u16 framebufferWidth,
              u16 framebufferHeight,
              const u8* text, 
              const Sprite* characterFont, 
              u32 drawLocation)
{
    u16 x = (drawLocation % 32) << 3;
    u16 y = drawLocation / 32;

    // for each character
    while (*text != 0xff)
    {
        // find the corresponding character in the font
        drawSprite(crtFramebuffer, 
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   *text,
                   characterFont);

        x += 8;
        text++;
    }
}

void drawPlayerLives(u8 playerLives,
					 u8 currentSpriteNumber,
					 const Sprite* playerSprite,
					 const Sprite* regenSprite,
					 u8 isRegenerating,
                     u32* framebuffer)
{
	u8 x = PLAYERLIVES_ICON_X;
	u8 y = PLAYERLIVES_ICON_Y;

    for (u8 loop = 0; loop < playerLives; loop++)
	{
        drawSpriteBottomClipped(framebuffer, 
                                FRAMEBUFFER_WIDTH, 
                                FRAMEBUFFER_HEIGHT,
                                x << 1,
                                y,
                                PLAYERICON_NUM_SPRITE_ROWS,
                                currentSpriteNumber,
                                playerSprite);

		x += PLAYERLIVES_ICON_SPACING;
    }

	if (isRegenerating)
	{
        drawSpriteBottomClipped(framebuffer, 
                                FRAMEBUFFER_WIDTH, 
                                FRAMEBUFFER_HEIGHT,
                                x << 1,
                                y,
                                PLAYERICON_NUM_SPRITE_ROWS,
                                0,
                                regenSprite);
    }
}

SDLVideoFilterModern::SDLVideoFilterModern(SDL_Renderer* renderer, 
						                   const Resources* resources) 
	: SDLVideoFilterBase(renderer, resources),
      m_resources(resources),
      m_dropSprite(resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT),
      m_ballSprite(resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT),
      m_playerSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT),
      m_playerSplatSprite(resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS, PLAYER_SPLAT_SPRITE_COUNT),
      m_birdSprite(resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT),
	  m_keySprite(resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
	  m_diamondSprite(resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
	  m_moneyBagSprite(resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
      m_regenSprite(m_regenSpriteBuffer, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 1),
      m_characterFont(resources->characterFont, 8, 7, 39)
{
    // in the original game, the splat is animated by erasing the
    // top part of the splat on the framebuffer itself. Here, we create
    // a copy the first frame of animation and erase the top to acheive the 
    // same effect.
    m_playerSplatSprite.m_frames.push_back(m_playerSplatSprite.m_frames[0]);
    u32* frame = m_playerSplatSprite.m_frames[1].data();
    for (int loop = 0; loop < 5 * PLAYER_SPLAT_SPRITE_WIDTH; loop++)
        frame[loop] = 0;

    m_pickUpSprites.push_back(&m_diamondSprite);
    m_pickUpSprites.push_back(&m_moneyBagSprite);
    m_pickUpSprites.push_back(&m_keySprite);

    // convert font to blue
    for (int loop = 0; loop < m_characterFont.m_numFrames; loop++)
    {
        std::vector<u32>& frame = m_characterFont.m_frames[loop];

        for (int counter = 0; counter < frame.size(); counter++)
        {
            if (frame[counter] != 0)
                frame[counter] = 0x0000FF;
        }
    }

    m_drawRoomFunctions = { &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawChamber,
                            &SDLVideoFilterModern::drawTitleScreen,
                            &SDLVideoFilterModern::drawTransition,
                            &SDLVideoFilterModern::drawWipeTransition,
                            &SDLVideoFilterModern::drawGetReadyScreen };
}

bool SDLVideoFilterModern::init()
{
    // Create the texture
    m_outputTexture = SDL_CreateTexture(m_renderer, 
                                        SDL_PIXELFORMAT_XRGB8888, 
                                        SDL_TEXTUREACCESS_STREAMING, 
                                        FRAMEBUFFER_WIDTH, 
                                        FRAMEBUFFER_HEIGHT);

    SDL_SetTextureScaleMode(m_outputTexture, SDL_SCALEMODE_NEAREST); // no smoothing

	return true;
}

void SDLVideoFilterModern::shutdown()
{
    SDLVideoFilterBase::shutdown();
}


void SDLVideoFilterModern::updateRegenSprite(u8 currentPlayerSpriteNumber)
{
    const u8* originalSprite = m_playerSprite.m_originalSprite;
    originalSprite += currentPlayerSpriteNumber * (m_playerSprite.m_width / 8) * m_playerSprite.m_height;

    memset(m_regenSpriteBuffer, 0, sizeof(m_regenSpriteBuffer));

    drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													m_playerSprite.m_height,
													(u8*)m_regenSpriteBuffer);

    m_regenSprite.updateSprite(0, m_regenSpriteBuffer);
}

void SDLVideoFilterModern::roomChanged(u8 roomNumber, s8 transitionType)
{

}

void SDLVideoFilterModern::drawDrops(const GameData* gameData, u32* framebuffer)
{
    // draw drops
    const Drop* dropsRunner = gameData->dropData.drops;

    for (int loop = 0; loop < NUM_DROPS; loop++)
    {
        if (dropsRunner->wiggleTimer)
        {
            drawSprite(framebuffer, 
                       FRAMEBUFFER_WIDTH, 
                       FRAMEBUFFER_HEIGHT,
                       dropsRunner->x << 1,
                       dropsRunner->y >> 8,
                       0,
                       &m_dropSprite);
        }

        dropsRunner++;
    }
}

void SDLVideoFilterModern::drawChamber(const GameData* gameData, u32* framebuffer)
{
    // Update texture from gameFramebuffer
    SDLUtils_convert1bppImageTo32bppCrtEffectImage(gameData->cleanBackground,
                                                   framebuffer,
                                                   FRAMEBUFFER_WIDTH,
                                                   FRAMEBUFFER_HEIGHT,
                                                   CrtColor::Blue);

    // draw drops
    drawDrops(gameData, framebuffer);



    const PlayerData* playerData = gameData->currentPlayerData;

    u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

    const Pickup* pickups = playerData->gamePickups[gameData->currentRoom->roomNumber];
    for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
    {
		if ((pickups->state & playerData->playerMask))
		{
            drawSprite(framebuffer, 
                        FRAMEBUFFER_WIDTH, 
                        FRAMEBUFFER_HEIGHT,
                        pickups->x << 1,
                        pickups->y,
                        0,
                        m_pickUpSprites[pickups->type]);
        }

        pickups++;
    }


    switch (playerData->state)
    {
    case PLAYER_STATE_SPLAT: 
        drawSprite(framebuffer, 
                    FRAMEBUFFER_WIDTH, 
                    FRAMEBUFFER_HEIGHT,
                    (playerData->x >> 8) << 1,
                    (playerData->y >> 8) + 7,
                    playerData->splatFrameNumber,
                    &m_playerSplatSprite);
        break;
    case PLAYER_STATE_REGENERATION: 

        if (!gameData->paused)
        {
            updateRegenSprite(playerData->currentSpriteNumber);
        }

        drawSprite(framebuffer, 
                    FRAMEBUFFER_WIDTH, 
                    FRAMEBUFFER_HEIGHT,
                    (playerData->x >> 8) << 1,
                    playerData->y >> 8,
                    0,
                    &m_regenSprite);
        break;
    default: 
        drawSprite(framebuffer, 
                    FRAMEBUFFER_WIDTH, 
                    FRAMEBUFFER_HEIGHT,
                    (playerData->x >> 8) << 1,
                    playerData->y >> 8,
                    playerData->currentSpriteNumber,
                    &m_playerSprite);
    }

    // draw ball
    if (gameData->ballData.enabled)
    {
        const BallData* ballData = &gameData->ballData;

        drawSprite(framebuffer, 
                    FRAMEBUFFER_WIDTH, 
                    FRAMEBUFFER_HEIGHT,
                    (ballData->x >> 8) << 1,
                    ballData->y >> 8,
                    ((s8)ballData->fallStateCounter < 0),
                    &m_ballSprite);

    }

    // draw bird
    if (gameData->birdData.state && currentTimer == 0)
    {
        const BirdData* birdData = &gameData->birdData;

        drawSprite(framebuffer, 
                    FRAMEBUFFER_WIDTH, 
                    FRAMEBUFFER_HEIGHT,
                    (birdData->x >> 8) << 1,
                    birdData->y >> 8,
                    birdData->animationFrame,
                    &m_birdSprite);
    }

    // draw player lives icons
    drawPlayerLives(gameData->currentPlayerData->lives,
                    playerData->currentSpriteNumber,
                    &m_playerSprite,
                    &m_regenSprite,
                    playerData->state == PLAYER_STATE_REGENERATION,
                    framebuffer);



    // draw timer
	drawText(framebuffer,
                FRAMEBUFFER_WIDTH,
                FRAMEBUFFER_HEIGHT,
                gameData->string_timer, 
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
                gameData->string_roomNumber, 
			    &m_characterFont, 
			    CHAMBER_NUMBER_TEXT_DRAW_LOCATION);

    // draw score
	drawText(framebuffer,
                FRAMEBUFFER_WIDTH,
                FRAMEBUFFER_HEIGHT,
                playerData->scoreString, 
			    &m_characterFont, 
			    SCORE_DRAW_LOCATION);
}

void SDLVideoFilterModern::drawTitleScreen(const GameData* gameData, u32* framebuffer)
{
    SDLUtils_convert1bppImageTo32bppCrtEffectImage(gameData->cleanBackground,
                                                   framebuffer,
                                                   FRAMEBUFFER_WIDTH,
                                                   FRAMEBUFFER_HEIGHT,
                                                   CrtColor::Blue);

    // draw drops
    drawDrops(gameData, framebuffer);

	// draw the cursor
	u16 drawLocation = gameData->numPlayers == 1 ? 0xf64 : 0xf70;  // hardcoded locations in the frambuffer

    framebuffer = framebuffer + (drawLocation * 8);
    for (int loop = 0; loop < 8; loop++)
    {
        *framebuffer = 0xffffffff;
        framebuffer++;
    }

}

void SDLVideoFilterModern::drawTransition(const GameData* gameData, u32* framebuffer)
{
	memset(framebuffer, 0, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(u32));
}

void SDLVideoFilterModern::drawWipeTransition(const GameData* gameData, u32* framebuffer)
{

}

void SDLVideoFilterModern::drawGetReadyScreen(const GameData* gameData, u32* framebuffer)
{
    SDLUtils_convert1bppImageTo32bppCrtEffectImage(gameData->cleanBackground,
                                                   framebuffer,
                                                   FRAMEBUFFER_WIDTH,
                                                   FRAMEBUFFER_HEIGHT,
                                                   CrtColor::Blue);

    drawDrops(gameData, framebuffer);
}

void SDLVideoFilterModern::update(const GameData* gameData)
{
    if (m_outputTexture == nullptr)
        init();

    (this->*m_drawRoomFunctions[gameData->currentRoom->roomNumber])(gameData, m_crtFramebuffer);

    // to make it easier to tell this filter is on
    drawSprite(m_crtFramebuffer, 
               FRAMEBUFFER_WIDTH, 
               FRAMEBUFFER_HEIGHT,
               240,
               176,
               0,
               &m_ballSprite);

    // Update the texture with the new data
    SDL_UpdateTexture(m_outputTexture, 
                      NULL, 
                      m_crtFramebuffer, 
                      FRAMEBUFFER_WIDTH * sizeof(uint32_t));
}