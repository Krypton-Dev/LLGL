/*
 * IOSDisplay.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IOSDisplay.h"
#include "../../Core/Helper.h"


namespace LLGL
{


std::vector<std::unique_ptr<Display>> Display::InstantiateList()
{
    std::vector<std::unique_ptr<Display>> displayList;
    displayList.push_back(Display::InstantiatePrimary());
    return displayList;
}

std::unique_ptr<Display> Display::InstantiatePrimary()
{
    return MakeUnique<IOSDisplay>();
}

bool Display::ShowCursor(bool show)
{
    return false;
}

bool Display::IsCursorShown()
{
    return false;
}


/*
 * IOSDisplay class
 */

bool IOSDisplay::IsPrimary() const
{
    return true;
}

std::wstring IOSDisplay::GetDeviceName() const
{
    //TODO
    return L"";
}

Offset2D IOSDisplay::GetOffset() const
{
    //TODO
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    return Offset2D
    {
        static_cast<std::int32_t>(screenRect.origin.x),
        static_cast<std::int32_t>(screenRect.origin.y)
    };
}

bool IOSDisplay::ResetDisplayMode()
{
    //TODO
    return false;
}

bool IOSDisplay::SetDisplayMode(const DisplayModeDescriptor& displayModeDesc)
{
    //TODO
    return false;
}

DisplayModeDescriptor IOSDisplay::GetDisplayMode() const
{
    DisplayModeDescriptor displayModeDesc;
    {
        /* Get pixel size from main display */
        CGRect screenRect = [[UIScreen mainScreen] bounds];
        displayModeDesc.resolution.width    = static_cast<std::uint32_t>(screenRect.size.width);
        displayModeDesc.resolution.height   = static_cast<std::uint32_t>(screenRect.size.height);
        displayModeDesc.refreshRate         = 60; //TODO
    }
    return displayModeDesc;
}

std::vector<DisplayModeDescriptor> IOSDisplay::GetSupportedDisplayModes() const
{
    std::vector<DisplayModeDescriptor> displayModeDescs;

    //TODO
    displayModeDescs.push_back(GetDisplayMode());

    return displayModeDescs;
}


} // /namespace LLGL



// ================================================================================
