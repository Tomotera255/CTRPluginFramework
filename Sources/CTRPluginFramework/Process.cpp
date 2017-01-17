#include "CTRPluginFramework.hpp"
#include "arm11kCommands.h"
#include "3DS.h"
#include <cstdio>
#include <cstring>

extern 		Handle gspThreadEventHandle;

namespace CTRPluginFramework
{
	u32         Process::_processID = 0;
	u64         Process::_titleID = 0;
	char        Process::_processName[8] = {0};
	u32         Process::_kProcess = 0;
	u32			Process::_kProcessState = 0;
	KCodeSet    Process::_kCodeSet = {0};
	Handle 		Process::_processHandle = 0;
	Handle 		Process::_mainThreadHandle = 0;
	Handle 		Process::_keepEvent = 0;
	bool 		Process::_isPaused = false;

	void    Process::Initialize(Handle keepEvent)
	{
		char    kproc[0x100] = {0};
		bool 	isNew3DS = System::IsNew3DS();

		// Get current KProcess
		_kProcess = (u32)arm11kGetCurrentKProcess();
		
		// Copy KProcess data
		arm11kMemcpy((u32)&kproc, _kProcess, 0x100);
		if (isNew3DS)
		{
			// Copy KCodeSet
			arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB8), sizeof(KCodeSet));          

			// Copy process id
			_processID = *(u32 *)(kproc + 0xBC);
			_kProcessState = _kProcess + 0x88;
		}
		else
		{
			// Copy KCodeSet
			arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB0), sizeof(KCodeSet));          

			// Copy process id
			_processID = *(u32 *)(kproc + 0xB4);
			_kProcessState = _kProcess + 0x80;
		}

		// Copy process name
		for (int i = 0; i < 8; i++)
				_processName[i] = _kCodeSet.processName[i];

		// Copy title id
		_titleID = _kCodeSet.titleId;
		// Create handle for this process
		svcOpenProcess(&_processHandle, _processID);
		// Set plugin's main thread handle
		_mainThreadHandle = threadGetCurrent()->handle;
		_keepEvent = keepEvent;
	}

	bool 	Process::IsPaused(void)
	{
		return (_isPaused);
	}

	Handle 	Process::GetHandle(void)
	{
		return (_processHandle);
	}

	u32     Process::GetProcessID(void)
	{
		return (_processID);
	}

	void     Process::GetProcessID(char *output)
	{
		if (!output)
			return;
		sprintf(output, "%02X", _processID);
	}

	u64     Process::GetTitleID(void)
	{
		return (_titleID);
	}

	void     Process::GetTitleID(char *output)
	{
		if (!output)
			return;
		sprintf(output, "%016llX", _titleID);
	}

	void    Process::GetName(char *output)
	{
		if (output != nullptr)
			for (int i = 0; i < 8; i++)
				output[i] = _processName[i];
	}

	u8 		Process::GetProcessState(void)
	{
		return (arm11kGetKProcessState(_kProcessState));
	}

	bool 	Process::Patch(u32 	addr, u8 *patch, u32 length, u8 *original)
	{
		return (PatchProcess(addr, patch, length, original));
	}

	void 	Process::Pause(void)
	{
		svcSetThreadPriority(gspThreadEventHandle, 0x17);

		// Attempts to always get different framebuffers
		gspWaitForVBlank();
		svcSetThreadPriority(_mainThreadHandle, 0x16);
		u32 	top[2];
		u32 	bot[2];
		Screen::Top->GetLeftFramebufferRegisters(top);
		Screen::Bottom->GetLeftFramebufferRegisters(bot);
		while (1)
		{
			if ((*(u32 *)(top[0]) != *(u32 *)(top[1]))
				&& (*(u32 *)(bot[0]) != *(u32 *)(bot[1])))
				break;
			svcSleepThread(100);
		}
		_isPaused = true;
		svcSignalEvent(_keepEvent);
		Screen::Top->Acquire();
        Screen::Bottom->Acquire();        
	}

	void 	Process::Play(void)
	{
		_isPaused = false;
		svcSetThreadPriority(gspThreadEventHandle, 0x3F);
		svcSetThreadPriority(_mainThreadHandle, 0x3F);
	}

    bool     Process::ProtectMemory(u32 addr, u32 size, int perm)
    {
    	if (R_FAILED(svcControlProcessMemory(_processHandle, addr, addr, size, 6, perm)))
        	return (false);
        return (true);
    }

    bool     Process::ProtectRegion(u32 addr, int perm)
    {
    	MemInfo 	minfo;
    	PageInfo 	pinfo;

    	if (R_FAILED(svcQueryProcessMemory(&minfo, &pinfo, _processHandle, addr))) goto error;
    	if (minfo.state == MEMSTATE_FREE) goto error;
    	if (addr < minfo.base_addr || addr > minfo.base_addr + minfo.size) goto error;

    	return (ProtectMemory(minfo.base_addr, minfo.size, perm));
    error:
        return (false);
    }

    bool     Process::PatchProcess(u32 addr, u8 *patch, u32 length, u8 *original)
    {
        if (!(ProtectMemory(((addr / 0x1000) * 0x1000), 0x1000))) goto error;
 
 		if (original != nullptr)
 		{
 			if (!CopyMemory((void *)original, (void *)addr, length))
 				goto error;
 		}

 		if (!CopyMemory((void *)addr, (void *)patch, length))
 			goto error;
        return (true);
    error:
        return (false);
    }

    bool     Process::CopyMemory(void *dst, void *src, u32 size)
    {
        if (R_FAILED(svcFlushProcessDataCache(_processHandle, src, size))) goto error;
        if (R_FAILED(svcFlushProcessDataCache(_processHandle, dst, size))) goto error;
        std::memcpy(dst, src, size);
        svcInvalidateProcessDataCache(_processHandle, dst, size);
        return (true);
    error:
        return (false);
    }
}
