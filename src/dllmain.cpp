#include <MaiSense/Sensor.hpp>
#include <MaiSense/InputManager.hpp>
#include <MaiSense/TouchController.hpp>
#include <MaiSense/KeyboardController.hpp>
#include <MaiSense/MouseController.hpp>
#include <MaiSense/SensorChecker.hpp>
#include <MaiSense/SensorProcessor.hpp>

#include <string>

#pragma warning (disable : 4996)

using namespace MaiSense;

TouchController touchController;
KeyboardController keyboardController;
MouseController mouseController;
SensorChecker sensorChecker;
SensorProcessor processor;

typedef DWORD(WINAPI* pfWaitForSingleObject)(HANDLE,DWORD);
pfWaitForSingleObject OldWait = NULL;
DWORD WINAPI MyWait(HANDLE hdl, DWORD dwd)
{
    DWORD dwd2 = dwd/2;
    return OldWait(hdl,dwd2);
}
PIMAGE_NT_HEADERS GetLocalNtHead()
{
    DWORD dwTemp = NULL;
    PIMAGE_DOS_HEADER pDosHead = NULL;
    PIMAGE_NT_HEADERS pNtHead = NULL;
    HMODULE ImageBase = GetModuleHandle(NULL);                              // 取自身ImageBase
    pDosHead = (PIMAGE_DOS_HEADER)(DWORD)ImageBase;                         // 取pDosHead地址
    dwTemp = (DWORD)pDosHead + (DWORD)pDosHead->e_lfanew;
    pNtHead = (PIMAGE_NT_HEADERS)dwTemp;                                    // 取出NtHead头地址
    return pNtHead;
}
void IATHook()
{
    PVOID pFuncAddress = NULL;
    HINSTANCE dll_file = GetModuleHandle(L"kernel32.dll");
    pFuncAddress = GetProcAddress(dll_file, "WaitForSingleObject");  // 取Hook函数地址
    OldWait = (pfWaitForSingleObject)pFuncAddress;
    PIMAGE_NT_HEADERS pNtHead = GetLocalNtHead();                                  // 获取到程序自身NtHead
    PIMAGE_FILE_HEADER pFileHead = (PIMAGE_FILE_HEADER)&pNtHead->FileHeader;
    PIMAGE_OPTIONAL_HEADER pOpHead = (PIMAGE_OPTIONAL_HEADER)&pNtHead->OptionalHeader;

    DWORD dwInputTable = pOpHead->DataDirectory[1].VirtualAddress;    // 找出导入表偏移
    DWORD dwTemp = (DWORD)GetModuleHandle(NULL) + dwInputTable;
    PIMAGE_IMPORT_DESCRIPTOR   pImport = (PIMAGE_IMPORT_DESCRIPTOR)dwTemp;
    PIMAGE_IMPORT_DESCRIPTOR   pCurrent = pImport;
    DWORD* pFirstThunk; //导入表子表,IAT存储函数地址表.
    //遍历导入表
    while (pCurrent->FirstThunk != NULL)
    {
        dwTemp = pCurrent->FirstThunk + (DWORD)GetModuleHandle(NULL);// 找到内存中的导入表
        pFirstThunk = (DWORD*)dwTemp;                               // 赋值 pFirstThunk
        while (*(DWORD*)pFirstThunk != NULL)                         // 不为NULl说明没有结束
        {
            if (*(DWORD*)pFirstThunk == (DWORD)pFuncAddress)       // 相等说明正是我们想要的地址
            {
                MessageBoxA(NULL, "hi", "hi", MB_OK);
                DWORD oldProtected;
                VirtualProtect(pFirstThunk, 0x1000, PAGE_EXECUTE_READWRITE, &oldProtected);  // 开启写权限
                dwTemp = (DWORD)MyWait;
                //(DWORD*)&dwTemp
                memcpy(pFirstThunk, (DWORD*)&dwTemp, 4);                                    // 将MyMessageBox地址拷贝替换
                VirtualProtect(pFirstThunk, 0x1000, oldProtected, &oldProtected);            // 关闭写保护
            }
            pFirstThunk++; // 继续递增循环
        }
        pCurrent++;        // 每次是加1个导入表结构.
    }
}
BOOL APIENTRY DllMain(HMODULE hMod, DWORD cause, LPVOID lpReserved)
{
    
    if (!InputManager::Ready())
        return TRUE;

    if (cause == DLL_PROCESS_ATTACH) 
    {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        IATHook();
        AllocConsole();
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);

        touchController.SetCallback([&](TouchEvent ev) 
        {
            processor.Handle(ev);
            if (sensorChecker.GetScreenWidth() == 0 || sensorChecker.GetScreenHeight() == 0)
            {
                RECT clientRect;
                GetClientRect(InputManager::GetGameWindow(), &clientRect);
                sensorChecker.SetScreenSize
                (
                    clientRect.left + clientRect.right,
                    clientRect.top + clientRect.bottom
                );
            }

            if (processor.GetChecker() == NULL)
                processor.SetChecker(&sensorChecker);

            if (processor.GetSensor() == NULL)
                processor.SetSensor(InputManager::GetSensor());
        });

        keyboardController.SetCallback([&](KeyEvent ev)
        {
            auto sensor = InputManager::GetSensor();
            switch (ev.KeyCode)
            {
            case 0x30: // 0
                sensor->Queue(Sensor::C, ev.Active);
                break;
            case 0x31: // 1
                sensor->Queue(Sensor::A1, ev.Active);
                break;
            case 0x32: // 2
                sensor->Queue(Sensor::A2, ev.Active);
                break;
            case 0x33: // 3
                sensor->Queue(Sensor::A3, ev.Active);
                break;
            case 0x34: // 4
                sensor->Queue(Sensor::A4, ev.Active);
                break;
            case 0x35: // 5
                sensor->Queue(Sensor::A5, ev.Active);
                break;
            case 0x36: // 6
                sensor->Queue(Sensor::A6, ev.Active);
                break;
            case 0x37: // 7
                sensor->Queue(Sensor::A7, ev.Active);
                break;
            case 0x38: // 8
                sensor->Queue(Sensor::A8, ev.Active);
                break;
            case VK_NUMPAD1: // NUMPAD 1
                sensor->Queue(Sensor::B1, ev.Active);
                break;
            case VK_NUMPAD2: // NUMPAD 2
                sensor->Queue(Sensor::B2, ev.Active);
                break;
            case VK_NUMPAD3: // NUMPAD 3
                sensor->Queue(Sensor::B3, ev.Active);
                break;
            case VK_NUMPAD4: // NUMPAD 4
                sensor->Queue(Sensor::B4, ev.Active);
                break;
            case VK_NUMPAD5: // NUMPAD 5
                sensor->Queue(Sensor::B5, ev.Active);
                break;
            case VK_NUMPAD6: // NUMPAD 6
                sensor->Queue(Sensor::B6, ev.Active);
                break;
            case VK_NUMPAD7: // NUMPAD 7
                sensor->Queue(Sensor::B7, ev.Active);
                break;
            case VK_NUMPAD8: // NUMPAD 8
                sensor->Queue(Sensor::B8, ev.Active);
                break;
            }
        });
        mouseController.SetCallback([&](MouseEvent ev)
        {
            if (ev.MButton)
                mouseController.EmulateTouch();
        });

        InputManager::Hook();
        InputManager::Install(&touchController);
        InputManager::Install(&keyboardController);
        InputManager::Install(&mouseController);
    }
    else if (cause == DLL_PROCESS_DETACH) 
    {
        InputManager::Unhook();
    }

    return TRUE;
}
