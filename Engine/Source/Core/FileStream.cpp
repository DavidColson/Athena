// Copyright 2020-2021 David Colson. All rights reserved.

#include "FileStream.h"

#include "Core/Log.h"

#include <SDL_rwops.h>

namespace An
{
    // ***********************************************************************

    FileStream::FileStream()
    {

    }

    // ***********************************************************************

    FileStream::FileStream(const FileStream & fileStream)
    {
        // Copy
        m_path = fileStream.m_path;
        m_modeString = fileStream.m_modeString;
        m_rwops = SDL_RWFromFile(m_path.c_str(), m_modeString.c_str());
    }

    // ***********************************************************************

    FileStream::FileStream(FileStream && fileStream)
    {
        // Move
        m_rwops = fileStream.m_rwops;
        m_path = fileStream.m_path;
        m_modeString = fileStream.m_modeString;

        fileStream.m_rwops = nullptr;
        fileStream.m_path = "";
        fileStream.m_modeString = "";
    }

    // ***********************************************************************

    FileStream::FileStream(eastl::string path, unsigned int mode)
    {
        m_modeString = "";

        if (mode & FileWrite)
        {
            m_modeString += "w";
            if (mode & FileRead)
                m_modeString += "+";
        }
        else if (mode & FileAppend)
        {
            m_modeString += "a";
            if (mode & FileRead)
                m_modeString += "+";
        }
        else if (mode & FileRead)
        {
            m_modeString += "r";
        }

        if (mode & FileBinary)
        {
            m_modeString += "b";
        }

        m_rwops = SDL_RWFromFile(path.c_str(), m_modeString.c_str());
        if (m_rwops == nullptr)
        {
            Log::Crit("Opening file stream for file %s failed with error: %s", path.c_str(), SDL_GetError());
        }

        m_path = path;
    }

    // ***********************************************************************

    FileStream::~FileStream()
    {
        Close();
    }

    // ***********************************************************************

    void FileStream::Close()
    {
        if (m_rwops == nullptr)
            return;
        
        SDL_RWclose(m_rwops);
        m_rwops = nullptr;
    }

    // ***********************************************************************

    eastl::string FileStream::Read(size_t length)
    {
        if (m_rwops == nullptr)
            return "";

        char* buffer = new char[length + 1];
        SDL_RWread(m_rwops, buffer, length, 1);
        buffer[length] = '\0';

        eastl::string result(buffer, buffer + length);
        delete[] buffer;
        return result;
    }

    // ***********************************************************************

    void FileStream::Read(char* buffer, size_t length)
    {
        if (m_rwops == nullptr)
            return;

        SDL_RWread(m_rwops, buffer, length, 1);
    }

    // ***********************************************************************

    void FileStream::Write(const char* buffer, size_t length)
    {
        if (m_rwops == nullptr)
            return;

        SDL_RWwrite(m_rwops, buffer, length, 1);
    }

    // ***********************************************************************

    size_t FileStream::Seek(size_t offset, SeekFrom from)
    {
        if (m_rwops == nullptr)
            return 0;

        if (from == SeekStart)
            return SDL_RWseek(m_rwops, offset, RW_SEEK_SET);
        else if(from == SeekEnd)
            return SDL_RWseek(m_rwops, offset, RW_SEEK_END);
        else
            return SDL_RWseek(m_rwops, offset, RW_SEEK_CUR);
    }

    // ***********************************************************************

    size_t FileStream::Tell()
    {
        if (m_rwops == nullptr)
            return 0;

        return SDL_RWtell(m_rwops);
    }

    // ***********************************************************************

    size_t FileStream::Size()
    {
        if (m_rwops == nullptr)
            return 0;

        return SDL_RWsize(m_rwops);
    }

    // ***********************************************************************

    bool FileStream::IsValid()
    {
        return m_rwops != nullptr;
    }

}