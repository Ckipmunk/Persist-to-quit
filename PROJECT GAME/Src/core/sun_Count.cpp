#include "sun_Count.h"

sun_Count::sun_Count()
{
    //ctor
}

sun_Count::~sun_Count()
{
    /*  TTF_CloseFont( roboto_Font );
      roboto_Font = NULL; */
}

void sun_Count::load_Media(SDL_Renderer* r, SDL_Texture* l)
{
    gRenderer = r;
    gTexture = l;
    s_count.ss_Rend = r;
    s_count.ss_Texture = l;
    suns = 500;
    roboto_Font = TTF_OpenFont("C:/Users/ADMIN/source/repos/PROJECT GAME/Assets/Fonts/Roboto/Roboto-Bold.ttf", 100);         //Initializing roboto font

    if (roboto_Font == NULL)
    {
        printf("Failed to load roboto font! SDL_ttf Error: %s\n", TTF_GetError());
    }
    else
    {
        black_color = { 0, 0, 0 };          //Initializing color
    }

    suncount_1digit.set_Coords(gRenderer, 52, 6, 14, 28);               //Sun count 1 digit slot
    suncount_2digit.set_Coords(gRenderer, 47, 6, 24, 28);               //Sun count 2 digit slot
    suncount_3digit.set_Coords(gRenderer, 42, 6, 35, 28);               //Sun count 3 digit slot
}

void sun_Count::set_Suns(int x)
{
    suns = x;
    string str_sun;
    stringstream ss;
    ss << suns;
    str_sun = ss.str();
    if (suns >= 0 && suns < 10)                                       //Condition for setting viewport for 1-digit sun count
    {
        suncount_1digit.set_Viewport();
        gTexture = s_count.loadMedia(str_sun, black_color, roboto_Font);
        SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
    }

    else if (suns >= 10 && suns < 100)     //Condition for setting viewport for 2-digit sun count
    {
        suncount_2digit.set_Viewport();
        gTexture = s_count.loadMedia(str_sun, black_color, roboto_Font);
        SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
    }
    else       //Condition for setting viewport for 3-digit sun count
    {
        suncount_3digit.set_Viewport();
        gTexture = s_count.loadMedia(str_sun, black_color, roboto_Font);
        SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
    }

}

