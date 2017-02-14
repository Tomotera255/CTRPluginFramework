#include "CTRPluginFrameworkImpl/Menu/GuideReader.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    MenuFolderImpl *CreateFolder(std::string path)
    {
        u32                         pos = path.rfind("/");
        std::string                 name = pos != std::string::npos ? path.substr(pos + 1) : path;
        MenuFolderImpl                  *mFolder = new MenuFolderImpl(name);
        Directory                   folder;
        std::vector<std::string>    directories;
        std::vector<std::string>    files;

        if (Directory::Open(folder, path) != 0)
        {
            delete mFolder;
            return (nullptr);
        }

        mFolder->note = path;

        // List all directories
        folder.ListFolders(directories);
        if (!directories.empty())
        {
            for (int i = 0; i < directories.size(); i++)
            {
                MenuFolderImpl *subMFolder = CreateFolder(path + "/" + directories[i]);
                if (subMFolder != nullptr)
                    mFolder->Append(subMFolder);
            }
        }

        // List all files
        folder.ListFiles(files, ".txt");
        if (!files.empty())
        {
            for (int i = 0; i < files.size(); i++)
            {
                u32 fpos = files[i].rfind(".txt");
                std::string fname = fpos != std::string::npos ? files[i].substr(0, fpos) : files[i];
                MenuEntryImpl *entry = new MenuEntryImpl(fname, path);
                mFolder->Append(entry);
            }
        }
        return (mFolder);
    }

    GuideReader::GuideReader(void) :
    _text(""), _isOpen(false), _guideTB(nullptr), _last(nullptr), _menu(CreateFolder("Guide")),
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose)
    {
        _isOpen = false;
        _image = nullptr;
        if (Directory::Open(_currentDirectory, "Guide") == 0)
        {
            _currentDirectory.ListFiles(_bmpList, ".bmp");
            if (!_bmpList.empty())
            {
                _currentBMP = 0;
                _image = new BMPImage("Guide/" + _bmpList[0]);
            }
            else
                _currentBMP = -1;
        }
        else
        {
            _currentBMP = -1;
        }

    }

    bool    GuideReader::operator()(EventList &eventList, Time &delta)
    {
        _isOpen = true;
        // Process event
        for (int i = 0; i < eventList.size(); i++)
            _ProcessEvent(eventList[i]);

        // Draw
        Draw();
        return (_closeBtn() || !_isOpen);
    }

    bool    GuideReader::Draw(void)
    {
        if (!_isOpen)
            return (false);

        /*
        ** Top Screen
        **************************************************/
        Renderer::SetTarget(TOP);
        // If a textbox exist
        if (_guideTB != nullptr && _guideTB->IsOpen())
        {
            _guideTB->Draw();
        }
        else
        {
            _menu.Draw();
        }

        /*
        ** Bottom Screen
        **************************************************/
        Renderer::SetTarget(BOTTOM);

        static IntRect  background(20, 20, 280, 200);
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);

        if (_image != nullptr && _image->IsLoaded())
            _image->Draw(background);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(22, 22, 276, 196, blank, false);
        }

        bool isTouchDown = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());
        _closeBtn.Update(isTouchDown, touchPos);
        _closeBtn.Draw();

    }

    bool    GuideReader::_ProcessEvent(Event &event)
    {
        if (!_isOpen)
            return (false);

        // Process TextBox Event
        if (_guideTB != nullptr && _guideTB->IsOpen())
        {
            _guideTB->ProcessEvent(event);
        }
        else
        {
            MenuItem *item = nullptr;
            // Process Menu Event
            int ret = _menu.ProcessEvent(event, &item);

            // If menu ask for close
            if (ret == MenuEvent::MenuClose)
            {
                Close();
                return (false);
            }

            // If folder changed
            if (ret == MenuEvent::FolderChanged)
            {
                delete _image;
                _image = nullptr;
                _currentBMP = -1;
                _bmpList.clear();
                if (item != nullptr && Directory::Open(_currentDirectory, item->note) == 0)
                {
                    _currentDirectory.ListFiles(_bmpList, ".bmp");
                    if (!_bmpList.empty())
                    {
                        _currentBMP = 0;
                        _image = new BMPImage(item->note + "/" + _bmpList[0]);
                    }
                }
            }
            // If an file entry was selected by user
            if (ret == MenuEvent::EntrySelected)
            {
                MenuEntryImpl *entry = (MenuEntryImpl *)item;
                if (entry != _last)
                {
                    _last = entry;
                    if (_guideTB != nullptr)
                    {
                        delete _guideTB;
                        _guideTB = nullptr;
                    }

                    File file;
                    if (File::Open(file, entry->note + "/" + entry->name + ".txt") != 0)
                        return (false);

                    u64 size = file.GetSize();
                    _text.clear();

                    char *data = new char[size + 2];
                    memset(data, 0, size + 2);
                    

                    file.Rewind();
                    if (file.Read(data, size) != 0)
                        return (false);

                    _text = data;
                    _text[size] = '\0';

                    delete[] data;
                    IntRect tb = IntRect(30, 20, 340, 200);

                    _guideTB = new TextBox(entry->name, _text, tb);
                    Color limegreen(50, 205, 50);
                    _guideTB->titleColor = limegreen;
                    _guideTB->borderColor = limegreen;
                    _guideTB->Open();
                }
                else
                    _guideTB->Open();
            }
        }

        // Process Event
        if (_currentBMP != -1)
        if (event.type == Event::EventType::TouchSwipped)
        {
            switch (event.swip.direction)
            {
                case Event::SwipDirection::Left:
                {
                    if (_currentBMP > 0)
                    {
                        _currentBMP--;
                        delete _image;
                        _image = new BMPImage(_currentDirectory.GetPath() + "/" + _bmpList[_currentBMP]);
                    }
                    break;
                }
                case Event::SwipDirection::Right:
                {
                    if (_currentBMP < _bmpList.size() - 1)
                    {
                        _currentBMP++;
                        delete _image;
                        _image = new BMPImage(_currentDirectory.GetPath() + "/" + _bmpList[_currentBMP]);
                    }
                    break;
                }
            }
        }
        return (true);
    }

    void    GuideReader::Open(void)
    {
        _isOpen = true;
    }

    void    GuideReader::Close(void)
    {
        _isOpen = false;
    }

    bool    GuideReader::IsOpen(void)
    {
        return (_isOpen);
    }
}