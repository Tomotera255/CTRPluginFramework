#ifndef CTRPLUGINFRAMEWORK_ICON_HPP
#define CTRPLUGINFRAMEWORK_ICON_HPP

namespace CTRPluginFramework
{
    class Icon
    {
    public:
        /*
        ** About
        ** 15px * 15px
        ************/
        static int      DrawAbout(int posX, int posY);


        /*
        ** CapsLockOn
        ** 15px * 15px
        ************/
        static int      DrawCapsLockOn(int posX, int posY, bool filled);
        
        /*
        ** CentreOfGravity
        ** 15px * 15px
        ************/
        static int      DrawCentreOfGravity(int posX, int posY);

        /*
        ** CheckBox 
        ** 15px * 15px
        ************/
        static int      DrawCheckBox(IntVector &pos, bool isChecked);
        static int      DrawCheckBox(int posX, int posY, bool isChecked);

        /*
        ** Clear Symbol
        ** 15px * 15px
        *************/
        static int      DrawClearSymbol(int posX, int posY, bool filled);

        /*
        ** Close
        ** 20px * 20px
        ************/
        static int      DrawClose(IntVector &pos, bool filled);
        static int      DrawClose(int posX, int posY, bool filled);

        /*
        ** Controller
        ** 15px * 15px
        *************/
        static int      DrawController(int posX, int posY);

        /*
        ** Enter Key
        ** 15px * 15px
        ************/
        static int      DrawEnterKey(int posX, int posY, bool filled);

        /*
        ** Folder
        ** 15px * 15px
        ************/
        static int      DrawFolder(IntVector &pos);
        static int      DrawFolder(int posX, int posY);

        /*
        ** Favorite
        ** 25px * 25px
        ************/
        static int      DrawAddFavorite(IntVector &pos, bool filled);
        static int      DrawAddFavorite(int posX, int posY, bool filled);
        static int      DrawFavorite(IntVector &pos);
        static int      DrawFavorite(int posX, int posY);

        /*
        ** File
        ** 15px * 15px
        *************/
        static int      DrawFile(int posX, int posY);

        /*
        ** Game Controller
        ** 15px * 15px
        ************/
        static int      DrawGameController(int posX, int posY);

        /*
        ** Game Controller
        ** 25px * 25px
        ************/
        static int      DrawGameController(int posX, int posY, bool filled);

        /*
        ** Grid
        ** 15px * 15px
        ************/
        static int      DrawGrid(int posX, int posY);

        
        /*
        ** Info
        ** 25px * 25px
        ***********/
        static int      DrawInfo(IntVector &pos, bool filled);
        static int      DrawInfo(int posX, int posY, bool filled);

        /*
        ** Guide
        ** 15px * 15px
        ***********/
        static int      DrawGuide(IntVector &pos);
        static int      DrawGuide(int posX, int posY);

        /*
        ** Hand Cursor
        ** 15 px * 15 px
        ****************/
        static int      DrawHandCursor(int posX, int posY);

        /*
        ** Happy face
        ** 15 px * 15 px
        ****************/
        static int      DrawHappyFace(int posX, int posY, bool filled);

        /*
        ** Keyboard
        ** 25px * 25px
        **************/
        static int      DrawKeyboard(int posX, int posY, bool filled);

        /*
        ** More
        ** 15px * 15px
        **************/
        static int      DrawMore(int posX, int posY);

        /*
        ** RAM
        ** 15px * 15 px
        **************/
        static int      DrawRAM(int posX, int posY);
        
        /*
        ** Save
        ** 25px * 25px
        **************/
        static int      DrawSave(int posX, int posY);
        static int      DrawSaveNTR(int posX, int posY);

        /*
        ** Search
        ** 15px * 15 px
        **************/
        static int      DrawSearch(int posX, int posY);

        /*
        ** Settings
        ** 15px * 15 px
        **************/
        static int      DrawSettings(int posX, int posY);

        /*
        ** Tools
        ** 15px * 15 px
        *************/
        static int      DrawTools(int posX, int posY);

    private:
        static int      DrawImg(u8 *img, int posX, int posY, int sizeX, int sizeY);
        static int      DrawImgNTR(u8 *img, int posX, int posY, int sizeX, int sizeY);
    };
}

#endif