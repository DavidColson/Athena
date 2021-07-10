#include "Text.h"

#include "Core/FileSystem.h"

namespace An
{
    void Text::Load(Path path, AssetHandle handleForThis)
    {
        contents = FileSys::ReadWholeFile(path);
    }
}