// Copyright 2020-2021 David Colson. All rights reserved.

#include "Path.h"

#include <EASTL/algorithm.h>

namespace An
{
    // ***********************************************************************

    Path::Path()
    {
        m_stringPath = "";
    }

    // ***********************************************************************

    Path::Path(eastl::string pathAsString)
    {
        m_stringPath = pathAsString;
        Analyze();
    }

    // ***********************************************************************

    Path::Path(const char* pathAsString)
    {
        m_stringPath = pathAsString;
        Analyze();
    }

    // ***********************************************************************

    Path::Path(const Path& path)
    {
        m_stringPath = path.m_stringPath;
        m_pathParts = path.m_pathParts;
        m_hasValidRootname = path.m_hasValidRootname;
    }

    // ***********************************************************************

    Path::Path(Path&& path)
    {
        m_stringPath = eastl::move(path.m_stringPath);
        m_pathParts = eastl::move(path.m_pathParts);
        m_hasValidRootname = eastl::move(path.m_hasValidRootname);
    }

    // ***********************************************************************

    Path& Path::operator=(const Path& path)
    {
        m_stringPath = path.m_stringPath;
        m_pathParts = path.m_pathParts;
        m_hasValidRootname = path.m_hasValidRootname;
        return *this;
    }

    // ***********************************************************************

    Path& Path::operator=(Path&& path)
    {
        m_stringPath = eastl::move(path.m_stringPath);
        m_pathParts = eastl::move(path.m_pathParts);
        m_hasValidRootname = eastl::move(path.m_hasValidRootname);
        return *this;
    }

    // ***********************************************************************

    Path Path::RootName() const
    {
        if (m_hasValidRootname)
            return m_pathParts[0];;
        return "";
    }

    // ***********************************************************************

    Path Path::RootDirectory() const
    {
        if (RootName() == "")
        {
            if ((m_pathParts[0] == "/" || m_pathParts[0] == "\\"))
                return m_pathParts[0];
            else
                return "";
        }
        return m_pathParts[1];
    }

    // ***********************************************************************

    Path Path::RootPath() const
    {
        return RootName() + RootDirectory();
    }

    // ***********************************************************************

    Path Path::RelativePath() const
    {
        eastl::string relativePath = m_stringPath;
        eastl::string rootPath = RootPath().AsString();
        // remove rootPath
        size_t pos = 0;
        if ((pos = relativePath.find(rootPath)) != eastl::string::npos)
        {
            relativePath.erase(0, pos + rootPath.length());
        }
        return relativePath;
    }

    // ***********************************************************************

    Path Path::ParentPath() const
    {
        eastl::string parentPath;
        for (size_t i = 0; i < eastl::max((int)m_pathParts.size()-2, 0); i++)
        {
            parentPath += m_pathParts[i];
        }
        return parentPath;
    }

    // ***********************************************************************

    Path Path::Filename() const
    {
        size_t numParts = m_pathParts.size();
        return (numParts > 0) ? m_pathParts[numParts-1] : "";
    }

    // ***********************************************************************

    Path Path::Stem() const
    {
        eastl::string filename = Filename().AsString();
        size_t pos = filename.find_first_of('.', 1);

        if (filename == "." || filename == ".." || pos == eastl::string::npos)
        {
            return filename;
        }
        else
        {
            return filename.substr(0, pos);
        }

    }

    // ***********************************************************************

    Path Path::Extension() const
    {
        eastl::string filename = Filename().AsString();
        size_t pos = filename.find_first_of('.', 1);

        if (filename == "." || filename == ".." || pos == eastl::string::npos)
        {
            return "";
        }
        else
        {
            return filename.substr(pos);
        }
    }

    // ***********************************************************************

    Path Path::RemoveTrailingSlash() const
    {
        char c = m_stringPath[m_stringPath.length() - 1];
        if (c == '/' || c == '\\')
            return m_stringPath.substr(0, m_stringPath.length() - 1);
        return m_stringPath;
    }

    // ***********************************************************************

    bool Path::IsEmpty() const
    {
        return m_stringPath.empty();
    }

    // ***********************************************************************

    bool Path::HasRootPath() const
    {
        return !RootPath().IsEmpty();
    }

    // ***********************************************************************

    bool Path::HasRootName() const
    {
        return m_hasValidRootname;
    }

    // ***********************************************************************

    bool Path::HasRootDirectory() const
    {
        return !RootDirectory().IsEmpty();
    }

    // ***********************************************************************

    bool Path::HasRelativePath() const
    {
        return !RelativePath().IsEmpty();
    }

    // ***********************************************************************

    bool Path::HasParentPath() const
    {
        return !ParentPath().IsEmpty();
    }

    // ***********************************************************************

    bool Path::HasFilename() const
    {
        return !Filename().IsEmpty();
    }

    // ***********************************************************************

    bool Path::HasStem() const
    {
        return !Stem().IsEmpty();
    }

    // ***********************************************************************

    bool Path::HasExtension() const
    {
        return !Extension().IsEmpty();
    }

    // ***********************************************************************

    bool Path::IsAbsolute() const
    {
        return m_hasValidRootname;
    }

    // ***********************************************************************

    bool Path::IsRelative() const
    {
        return !m_hasValidRootname;
    }

    // ***********************************************************************

    eastl::string Path::AsString() const
    {
        return m_stringPath;
    }

    // ***********************************************************************

    const char* Path::AsRawString() const
    {
        return m_stringPath.c_str();
    }

    // ***********************************************************************

    Path& Path::operator/=(const Path& pathToAppend)
    {
        if (IsEmpty())
        {
            m_stringPath = pathToAppend.m_stringPath;
            Analyze();
            return *this;   
        }

        if (pathToAppend.IsAbsolute())
        {
            m_stringPath = pathToAppend.m_stringPath;
            Analyze();
            return *this;
        }

        if (m_stringPath[m_stringPath.size() - 1] == '/' || m_stringPath[m_stringPath.size() - 1] == '\\')
            m_stringPath += pathToAppend.m_stringPath;
        else
            m_stringPath += m_preferredSeparator + pathToAppend.m_stringPath;
        Analyze();
        return *this;
    }
        
    // ***********************************************************************

    Path& Path::operator+=(const Path& pathToConcat)
    {
        m_stringPath += pathToConcat.m_stringPath;
        Analyze();
        return *this;
    }

    // ***********************************************************************

    size_t FindNextSeparatorLoc(const eastl::string& path, size_t startPos)
    {
        size_t nextForward = path.find('/', startPos);
        size_t nextBackward = path.find('\\', startPos);

        return nextForward < nextBackward ? nextForward : nextBackward;
    } 

    // ***********************************************************************

    void Path::Analyze()
    {
        if (IsEmpty())
            return;

        m_pathParts.clear();
        // Split path into parts
        size_t pos1 = 0;
        size_t pos2 = 0;
        eastl::string token;

        // Parse rootname
        if (m_stringPath[1] == ':')
        {
            m_pathParts.push_back(m_stringPath.substr(0, 2));
            pos1 = 2; pos2 = 2; m_hasValidRootname = true;
        }
        else if (m_stringPath.substr(0, 2) == "\\\\" || m_stringPath.substr(0, 2) == "//")
        {
            size_t rootEnd = FindNextSeparatorLoc(m_stringPath, 2);
            m_pathParts.push_back(m_stringPath.substr(0, rootEnd));
            pos1 = rootEnd; pos2 = rootEnd; m_hasValidRootname = true;
        }
        else if (m_stringPath.substr(0, 3) == "\\\\." || m_stringPath.substr(0, 3) == "\\\\?" || m_stringPath.substr(0, 3) == "//?" || m_stringPath.substr(0, 3) == "//.")
        {
            m_pathParts.push_back(m_stringPath.substr(0, 3));
            pos1 = 3; pos2 = 3; m_hasValidRootname = true;
        }

        while((pos2 = FindNextSeparatorLoc(m_stringPath, pos1)) != eastl::string::npos)
        {
            token = m_stringPath.substr(pos1, pos2-pos1);
            if (!token.empty())
                m_pathParts.push_back(token);
            m_pathParts.push_back(m_stringPath.substr(pos2, 1)); // separator
            pos1 = pos2+1;
        }
        
        // Final token
        if (pos1 != eastl::string::npos)
        {
            token = m_stringPath.substr(pos1, m_stringPath.length() - pos1);
            if (!token.empty())
                m_pathParts.push_back(m_stringPath.substr(pos1, m_stringPath.length() - pos1));
        }
    }

    // ***********************************************************************

    const Path::PathIterator Path::begin() const
    {
        return PathIterator(m_pathParts.begin());
    }

    // ***********************************************************************

    const Path::PathIterator Path::end() const
    {
        return PathIterator(m_pathParts.end());
    }

    // ***********************************************************************

    Path Path::PathIterator::operator*() const 
    { 
        return Path(*m_pathPartsIter);
    }

    // ***********************************************************************

    bool Path::PathIterator::operator==(const PathIterator& other) const 
    {
        return m_pathPartsIter == other.m_pathPartsIter;
    }

    // ***********************************************************************

    bool Path::PathIterator::operator!=(const PathIterator& other) const 
    {
        return m_pathPartsIter != other.m_pathPartsIter;
    }

    // ***********************************************************************

    Path::PathIterator& Path::PathIterator::operator++()
    {
        if (Path(*m_pathPartsIter).m_hasValidRootname)
        {
            ++m_pathPartsIter;
            return *this;
        }
        
        ++m_pathPartsIter;
        if (*m_pathPartsIter == "\\" || *m_pathPartsIter == "/")
            ++m_pathPartsIter;
        return *this;
    }

    // ***********************************************************************

    Path operator/(const Path& lhs, const Path& rhs)
    {
        Path newPath = lhs;
        newPath /= rhs;
        return newPath;
    }

    // ***********************************************************************

    Path operator+(const Path& lhs, const Path& rhs)
    {
        Path newPath = lhs;
        newPath += rhs;
        return newPath;
    }

    // ***********************************************************************

    bool operator==(const Path& lhs, const Path& rhs)
    {
        return lhs.AsString() == rhs.AsString();
    }

    // ***********************************************************************

    bool operator!=(const Path& lhs, const Path& rhs)
    {
        return lhs.AsString() != rhs.AsString();
    }

}