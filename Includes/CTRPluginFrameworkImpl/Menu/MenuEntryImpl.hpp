#ifndef CTRPLUGINFRAMEWORKIMPL_MENUENTRYIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUENTRYIMPL_HPP

#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"
#include "CTRPluginFramework/Menu/MenuEntry.hpp"
#include <string>

namespace CTRPluginFramework
{
    class MenuEntryImpl;
    
    using FuncPointer = void (*)(MenuEntry*);
    
    class MenuEntryImpl : public MenuItem
    {
        struct Flags
        {
            bool  state : 1;
            bool  justChanged : 1;
            bool  isRadio : 1;
        };

    public:
        MenuEntryImpl(std::string name, std::string note = "", MenuEntry *owner = nullptr);
        MenuEntryImpl(std::string name, FuncPointer func, std::string note = "", MenuEntry *owner = nullptr);
        ~MenuEntryImpl(){};

        // Disable the entry
        void    Disable(void);
        // Set the entry as radio, an ID must be provided
        void    SetRadio(int id);        
        // Set an argument for the entry
        void    SetArg(void *arg);
        // Get the argument
        void    *GetArg(void);
        // Return if the entry just got activated
        bool    WasJustActivated(void);
        // Return if the entry is activated
        bool    IsActivated(void);

        // Public members
        FuncPointer     GameFunc;
        FuncPointer     MenuFunc;

    private:
        friend class PluginMenuImpl;

        // Functions used by the menu
        bool    _TriggerState(void);
        bool    _MustBeRemoved(void);
        bool    _Execute(void);
        int     _executeIndex;
        MenuEntry *_owner;

        Flags       _flags;
        int         _radioId;
        void        *_arg;
    };
}

#endif