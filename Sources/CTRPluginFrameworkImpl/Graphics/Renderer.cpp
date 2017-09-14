#include "types.h"
#include "ctrulib/services/gspgpu.h"


#include "CTRPluginFramework/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include <cstdio>
#include <cmath>

namespace CTRPluginFramework
{
    

    Target      Renderer::_target = BOTTOM;
    ScreenImpl* Renderer::_screen = ScreenImpl::Bottom;


    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void        Renderer::SetTarget(Target target)
    {
        _target = target;
        if (_target == BOTTOM)
            _screen = ScreenImpl::Bottom;
        else
            _screen = ScreenImpl::Top;

        PrivColor::SetFormat(_screen->GetFormat());
    }

    void        Renderer::EndFrame(bool copy)
    {
        static IntRect                  background(20, 20, 280, 200);
        static Color                    blank(255, 255, 255);
        //static Color                    black;
        //static Clock                    fpsCounter;

        bool isTouchDown = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        // Draw Touch Cursor (on menu)
        if (isTouchDown && background.Contains(touchPos))
        {
            int posX = touchPos.x - 2;
            int posY = touchPos.y - 1;
            touchPos.x += 10;
            touchPos.y += 15;
            if (background.Contains(touchPos))
                DrawSysString("\uE058", posX, posY, 320, blank);
        }

        // Draw fps counter
       /* char buffer[20] = {0};
        Time delta = fpsCounter.Restart();

        sprintf(buffer, "FPS: %.02f", Seconds(1.f).AsSeconds() / delta.AsSeconds());
        int posY = 30;
        DrawString(buffer, 200, posY, blank, black);*/
        ScreenImpl::Bottom->SwapBuffer(true, copy);
        ScreenImpl::Top->SwapBuffer(true, copy);
        gspWaitForVBlank();
    }

    void    Renderer::MenuSelector(int posX, int posY, int width, int height)
    {
        int x = posX;
        int y = posY;
        int w = width;
        int h = height;


        u8 *left = _screen->GetLeftFramebuffer(posX, posY + 1);
        int bpp;
        int rowstride;
        GSPGPU_FramebufferFormats fmt;
        _screen->GetFramebufferInfos(rowstride, bpp, fmt);

        // Draw Rectangle
        while (--w >= 0)
        {
            h = height;
            //x = posX + w;
            u8 *dst = left + rowstride * w;
            while (--h >= 0)
            {   
                Color c = PrivColor::FromFramebuffer(dst);

                c.Fade(0.2f);
                PrivColor::ToFramebuffer(dst, c);
                dst -= bpp;
            }
        }

        int tier = width / 3;
        int pitch = tier / 10;
        int j = 0;
        float fading = 0.0f;

        Color l(255, 255, 255);
        posY += height;
        u8 *dst = _screen->GetLeftFramebuffer(posX + (width - tier), posY);
        u8 *rtier = dst;
        Color black(60, 60, 60);
        // Right tier
        for (int i = tier; i > 0; --i)
        {
            l.Fade(fading);
            if (l <= black)
                break;
            PrivColor::ToFramebuffer(rtier, l);
            j++;
            if (j == pitch)
            {
                fading -= 0.01f;
                j = 0;
            }
            rtier += rowstride;

        }

        l = Color(255, 255, 255);
        // Middle tier
        for (int i = 0; i < tier; ++i)
        {
            PrivColor::ToFramebuffer(dst, l);
            dst -= rowstride;
        }
        fading = 0.0f;

        // Left tier
        for (int i = tier; i > 0; --i)
        {
            l.Fade(fading);
            if (l <= black)
                break;
            PrivColor::ToFramebuffer(dst, l);
            dst -= rowstride;
            j++;
            if (j == pitch)
            {
                fading -= 0.01f;
                j = 0;
            }
        }
    }
}
