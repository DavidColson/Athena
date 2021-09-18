// Copyright 2020-2021 David Colson. All rights reserved.

#include "Text.h"

#include "Core/FileSystem.h"

namespace An
{
    void Text::Load(Path path, AssetHandle handleForThis)
    {
        contents = FileSys::ReadWholeFile(path);
    }
}