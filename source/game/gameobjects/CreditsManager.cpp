#include "CreditsManager.h"

void CreditsManager::start(SDL_Window* window)
{
    if (!m_renderer)
    {
        m_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (m_renderer == NULL)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        }

        TTF_Init();
        m_font = TTF_OpenFont("../scenes/TitilliumWeb-Regular.ttf", 25);
        if (m_font == NULL)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "font cannot be created! SDL_Error: %s\n", SDL_GetError());
        }

        //Initialize renderer color
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

        //Clear screen
        SDL_RenderClear(m_renderer);
    }
}


void CreditsManager::updateCredits()
{
    for (auto& credit : m_credits)
    {
        credit.m_position.y -= 1;
    }
}

bool CreditsManager::creditsFinished()
{
    if ((m_credits.size() > 0) &&  (m_credits[m_credits.size() - 1].m_position.y < 0))
    {
        return true;
    }
    return false;
}

void CreditsManager::renderCredits()
{
    for (auto& credit : m_credits)
    {
        //int textLengthOffset = credit.getText().size() / 2;
        drawText(credit.m_position.x, credit.m_position.y, credit.getText().c_str());
    }
    SDL_RenderPresent(m_renderer);
    SDL_RenderClear(m_renderer);
}

void CreditsManager::drawText(int x, int y, const char * msg)
{
    SDL_Color white = { 255, 255, 255 };  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color

    SDL_Surface* surf = TTF_RenderText_Solid(m_font, msg, white); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

    SDL_Texture* msg_texture = SDL_CreateTextureFromSurface(m_renderer, surf); //now you can convert it into a texture

    int w = 0;
    int h = 0;
    SDL_QueryTexture(msg_texture, NULL, NULL, &w, &h);
    SDL_Rect dst_rect = { x, y, w, h };

    SDL_RenderCopy(m_renderer, msg_texture, NULL, &dst_rect);

    SDL_DestroyTexture(msg_texture);
    SDL_FreeSurface(surf);
}

void CreditsManager::addCredit(const Credit& credit)
{
    m_credits.push_back(credit);
}