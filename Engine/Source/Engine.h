// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

namespace An
{
    void InitWindow(int width, int height);
    bool ShouldWindowClose();
    void CloseWindow();

    float StartFrame();
    void EndFrame();
}