#include "sdl_video_filter_modern.h"
#include "..\sdl_video_filter_utils.h"

#include "..\..\..\game\drops_manager.h"
#include "..\..\..\game\drops_types.h"

Sprite::Sprite(const u8* orginalSprite, 
               u8 width, 
               u8 height, 
               u8 numFrames)
{
    const u8* spriteRunner = orginalSprite;

    m_width = width;
    m_height = height;

    for (int loop = 0; loop < numFrames; loop++)
    {
        std::vector<u32> spriteFrame;
        spriteFrame.resize(width * height);

        SDLUtils_convert1bppImageTo32bppCrtEffectImage(spriteRunner,
                                                       spriteFrame.data(),
                                                       width,
                                                       height,
                                                       CrtColor::Blue);

        m_frames.push_back(spriteFrame);

        spriteRunner += (width / 8) * height;
    }
}

SDLVideoFilterModern::SDLVideoFilterModern(SDL_Renderer* renderer, 
						                   const Resources* resources) 
	: SDLVideoFilterBase(renderer, resources),
      m_dropSprite(resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT),
      m_ballSprite(resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT),
      m_playerSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT),
      m_playerSplatSprite(resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS, PLAYER_SPLAT_SPRITE_COUNT),
      m_birdSprite(resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT),
	  m_keySprite(resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
	  m_diamondSprite(resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
	  m_moneyBagSprite(resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1)
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

void drawSprite(u32* crtFramebuffer, 
                u16 framebufferWidth,
                u16 framebufferHeight,
                u16 spriteX, 
                u16 spriteY, 
                u16 frameNumber,
                const Sprite* sprite)
{
    u32 drawLocation = spriteX + (spriteY * framebufferWidth);
    crtFramebuffer += drawLocation;

    const u32* spriteRunner = sprite->m_frames[frameNumber].data();

    for (int loopY = 0; loopY < sprite->m_height; loopY++)
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

/*
u8 corruptByte(u8 value)
{
	return ((value << 1) | value) & (rand() % 0xff);
}
*/

void drawRegenSprite(u32* crtFramebuffer, 
                     u16 framebufferWidth,
                     u16 framebufferHeight,
                     u16 spriteX, 
                     u16 spriteY, 
                     u16 frameNumber,
                     const Sprite* sprite)
{
    u32 drawLocation = spriteX + (spriteY * framebufferWidth);
    crtFramebuffer += drawLocation;

    const u32* spriteRunner = sprite->m_frames[frameNumber].data();

    for (int loopY = 0; loopY < sprite->m_height; loopY++)
    {
        for (int loopX = 0; loopX < sprite->m_width; loopX++)
        {
            u32 color = *spriteRunner;

            if (color != 0)
            {
                switch (rand() % 4)
                {
                case 0: *crtFramebuffer = 0x00000000; break;
                case 1: *crtFramebuffer = 0x0000FF; break;
                case 2: *crtFramebuffer = 0xFFA500; break;
                case 3: *crtFramebuffer = 0xFFFFFF; break;
                }
            }

            crtFramebuffer++;
            spriteRunner++;
        }

        crtFramebuffer += (framebufferWidth - sprite->m_width);
    }
}

void SDLVideoFilterModern::update(const GameData* gameData)
{
    if (m_outputTexture == nullptr)
        init();

    // Update texture from gameFramebuffer
    SDLUtils_convert1bppImageTo32bppCrtEffectImage(gameData->cleanBackground,
                                                   m_crtFramebuffer,
                                                   FRAMEBUFFER_WIDTH,
                                                   FRAMEBUFFER_HEIGHT,
                                                   CrtColor::Blue);

    // draw drops
    const Drop* dropsRunner = gameData->dropData.drops;

    for (int loop = 0; loop < NUM_DROPS; loop++)
    {
        if (dropsRunner->wiggleTimer)
        {
            drawSprite(m_crtFramebuffer, 
                       FRAMEBUFFER_WIDTH, 
                       FRAMEBUFFER_HEIGHT,
                       dropsRunner->x << 1,
                       dropsRunner->y >> 8,
                       0,
                       &m_dropSprite);
        }

        dropsRunner++;
    }

    u16 currentTimer = 0xffff;

    // draw player and pickups
    if (gameData->currentRoom->roomNumber < TITLESCREEN_ROOM_INDEX)
    {
        const PlayerData* playerData = gameData->currentPlayerData;

        currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

        const Pickup* pickups = playerData->gamePickups[gameData->currentRoom->roomNumber];
        for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
        {
		    if ((pickups->state & playerData->playerMask))
		    {
                drawSprite(m_crtFramebuffer, 
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
            drawSprite(m_crtFramebuffer, 
                       FRAMEBUFFER_WIDTH, 
                       FRAMEBUFFER_HEIGHT,
                       (playerData->x >> 8) << 1,
                       (playerData->y >> 8) + 7,
                       playerData->splatFrameNumber,
                       &m_playerSplatSprite);
            break;
        case PLAYER_STATE_REGENERATION: 
            drawRegenSprite(m_crtFramebuffer, 
                            FRAMEBUFFER_WIDTH, 
                            FRAMEBUFFER_HEIGHT,
                            (playerData->x >> 8) << 1,
                            playerData->y >> 8,
                            playerData->currentSpriteNumber,
                            &m_playerSprite);
            break;
        default: 
            drawSprite(m_crtFramebuffer, 
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

            drawSprite(m_crtFramebuffer, 
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

            drawSprite(m_crtFramebuffer, 
                       FRAMEBUFFER_WIDTH, 
                       FRAMEBUFFER_HEIGHT,
                       (birdData->x >> 8) << 1,
                       birdData->y >> 8,
                       birdData->animationFrame,
                       &m_birdSprite);
        }

        // draw player lives icons
        // draw regen player life icon
        // draw timer
        // draw player text
        // draw score
    }
    else if (gameData->currentRoom->roomNumber == TITLESCREEN_ROOM_INDEX)
    {
	    // draw the cursor
	    u16 drawLocation = gameData->numPlayers == 1 ? 0xf64 : 0xf70;  // hardcoded locations in the frambuffer

        u32* crtFramebuffer = m_crtFramebuffer + (drawLocation * 8);
        for (int loop = 0; loop < 8; loop++)
        {
            *crtFramebuffer = 0xffffffff;
            crtFramebuffer++;
        }
    }

    // Update the texture with the new data
    SDL_UpdateTexture(m_outputTexture, 
                      NULL, 
                      m_crtFramebuffer, 
                      FRAMEBUFFER_WIDTH * sizeof(uint32_t));
}