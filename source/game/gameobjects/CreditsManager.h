#pragma once
#include "Credit.h"
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>

class CreditsManager
{
public:

    void start(SDL_Window* window);
    void updateCredits();
    void renderCredits();
    void addCredit(const Credit& credit);
    bool creditsFinished();

private:
    void drawText(int x, int y, const char* msg);

private:
    std::vector<Credit> m_credits;
    TTF_Font* m_font;
    SDL_Renderer* m_renderer;
};