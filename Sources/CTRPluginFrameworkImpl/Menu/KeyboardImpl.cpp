#include "CTRPluginFrameworkImpl/Menu/KeyboardImpl.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"

#include <cmath>
#include "ctrulib/util/utf.h"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuImpl.hpp"

namespace CTRPluginFramework
{
    #define USER_VALID  0
    #define USER_ABORT  -1
    #define KEY_ENTER 0xA
    #define KEY_BACKSPACE 0x8
    #define KEY_SYMBOLS -2
    #define KEY_CAPS -3
    #define KEY_SMILEY -4
    #define KEY_SPACE -5
    #define KEY_SYMBOLS_PAGE -6
    #define KEY_NINTENDO_PAGE -7
    #define KEY_PLUS_MINUS -8

    std::vector<TouchKey>    KeyboardImpl::_DecimalKeys;
    std::vector<TouchKey>    KeyboardImpl::_HexaDecimalKeys;
    std::vector<TouchKey>    KeyboardImpl::_QwertyKeys;

    KeyboardImpl::KeyboardImpl(const std::string &text)
    {
        _owner = nullptr;

        _text = text;
        _error = "";
        _userInput = "";
        _canChangeLayout = false;
        _canAbort = true;
        _isOpen = false;
        _userAbort = false;
        _mustRelease = false;
        _useCaps = false;
        _useSymbols = false;
        _useNintendo = false;
        _errorMessage = false;
        _askForExit = false;
        _isHex = false;
        _offset = 0.f;
        _max = 0;
        _layout = HEXADECIMAL;
        _cursorPositionInString = 0;
        _cursorPositionOnScreen = 0;
        _showCursor = true;
        _keys = nullptr;
        _convert = nullptr;
        _compare = nullptr;
        _onInputChange = nullptr;

        _customKeyboard = false;
        _displayScrollbar = false;
        _currentPosition = 0;
        _scrollbarSize = 0;
        _scrollCursorSize = 10;
        _scrollPadding = 0.f;
        _scrollJump = 0.f;
        _scrollSize = 0.f;
        _scrollPosition = 0.f;
        _inertialVelocity = 0.f;
        _scrollStart = 0.f;
        _scrollEnd = 0.f;

        _symbolsPage = 0;
        _nintendoPage = 0;

        DisplayTopScreen = true;
    }

    KeyboardImpl::KeyboardImpl(Keyboard* kb, const std::string &text)
    {
        _owner = kb;

        _text = text;
        _error = "";
        _userInput = "";
        _canChangeLayout = false;
        _canAbort = true;
        _isOpen = false;
        _userAbort = false;
        _mustRelease = false;
        _useCaps = false;
        _useSymbols = false;
        _useNintendo = false;
        _errorMessage = false;
        _askForExit = false;
        _isHex = false;
        _offset = 0.f;
        _max = 0;
        _layout = HEXADECIMAL;
        _cursorPositionInString = 0;
        _cursorPositionOnScreen = 0;
        _showCursor = true;

        _convert = nullptr;
        _compare = nullptr;
        _onInputChange = nullptr;
        _keys = nullptr;

        _customKeyboard = false;
        _displayScrollbar = false;
        _currentPosition = 0;
        _scrollbarSize = 0;
        _scrollCursorSize = 10;
        _scrollPadding = 0.f;
        _scrollJump = 0.f;
        _scrollSize = 0.f;
        _scrollPosition = 0.f;
        _inertialVelocity = 0.f;
        _scrollStart = 0.f;
        _scrollEnd = 0.f;

        _symbolsPage = 0;
        _nintendoPage = 0;

        DisplayTopScreen = true;
    }

    KeyboardImpl::~KeyboardImpl(void)
    {
      //  for (KeyIter iter = _keys.begin(); iter != _keys.end(); ++iter)
      //      iter->Clear();
        for (TouchKeyString *tks : _strKeys)
            delete tks;
    }

    void    KeyboardImpl::SetLayout(Layout layout)
    {
        _layout = layout;
        _isHex = false;
        _userInput.clear();
        if (_layout == QWERTY) _Qwerty();
        else if (_layout == DECIMAL) _Decimal();
        else if (_layout == HEXADECIMAL)
        {
            _isHex = true;
            _Hexadecimal();
        }
    }

    void    KeyboardImpl::SetHexadecimal(bool isHex)
    {
        _isHex = isHex;
    }

    bool    KeyboardImpl::IsHexadecimal(void) const
    {
        return (_isHex);
    }

    void    KeyboardImpl::SetMaxInput(int max)
    {
        _max = max;
    }

    void    KeyboardImpl::CanAbort(bool canAbort)
    {
        _canAbort = canAbort;
    }

    std::string &KeyboardImpl::GetInput(void)
    {
        return (_userInput);
    }

    std::string &KeyboardImpl::GetMessage(void)
    {
        return (_text);
    }

    void    KeyboardImpl::SetError(std::string &error)
    {
        _errorMessage = true;
        _error = error;
    }

    void    KeyboardImpl::SetConvertCallback(ConvertCallback callback)
    {
        _convert = callback;
    }

    void    KeyboardImpl::SetCompareCallback(CompareCallback callback)
    {
        _compare = callback;
    }

    void    KeyboardImpl::OnInputChange(OnInputChangeCallback callback)
    {
        _onInputChange = callback;
    }

    void    KeyboardImpl::Populate(const std::vector<std::string> &input)
    {
        _customKeyboard = true;
        _currentPosition = 0;
        for (TouchKeyString *tks : _strKeys)
            delete tks;
        _strKeys.clear();

        int posY = 30;
        int count = input.size();

        if (count < 6)
        {
            posY = 20 + (200 - ((30 * count) + 6 * (count - 1))) / 2;
            _displayScrollbar = false;
        }
        else
        {
            int height = 190;


            float lsize = 36.f * (float)count + 1;

            float padding = (float)height / lsize;
            int cursorSize =  padding * height;
            float scrollTrackSpace = lsize - height;
            float scrollThumbSpace = height - cursorSize;

            _scrollJump = scrollTrackSpace / scrollThumbSpace;
            _scrollbarSize = height;

            if (cursorSize < 5)
                cursorSize = 5;

            _scrollPadding = padding;
            _scrollCursorSize = cursorSize;
            _scrollPosition = 0.f;
            _scrollEnd = _scrollbarSize - _scrollCursorSize;
            _displayScrollbar = true;
        }

        IntRect box(60, posY, 200, 30);

        for (const std::string &str : input)
        {
            _strKeys.push_back(new TouchKeyString(str, box, true));
            box.leftTop.y += 36;
        }
    }

    int     KeyboardImpl::Run(void)
    {
        _isOpen = true;
        _userAbort = false;
        _askForExit = false;

        // Check if Process is paused
        if (!ProcessImpl::IsPaused())
        {
            _mustRelease  = true;
            ProcessImpl::Pause(false);
        }
        else
            _mustRelease = false;

        int                 ret = -2;
        Event               event;
        EventManager        manager;
        Clock               clock;

        // Construct keyboard
        if (!_customKeyboard)
        {
            if (_layout == QWERTY) _Qwerty();
            else if (_layout == DECIMAL) _Decimal();
            else if (_layout == HEXADECIMAL) _Hexadecimal();
        }

		// Check start input
		_errorMessage = !_CheckInput();

        // Set cursor
        if (_showCursor)
        {
            _cursorPositionInString = _userInput.size();
            _ScrollUp();
        }

        // Loop until exit
        while (_isOpen)
        {
            while (manager.PollEvent(event))
            {
                _ProcessEvent(event);
                if (_userAbort)
                {
                    ret = USER_ABORT;
                    goto exit;
                }
            }

            // Update current keys
            _Update(clock.Restart().AsSeconds());

            // Render Top Screen
            if (DisplayTopScreen)
                _RenderTop();
            // Render Bottom Screen
            _RenderBottom();
            Renderer::EndFrame();

            // if it's a standard keyboard
            if (!_customKeyboard)
            {
                // Check keys
                bool inputChanged = _CheckKeys();

                if (_errorMessage && inputChanged)
                    _errorMessage = false;

                if (inputChanged)
                {
                    _errorMessage = !_CheckInput();
                    if (_onInputChange != nullptr && _owner != nullptr)
                        _onInputChange(*_owner, _inputChangeEvent);
                }

                // If user try to exit the keyboard
                if (_askForExit)
                {
                    // If input is invalid, user can't exit
                    if (_errorMessage)
                        _askForExit = false;
                    else
                    {
                        // Check input
                        _errorMessage = !_CheckInput();
                        if (!_errorMessage)
                        {
                            // input is valid, exit
                            _isOpen = false;
                            ret = 0;
                        }
                    }
                }
            }
            else
            {
                int  choice = -1;
                bool isSelected = _CheckButtons(choice);

                if (isSelected)
                {
                    ret = choice;
                    _isOpen = false;
                }
            }
        }

    exit:
        PluginMenu *menu = PluginMenu::GetRunningInstance();
        if (menu && !menu->IsOpen())
            ScreenImpl::Clean();
        if (_mustRelease)
            ProcessImpl::Play(false);
        return (ret);
    }

    void    KeyboardImpl::Close(void)
    {
        _isOpen = false;
    }

    void    KeyboardImpl::_RenderTop(void)
    {
        const Color     &red = Color::Red;
        static IntRect  background1(30, 20, 340, 200);
        static IntRect  background2(50, 30, 300, 180);

        IntRect &background = _mustRelease ? background2 : background1;
        int     maxX = background.leftTop.x + background.size.x;
        int     maxY = background.leftTop.y + background.size.y;

        int   posY =  background.leftTop.y + 5;
        int   posX =  background.leftTop.x + 5;

        Renderer::SetTarget(TOP);
        Window::TopWindow.Draw();

        Renderer::DrawSysStringReturn(reinterpret_cast<const u8 *>(_text.c_str()), posX, posY, maxX, Preferences::Settings.MainTextColor, maxY);

        // IF error
        if (_errorMessage && _error.size() > 0)
        {
            if (posY < 120)
                posY += 48;
            Renderer::DrawSysStringReturn(reinterpret_cast<const u8 *>(_error.c_str()), posX, posY, maxX, red, maxY);
        }
    }

    void    KeyboardImpl::_RenderBottom(void)
    {
        const Color     &black = Color::Black;
        const Color     &blank = Color::Blank;
        const Color     &grey = Color::BlackGrey;
        static IntRect  background(20, 20, 280, 200);
        static IntRect  background2(22, 22, 276, 196);
        static IntRect  background3(22, 25, 270, 190);

        Renderer::SetTarget(BOTTOM);

        /*// Background
        if (Preferences::bottomBackgroundImage->IsLoaded())
            Preferences::bottomBackgroundImage->Draw(background.leftTop);
        else
        {*/

           // Renderer::DrawRect(22, 22, 276, 196, blank, false);
        //}

        // Draw current input

        if (!_customKeyboard)
        {
            int   posY = 20;
            int   posX = 25;

            Renderer::DrawRect(background, black);
            Renderer::DrawSysString(_userInput.c_str(), posX, posY, 300, blank, _offset);

            // Draw cursor
            if (_showCursor && _cursorPositionOnScreen >= 0)
                Renderer::DrawLine(_cursorPositionOnScreen + posX, 21, 1, Color::Blank, 16);

            // Digit layout
            if (_layout != Layout::QWERTY)
            {
                // Draw keys
                for (TouchKey &key : *_keys)
                {
                    key.Draw();
                }
            }
            // Qwerty layout
            else
            {
                // Symbols
                if (_useSymbols)
                {
                    int start;
                    int end;

                    if (!_symbolsPage)
                    {
                        start = 72;
                        end = 109;
                    }
                    else
                    {
                        start = 109;
                        end = 147;
                    }

                    for (; start < end; start++)
                    {
                        (*_keys)[start].Draw();
                    }
                }
                // Nintendo
                else if (_useNintendo)
                {
                    int start;
                    int end;

                    if (!_nintendoPage)
                    {
                        start = 147;
                        end = 182;
                    }
                    else
                    {
                        start = 182;
                        end = 217;
                    }

                    for (; start < end; start++)
                    {
                        (*_keys)[start].Draw();
                    }
                }
                // Letters
                else
                {
                    // Upper Case
                    if (_useCaps)
                    {
                        /*36 - 71*/
                        for (int i = 36; i < 72; i++)
                        {
                            (*_keys)[i].Draw();
                        }
                    }
                    // Lower Case
                    else
                    {
                        /* 0 - 35*/
                        for (int i = 0; i < 36; i++)
                        {
                            (*_keys)[i].Draw();
                        }
                    }
                }
            }
        }
        else
        {
            Renderer::DrawRect2(background, black, grey);
            Renderer::DrawRect(background2, blank, false);

            int max = _strKeys.size();
            max = std::min(max, _currentPosition + 6);

            PrivColor::UseClamp(true, background3);

            for (int i = _currentPosition; i < max && i < _strKeys.size(); i++)
            {
                _strKeys[i]->Draw();
            }

            PrivColor::UseClamp(false);
            if (!_displayScrollbar)
                return;

            // Draw scroll bar
            const Color &dimGrey = Color::DimGrey;
            const Color &silver = Color::Silver;

            // Background
            int posX = 292;
            int posY = 25;

            Renderer::DrawLine(posX, posY + 1, 1, silver, _scrollbarSize - 2);
            Renderer::DrawLine(posX + 1, posY, 1, silver, _scrollbarSize);
            Renderer::DrawLine(posX + 2, posY + 1, 1, silver, _scrollbarSize - 2);

            posY += (int)(_scrollPosition);// * _scrollbarSize);

            Renderer::DrawLine(posX, posY + 1, 1, dimGrey, _scrollCursorSize - 2);
            Renderer::DrawLine(posX + 1, posY, 1, dimGrey, _scrollCursorSize);
            Renderer::DrawLine(posX + 2, posY + 1, 1, dimGrey, _scrollCursorSize - 2);
        }
    }

    void    KeyboardImpl::_ProcessEvent(Event &event)
    {
        if (event.type == Event::KeyPressed)
        {
            if (event.key.code == Key::B)
            {
                if (_canAbort)
                    _userAbort = true;
                return;
            }
            if (!_customKeyboard && event.key.code == Y && !_userInput.empty())
            {
                _userInput.clear();
                _inputChangeEvent.type = InputChangeEvent::EventType::InputWasCleared;
                _inputChangeEvent.codepoint = 0;
                _errorMessage = !_CheckInput();
                if (_onInputChange != nullptr && _owner != nullptr)
                    _onInputChange(*_owner, _inputChangeEvent);
            }
            if (event.key.code == X && !_customKeyboard && _layout != QWERTY && _canChangeLayout)
            {
                _userInput.clear();
                _inputChangeEvent.type = InputChangeEvent::EventType::InputWasCleared;
                _inputChangeEvent.codepoint = 0;
                _errorMessage = !_CheckInput();
                if (_onInputChange != nullptr && _owner != nullptr)
                    _onInputChange(*_owner, _inputChangeEvent);
                SetLayout(_layout == DECIMAL ? HEXADECIMAL : DECIMAL);
            }
        }

        if (event.type == Event::KeyDown)
        {
            static Clock inputClock;

            if (_showCursor && inputClock.HasTimePassed(Milliseconds(200)))
            {
                if (event.key.code == Key::DPadLeft)
                    _ScrollDown();
                else if (event.key.code == Key::DPadRight)
                    _ScrollUp();
                inputClock.Restart();
            }
        }

        if (!_displayScrollbar)
            return;

        static IntRect  buttons(60, 26, 200, 200);
        // Touch / Scroll
        if (event.type == Event::TouchBegan)
        {
            if (!buttons.Contains(event.touch.x, event.touch.y))
            {
                _inertialVelocity = 0;
                _lastTouch = IntVector(event.touch.x, event.touch.y);
                _touchTimer.Restart();
            }
        }

        if (event.type == Event::TouchMoved)
        {
            if (!buttons.Contains(event.touch.x, event.touch.y))
            {
                Time delta = _touchTimer.Restart();

                float moveDistance = (float)(_lastTouch.y - event.touch.y);
                _inertialVelocity = moveDistance / delta.AsSeconds();
                _lastTouch = IntVector(event.touch.x, event.touch.y);
            }
        }

        if (event.type == Event::TouchEnded)
        {
            if (!buttons.Contains(event.touch.x, event.touch.y))
            {
                if (_touchTimer.GetElapsedTime().AsSeconds() > 0.3f)
                    _inertialVelocity = 0.f;
            }
        }
    }

    #define INERTIA_SCROLL_FACTOR 0.9f
    #define INERTIA_ACCELERATION 0.75f
    #define INERTIA_THRESHOLD 1.0f

    void    KeyboardImpl::_Update(float delta)
    {
        bool			isTouchDown = Touch::IsDown();
        IntVector		touchPos(Touch::GetPosition());

        if (!_customKeyboard)
        {
            int start = 0;
            int end = _keys->size();

            if (_layout == Layout::QWERTY)
            {
                // Symbols
                if (_useSymbols)
                {
                    if (!_symbolsPage)
                    {
                        start = 72;
                        end = 109;
                    }
                    else
                    {
                        start = 109;
                        end = 147;
                    }
                }
                // Nintendo
                else if (_useNintendo)
                {
                    if (!_nintendoPage)
                    {
                        start = 147;
                        end = 182;
                    }
                    else
                    {
                        start = 182;
                        end = 217;
                    }
                }
                else
                {
                    // Upper Case
                    if (_useCaps)
                    {
                        start = 36;
                        end = 72;
                    }
                    // Lower Case
                    else
                    {
                        start = 0;
                        end = 36;
                    }
                }
            }

            for (int i = start; i < end; i++)
            {
                (*_keys)[i].Update(isTouchDown, touchPos);
            }

            if (_showCursor && _blinkingClock.HasTimePassed(Seconds(0.5f)))
            {
                if (_cursorPositionOnScreen == 0)
                    _cursorPositionOnScreen = -1;
                else if (_cursorPositionOnScreen == -1)
                    _cursorPositionOnScreen = 0;
                else
                    _cursorPositionOnScreen = -_cursorPositionOnScreen;
                _blinkingClock.Restart();
            }
        }
        else ///< Custom Keyboard
        {
            if (_displayScrollbar)
            {
                _scrollSize = (_inertialVelocity * INERTIA_SCROLL_FACTOR * delta);

                _scrollPosition += _scrollSize;

                if (_scrollPosition <= 0.f)
                {
                    _scrollSize = _scrollSize - _scrollPosition;
                    _scrollPosition = 0.f;
                    _inertialVelocity = 0.f;
                }
                else if (_scrollPosition >= _scrollEnd)
                {
                    _scrollSize -= (_scrollPosition - _scrollEnd);
                    _scrollPosition = _scrollEnd;
                    _inertialVelocity = 0.f;
                }


                _inertialVelocity += (0.98f ) * delta;

                _inertialVelocity *= INERTIA_ACCELERATION;

                _currentPosition = (_scrollPosition * _scrollJump) / 36; //(_scrollPosition / 36);
                if (std::abs(_inertialVelocity) < INERTIA_THRESHOLD)
                    _inertialVelocity = 0.f;

                float scr = -_scrollSize * _scrollJump;

                for (TouchKeyString *tks : _strKeys)
                {
                    tks->Scroll(scr);
                    tks->Update(isTouchDown, touchPos);
                }
            }
            else
            {
                for (TouchKeyString *tks : _strKeys)
                    tks->Update(isTouchDown, touchPos);
            }
        }
    }

    /*
    ** _keys:
    **
    ** [0] = 'Q'
    ** [1] = 'W'
    ** [2] = 'E'
    ** [3] = 'R'
    ** [4] = 'T'
    ** [5] = 'Y'
    ** [6] = 'U'
    ** [7] = 'I'
    ** [8] = 'O'
    ** [9] = 'P'
    ** [10] = 'KEY_BACKSPACE'

    ** First Line : 11 keys **

    ** [11] = 'A'
    ** [12] = 'S'
    ** [13] = 'D'
    ** [14] = 'F'
    ** [15] = 'G'
    ** [16] = 'H'
    ** [17] = 'J'
    ** [18] = 'K'
    ** [19] = 'L'
    ** [20] = '''
    ** [21] = 'KEY_ENTER'

    ** Second Line : 11 keys **

     ** [22] = 'CAPS'
     ** [23] = 'Z'
     ** [24] = 'X'
     ** [25] = 'C'
     ** [26] = 'V'
     ** [27] = 'B'
     ** [28] = 'N'
     ** [29] = 'M'
     ** [30] = ','
     ** [31] = '.'
     ** [32] = '?'
     *
     ** Third Line : 11 keys **

     ** [33] = 'KEY_SYMBOL'
     ** [34] = 'SMILEY'
     ** [35] = 'SPACE'
     *
     ** Fourth Line : 3 keys **
    *************************/

    void    KeyboardImpl::_QwertyLowCase(void)
    {
        // Key rectangle
        IntRect pos(20, 36, 25, 40);

        /*low case*/
        _QwertyKeys.push_back(TouchKey('q', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('w', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('e', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('r', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('t', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('y', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('u', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('i', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('o', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('p', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 76;

        _QwertyKeys.push_back(TouchKey('a', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('s', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('d', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('f', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('g', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('h', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('j', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('k', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('l', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('\'', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 116;

        _QwertyKeys.push_back(TouchKey(KEY_CAPS, Icon::DrawCapsLockOn, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('z', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('x', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('c', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('v', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('b', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('n', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('m', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(',', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('.', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('?', pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 156;

        pos.size.x = 40;
        _QwertyKeys.push_back(TouchKey("+=@", pos, KEY_SYMBOLS)); pos.leftTop.x += 40;
        _QwertyKeys.push_back(TouchKey("\uE008", pos, KEY_SMILEY)); pos.leftTop.x += 40;
        pos.size.x = 120;
        _QwertyKeys.push_back(TouchKey("\uE057", pos, KEY_SPACE));
    }

    void    KeyboardImpl::_QwertyUpCase(void)
    {
        // Key rectangle
        IntRect pos(20, 36, 25, 40);

        /*low case*/
        _QwertyKeys.push_back(TouchKey('Q', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('W', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('E', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('R', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('T', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('Y', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('U', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('I', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('O', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('P', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 76;

        _QwertyKeys.push_back(TouchKey('A', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('S', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('D', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('F', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('G', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('H', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('J', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('K', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('L', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('"', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 116;

        _QwertyKeys.push_back(TouchKey(KEY_CAPS, Icon::DrawCapsLockOn, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('Z', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('X', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('C', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('V', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('B', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('N', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('M', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(';', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(':', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('!', pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 156;

        pos.size.x = 40;
        _QwertyKeys.push_back(TouchKey("+=@", pos, KEY_SYMBOLS)); pos.leftTop.x += 40;
        _QwertyKeys.push_back(TouchKey("\uE008", pos, KEY_SMILEY)); pos.leftTop.x += 40;
        pos.size.x = 120;
        _QwertyKeys.push_back(TouchKey("\uE057", pos, KEY_SPACE));
    }

    void KeyboardImpl::_QwertySymbols(void)
    {
        // Key rectangle
        IntRect pos(20, 36, 25, 40);

        /*page 1*/
        _QwertyKeys.push_back(TouchKey('?', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('!', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('@', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('#', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('$', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('%', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('&', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('1', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('2', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('3', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 76;

        _QwertyKeys.push_back(TouchKey('(', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(')', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('-', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('_', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('=', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00F7", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('+', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('4', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('5', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('6', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 116;

        _QwertyKeys.push_back(TouchKey("\u2192", pos, KEY_SYMBOLS_PAGE)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('\\', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(';', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(':', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('"', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('*', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('/', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('7', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('8', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('9', pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 156;

        pos.size.x = 40;
        _QwertyKeys.push_back(TouchKey("+=@", pos, KEY_SYMBOLS)); pos.leftTop.x += 40;
        _QwertyKeys.push_back(TouchKey("\uE008", pos, KEY_SMILEY)); pos.leftTop.x += 40;
        pos.size.x = 120;
        _QwertyKeys.push_back(TouchKey("\uE057", pos, KEY_SPACE)); pos.leftTop.x += 120;
        pos.size.x = 25;
        _QwertyKeys.push_back(TouchKey('0', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('.', pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 36;

        /*page 2*/
        _QwertyKeys.push_back(TouchKey("\u2022", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00A9", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u20AC", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00A3", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00A5", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00B5", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00A7", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('1', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('2', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('3', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 76;

        _QwertyKeys.push_back(TouchKey("\u2122", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('<', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('>', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('[', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(']', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('{', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('}', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('4', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('5', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('6', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 116;

        _QwertyKeys.push_back(TouchKey("\u2190", pos, KEY_SYMBOLS_PAGE)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('|', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00B2", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('`', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00B0", pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('~', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('^', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('7', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('8', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('9', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\u00B1", pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 156;

        pos.size.x = 40;
        _QwertyKeys.push_back(TouchKey("+=@", pos, KEY_SYMBOLS)); pos.leftTop.x += 40;
        _QwertyKeys.push_back(TouchKey("\uE008", pos, KEY_SMILEY)); pos.leftTop.x += 40;
        pos.size.x = 120;
        _QwertyKeys.push_back(TouchKey("\uE057", pos, KEY_SPACE)); pos.leftTop.x += 120;
        pos.size.x = 25;
        _QwertyKeys.push_back(TouchKey('0', pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey('.', pos));
    }

    void KeyboardImpl::_QwertyNintendo()
    {
        // Key rectangle
        IntRect pos(20, 36, 25, 40);

        /*page 1*/
        _QwertyKeys.push_back(TouchKey("\uE000" /* A */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE001" /* B */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE002" /* X */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE003" /* Y */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE004" /* L */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE005" /* R */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE054" /* ZL */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE055" /* ZR */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE006" /* DPAD */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE041" /* DPAD Wii*/, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 76;

        _QwertyKeys.push_back(TouchKey("\uE04C" /* a */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE04D" /* b */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE04E" /* x */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE04F" /* y */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE052" /* l */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE053" /* r */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE050" /* L Stick */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE051" /* R Stick */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE042" /* A Wii */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE043" /* B Wii */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 116;

        _QwertyKeys.push_back(TouchKey("\u2192", pos, KEY_NINTENDO_PAGE)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE040" /* Power */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE044" /* Home */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE045" /* + */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE046" /* - */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE047" /* 1 */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE048" /* 2 */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE049" /* Stick */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE04A" /* C */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE04B" /* Z */, pos)); pos.leftTop.x += 25;

        pos.leftTop.x = 20;
        pos.leftTop.y = 156;

        pos.size.x = 40;
        _QwertyKeys.push_back(TouchKey("+=@", pos, KEY_SYMBOLS)); pos.leftTop.x += 40;
        _QwertyKeys.push_back(TouchKey("\uE008", pos, KEY_SMILEY)); pos.leftTop.x += 40;

        pos.size.x = 120;
        _QwertyKeys.push_back(TouchKey("\uE057", pos, KEY_SPACE));
        pos.size.x = 25;

        pos.leftTop.x = 20;
        pos.leftTop.y = 36;

        /*page 2*/
        _QwertyKeys.push_back(TouchKey("\uE079" /* DPAD UP */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE07B" /* DPAD DOWN */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE07C" /* DPAD LEFT */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE07D" /* DPAD RIGHT */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE07E" /* DPAD UP&DOWN */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE07F" /* DPAD LEFT&RIGHT */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE077" /* Wii Stick */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE078" /* Wii Power */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE056" /* Enter */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE057" /* Space*/, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 76;

        _QwertyKeys.push_back(TouchKey("\uE007" /* Clock */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE008" /* Happy */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE009" /* Angry */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE00A" /* Sad */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE00B" /* ExpressionLess */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE00C" /* Sun */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE00D" /* Cloud */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE00E" /* Umbrella */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE00F" /* Snowman */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE06B" /* ? */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));

        pos.leftTop.x = 20;
        pos.leftTop.y = 116;

        _QwertyKeys.push_back(TouchKey("\u2190", pos, KEY_NINTENDO_PAGE)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE015" /*  */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE016" /*  */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE017" /*  */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE018" /*  */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE019" /* Arrow Right */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE01A" /* Arrow Left */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE01B" /* Arrow Up */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE01C" /* Arow Down */, pos)); pos.leftTop.x += 25;
        _QwertyKeys.push_back(TouchKey("\uE01E" /* Camera */, pos)); pos.leftTop.x += 25;

        pos.leftTop.x = 20;
        pos.leftTop.y = 156;

        pos.size.x = 40;
        _QwertyKeys.push_back(TouchKey("+=@", pos, KEY_SYMBOLS)); pos.leftTop.x += 40;
        _QwertyKeys.push_back(TouchKey("\uE008", pos, KEY_SMILEY)); pos.leftTop.x += 40;
        pos.size.x = 120;
        _QwertyKeys.push_back(TouchKey("\uE057", pos, KEY_SPACE));
    }


    void    KeyboardImpl::_Qwerty(void)
    {
        _keys = &_QwertyKeys;

        if (!_QwertyKeys.empty())
            return;

        /* 0 - 35*/
        _QwertyLowCase();
        /*36 - 71*/
        _QwertyUpCase();
        /* 72 - 108 Page 1*/
        /* 109 - 146 Page 2*/
        _QwertySymbols();
        /* 147 - 181 Page 1*/
        /* 182 - 216 Page 2*/
        _QwertyNintendo();
    }

    /*
    ** _keys:
    **
    ** [0] = 'A'
    ** [1] = 'B'
    ** [2] = 'C'
    ** [3] = 'D'
    ** [4] = 'E'
    ** [5] = 'F'
    ** [6] = '1'
    ** [7] = '2'
    ** [8] = '3'
    ** [9] = '4'
    ** [10] = '5'
    ** [11] = '6'
    ** [12] = '7'
    ** [13] = '8'
    ** [14] = '9'
    ** [15] = KEY_BACKSPACE
    ** [16] = KEY_ENTER
    ** [17] = '.'
    ** [18] = '0'
    *************************/

    void    KeyboardImpl::_DigitKeyboard(std::vector<TouchKey> &keys)
    {
        //Start
        IntRect pos(20, 36, 46, 46);

        // A - F
        char c = 'A';
        for (int i = 0; i < 6; i++, c++)
        {
            keys.push_back(TouchKey(c, pos));
            pos.leftTop.x += 46;

            if (i % 2)
            {
                pos.leftTop.x = 20;
                pos.leftTop.y += 46;
            }
        }

        pos.leftTop.y = 36;
        pos.leftTop.x = 112;

        // 1 - 9
        c = '1';
        for (int i = 0; i < 9; i++, c++)
        {
            keys.push_back(TouchKey(c, pos));
            pos.leftTop.x += 46;

            if (i % 3 == 2)
            {
                pos.leftTop.x = 112;
                pos.leftTop.y += 46;
            }
        }

        // Special keys
        pos.leftTop.y = 36;
        pos.leftTop.x = 250;

        // Clear key
        keys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));
        pos.leftTop.y += 46;

        // Enter key
        keys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));
        pos.leftTop.y += 46;

        // Dot key
        keys.push_back(TouchKey('.', pos));
        pos.leftTop.y += 46;

        // Plus-Minus key
        keys.push_back(TouchKey("\u00B1", pos));

        // 0 key
        pos.leftTop.y = 174;
        pos.leftTop.x = 112;
        pos.size.x = 138;
        keys.push_back(TouchKey('0', pos));
    }

    void    KeyboardImpl::_Decimal(void)
    {
        _keys = &_DecimalKeys;

        if (!_DecimalKeys.empty())
            return;

        _DigitKeyboard(_DecimalKeys);

        // Disable Hex keys
        KeyIter  iter = _DecimalKeys.begin();
        KeyIter  end = iter;
        std::advance(end, 6);

        for (; iter != end; ++iter)
            (*iter).Enable(false);
    }

    void    KeyboardImpl::_Hexadecimal(void)
    {
        _keys = &_HexaDecimalKeys;

        if (!_HexaDecimalKeys.empty())
            return;

        _DigitKeyboard(_HexaDecimalKeys);

        // Disable dot key
        _HexaDecimalKeys.at(17).Enable(false);

        // Disable pllus-minus key
        _HexaDecimalKeys.at(18).Enable(false);

        // Disable Hex keys if users asks so
        /*if (!_isHex)
        {
            KeyIter  iter = _HexaDecimalKeys.begin();
            KeyIter  end = iter;
            std::advance(end, 6);

            for (; iter != end; ++iter)
                (*iter).Enable(false);
        } */
    }

    static int UnitsToAdvance(const char *str, u32 target)
    {
        const u8 *s = reinterpret_cast<const u8 *>(str);
        int units = 0;

        // Skip UTF8 sig
        if (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF)
            s += 3;

        while (*s && target > 0)
        {
            if (*s == 0x18)
            {
                ++s;
                ++units;
                --target;
                continue;
            }

            if (*s == 0x1B)
            {
                s += 4;
                units += 4;
                target -= 4;
                continue;
            }

            u32 code;
            int unit = decode_utf8(&code, s);

            if (code == 0)
                break;
            if (unit == -1)
                return -1;

            s += unit;
            units += unit;
            target -= unit;
        }
        return units;
    }

    /*
    static int     GetPreviousCharUnits(const char *cstr, u32 position)
    {
        int unitss = 0;

        cstr--;
        while (*cstr)
        {

            if (*cstr == 0x18)
            {
                ++unitss;
                cstr--;
                continue;
            }
            if (position >= 3 *(cstr - 3) == 0x1B)
            u32 code;

            int units = decode_utf8()

            cstr--;
        }
        return unitss;
    } */
    int     UnitsToNextChar(const char *cstr, int left)
    {
        int units = 0;

        while (*cstr && left > 0)
        {
            if (*cstr == 0x1B)
            {
                units += 4;
                left -= 4;
                cstr += 4;
                continue;
            }

            if (*cstr == 0x18)
            {
                ++units;
                --left;
                ++cstr;
                continue;
            }

            u32 code;
            int u = decode_utf8(&code, (u8 *)cstr);

           /* if (code == 0)
                break;*/
            if (u == -1)
                return u;

            units += u;
            break;
        }
        return units;
    }

    int     UnitsToPreviousChar(const char *cstr, int cursor)
    {
        int units = 0;

        --cstr;
        while (*cstr && cursor > 0)
        {
            --cursor;
            if (cursor > 4 && *(cstr - 3) == 0x1B)
            {
                units += 4;
                cursor -= 4;
                cstr -= 3;
                continue;
            }

            if (*cstr == 0x18)
            {
                ++units;
                --cursor;
                --cstr;
                continue;
            }

            u32 code;
            int u = decode_utf8(&code, (u8 *)cstr);

          /*  if (code == 0)
                break;*/

            if (u == -1 && !cursor)
                return u;

            if (u != -1)
            {
                units += u;
                break;
            }
            --cstr;
        }
        return units;
    }

    void    KeyboardImpl::_ScrollUp(void)
    {
        const u32 strLength = _userInput.size();
        const char *cstr = _userInput.c_str();
        const float strWidth = Renderer::GetTextSize(cstr);

        // If input is empty
        if (strLength == 0)
            goto empty;

        {
            // Get units to advance to go to the next char
            const int units = UnitsToNextChar(cstr + _cursorPositionInString, strLength - _cursorPositionInString);

            if (units < 0)
            {
                ///< Weird char being found, better clear the input than abort/crash later
                goto error;
            }

            // Increase cursor position
            _cursorPositionInString += units;
        }
        // cursor must be at most, after the last char of input
        if (_cursorPositionInString > strLength)
            _cursorPositionInString = strLength;

        // Update graphics infos
        _UpdateScrollInfos(cstr, strWidth);
        return;

    error:
        _userInput.clear();
    empty:
        _offset = 0.f;
        _cursorPositionInString = _cursorPositionOnScreen = 0;
        return;

    }

    void    KeyboardImpl::_ScrollDown(void)
    {
        const u32 strLength = _userInput.size();
        const char *cstr = _userInput.c_str();
        const float strWidth = Renderer::GetTextSize(cstr);

        // If input is empty
        if (strLength == 0)
            goto empty;

        {
            // Get units to advance to go to the next char
            const int units = UnitsToPreviousChar(cstr + _cursorPositionInString, _cursorPositionInString);

            if (units < 0)
            {
                ///< Weird char being found, better clear the input than abort/crash later
                goto error;
            }

            // Increase cursor position
            _cursorPositionInString -= units;
        }

        // cursor must be at most, at the begining of the string
        if (_cursorPositionInString < 0)
            _cursorPositionInString = 0;

        // Update graphics infos
        _UpdateScrollInfos(cstr, strWidth);
        return;

    error:
        _userInput.clear();
    empty:
        _offset = 0.f;
        _cursorPositionInString = _cursorPositionOnScreen = 0;
        return;
    }

    void    KeyboardImpl::_UpdateScrollInfos(const char *cstr, float strWidth)
    {
        _blinkingClock.Restart();

        // Get units untils current cursor position
        const int unitsToCursor = UnitsToAdvance(cstr, _cursorPositionInString);

        // Weird character, might as well purge the string
        if (unitsToCursor < 0)
        {
            _offset = 0.f;
            _cursorPositionInString = _cursorPositionOnScreen = 0;
            _userInput.clear();
            return;
        }

        // Get width of both before and after cursor
        const float after = Renderer::GetTextSize(cstr + unitsToCursor);
        const float before = strWidth - after;

        // Compute offsets
        if (strWidth > 260.f)
        {
            if (before > 140.f)
                _offset = before + std::min(after, 140.f) - 260.f;
            else
                _offset = 0.f;
            _cursorPositionOnScreen = before - _offset;
        }
        else
        {
            _offset = 0;
            _cursorPositionOnScreen = before;
        }
    }

    enum
    {
        CLEAR_NOT_PRESSED = 0,
        CLEAR_JUST_PRESSED,
        CLEAR_QUICK_MODE
    };
    bool    KeyboardImpl::_CheckKeys(void)
    {
        static Clock    backspacetimer;
        static u32      backspaceFastMode = CLEAR_NOT_PRESSED;
        static Time     FastModeWaitTime = Seconds(0.5f);
        static Time     FastClearingFrame = Seconds(0.1f);

        int start = 0;
        int end = _keys->size();

        if (_layout == Layout::QWERTY)
        {
            // Symbols
            if (_useSymbols)
            {
                if (!_symbolsPage)
                {
                    start = 72;
                    end = 109;
                }
                else
                {
                    start = 109;
                    end = 147;
                }
            }
            // Nintendo
            else if (_useNintendo)
            {
                if (!_nintendoPage)
                {
                    start = 147;
                    end = 182;
                }
                else
                {
                    start = 182;
                    end = 217;
                }
            }
            else
            {
                // Upper Case
                if (_useCaps)
                {
                    start = 36;
                    end = 72;
                }
                // Lower Case
                else
                {
                    start = 0;
                    end = 36;
                }
            }
        }

        // Check cursor position, just in case
        if (_cursorPositionInString > _userInput.size())
            _ScrollUp();

        for (int i = start; i < end; i++)
        {
            std::string  temp;

            int ret = (*_keys)[i](temp);

            if (ret != -1)
            {
                if (ret == ~KEY_BACKSPACE)
                {
                    backspaceFastMode = CLEAR_NOT_PRESSED;
                    goto _backspacePressed;
                }
                if (ret == KEY_BACKSPACE)
                {
                    if (backspaceFastMode == CLEAR_NOT_PRESSED)
                    {
                        backspaceFastMode = CLEAR_JUST_PRESSED;
                        backspacetimer.Restart();
                        return (false);
                    }
                    if (backspaceFastMode == CLEAR_JUST_PRESSED)
                    {
                        if (!backspacetimer.HasTimePassed(FastModeWaitTime))
                            return (false);
                        backspaceFastMode = CLEAR_QUICK_MODE;
                    }
                    if (backspaceFastMode == CLEAR_QUICK_MODE)
                    {
                        if (!backspacetimer.HasTimePassed(FastClearingFrame))
                            return (false);
                    }
                    backspacetimer.Restart();
                    goto _backspacePressed;
                }

                // Reset backspace state if any other key is pressed
                backspaceFastMode = CLEAR_NOT_PRESSED;

                if (ret == 0x12345678)
                {
                    if ((_layout == DECIMAL && _userInput.size() >= 18)
                        || (_layout == QWERTY && _max && Utils::GetSize(_userInput) >= _max))
                        return (false);

                    _inputChangeEvent.type = InputChangeEvent::CharacterAdded;
                    decode_utf8(&_inputChangeEvent.codepoint, (const u8 *)temp.c_str());

                    if (_inputChangeEvent.codepoint == 0x00B1) // � key
                    {
                        if (_userInput[0] == '-')
                        {
                            _userInput.erase(0, 1);
                            _ScrollDown();
                        }
                        else
                        {
                            _userInput.insert(0, 1, '-');
                            _ScrollUp();
                        }
                    }
                    else
                    {
                        _userInput.insert(_cursorPositionInString, temp);
                        _ScrollUp();
                    }
                    return (true);
                }
                if (ret == KEY_ENTER)
                {
                    _askForExit = true;
                    return (false);
                }
                else if (ret == KEY_SPACE && (!_max || Utils::GetSize(_userInput) < _max))
                {
                    _userInput.insert(_cursorPositionInString, " ");
                    _ScrollUp();
                    return (true);
                }
                else if (ret == KEY_CAPS)
                {
                    _useCaps = !_useCaps;
                }
                else if (ret == KEY_SMILEY)
                {
                    _useNintendo = !_useNintendo;
                    _useSymbols = false;
					_useCaps = false;
					if (!_useNintendo)
						_nintendoPage = 0;
					_symbolsPage = 0;
                }
                else if (ret == KEY_SYMBOLS)
                {
                    _useSymbols = !_useSymbols;
                    _useNintendo = false;
					_useCaps = false;
					if (!_useSymbols)
						_symbolsPage = 0;
					_nintendoPage = 0;
                }
                else if (ret == KEY_SYMBOLS_PAGE)
                {
                    _symbolsPage = !_symbolsPage;
                }
                else if (ret == KEY_NINTENDO_PAGE)
                {
                    _nintendoPage = !_nintendoPage;
                }
                else
                {
                    if (_layout == DECIMAL && _userInput.length() >= 18)
                        return (false);

                    if (_layout != Layout::QWERTY &&_cursorPositionInString == 0 && ret == '.')
                    {
                        _userInput += "0.";
                        _ScrollUp();
                        _ScrollUp(); ///< Yeah I know, I'm f*cking lazy
                    }
                    else if (_max == 0 || Utils::GetSize(_userInput) < _max)
                    {
                        temp.clear();
                        temp += ret;
                        _userInput.insert(_cursorPositionInString, temp);
                        _inputChangeEvent.type = InputChangeEvent::CharacterAdded;
                        _inputChangeEvent.codepoint = ret;
                        _ScrollUp();
                    }
                    return (true);
                }
            }
        }

        backspaceFastMode = CLEAR_NOT_PRESSED;
        return (false);

    _backspacePressed:
        std::string &&right = _userInput.substr(_cursorPositionInString);
        _userInput.erase(_cursorPositionInString);
        _inputChangeEvent.codepoint = Utils::RemoveLastChar(_userInput);
        _userInput += right;
        if (_inputChangeEvent.codepoint != 0)
        {
            _inputChangeEvent.type = InputChangeEvent::CharacterRemoved;
            _ScrollDown();
            return (true);
        }
        return (false);
    }

    bool    KeyboardImpl::_CheckInput(void)
    {
        if (_layout == QWERTY)
        {
            if (_compare != nullptr)
                return (_compare((void *)&_userInput, _error));
            return (true);
        }

        // In case there's no convert function, always consider input as valid
        if (_convert == nullptr)
            return (true);

        // Always call convert function, can avoid overflow
        void *convertedInput = _convert(_userInput, _isHex);

        // In case there's no callback, always consider input as valid
        if (_compare == nullptr)
            return (true);

        return (_compare(convertedInput, _error));
    }

    bool    KeyboardImpl::_CheckButtons(int &ret)
    {

        for (int i = 0; i < _strKeys.size(); i++)
        {
            ret = (*_strKeys[i])();
            if (ret != -1)
            {
                ret = i;
                return (true);
            }
        }

        return (false);
    }

    // WIll only be used in the hex editor, so no need to do a full implementation
    bool    KeyboardImpl::operator()(int &out)
    {
        _Update(0.f);
        if (!_CheckKeys())
            return (false);
        if (_userInput.size())
        {
            out = (int)_userInput[0];
            if (out >= '0' && out <= '9')
                out -= '0';
            if (out >= 'A' && out <= 'F')
                out = 10 + out - 'A';
            _userInput.pop_back();
            return (true);
        }
        return (false);
    }

    void    KeyboardImpl::CanChangeLayout(bool canChange)
    {
        _canChangeLayout = canChange;
    }
}
