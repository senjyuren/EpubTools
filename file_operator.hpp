#ifndef EPUB_FILE_OPERATOR_HPP
#define EPUB_FILE_OPERATOR_HPP

#include "kernel.hpp"
#include "strings.hpp"

#include <io.h>

#if defined(__WINDOWS_OS__)

#include <direct.h>
#include <corecrt_io.h>

#endif

namespace bw::file
{

namespace parameter
{

constexpr int CURRENT_DIRECTORY_RENAME_MAX_SIZE = 256;
constexpr int FILE_INFO_PATH_MAX_SIZE = 256;

typedef struct
{
    int size;
    char path[FILE_INFO_PATH_MAX_SIZE];
} FileInfo;

typedef struct
{
    int size;
    FileInfo *infos;
} DirectoryInfo;

constexpr char PARENT_DIRCTORY_SYMBOL[] = "..";
constexpr char CURRENT_DIRECTORY_SYMBOL[] = ".";
constexpr char CURRENT_DIRECTORY_RENAME_SUFFIX[] = "%s\\*.*";

#if defined(__WINDOWS_OS__)
constexpr char CURRENT_COMPLEX_FILE_AND_DIRECTORY[] = "%s\\%s";
#endif

}

using bw::strings::Strings;

class FileOperator
{
    using FileInfo = parameter::FileInfo;
    using DirectoryInfo = parameter::DirectoryInfo;

public:
    static FileOperator &Instance()
    {
        static FileOperator *obj = nullptr;
        if (obj == nullptr)
            obj = new FileOperator();
        return *obj;
    }

    FileOperator &Init()
    {
        if (this->mDirectoryInfo.infos != nullptr)
            delete[](this->mDirectoryInfo.infos);
        memset(this, 0, sizeof(FileOperator));
        return *this;
    }

    DirectoryInfo &FileListInFolder(const char *v)
    {
        if (v == nullptr)
            return this->mDirectoryInfo;

        memset(this->mRenameDirectory, 0, parameter::CURRENT_DIRECTORY_RENAME_MAX_SIZE);
        if (snprintf(
                this->mRenameDirectory,
                parameter::CURRENT_DIRECTORY_RENAME_MAX_SIZE,
                parameter::CURRENT_DIRECTORY_RENAME_SUFFIX,
                v
        ) <= 0)
            return this->mDirectoryInfo;

        this->mDirectoryInfo.size = this->SearchAllFile(nullptr, nullptr, 0);
        if ((this->mDirectoryInfo.infos = new FileInfo[this->mDirectoryInfo.size]) == nullptr)
        {
            this->mDirectoryInfo.size = 0;
            return this->mDirectoryInfo;
        }

        this->SearchAllFile(v, this->mDirectoryInfo.infos, this->mDirectoryInfo.size);
        return this->mDirectoryInfo;
    }

    bool Exist(const char *v)
    {
#if defined(__WINDOWS_OS__)
        auto ret = _access(v, 0);
        return (ret == 0 || ret == EACCES);
#endif
    }

    bool CreateFolder(const char *v)
    {
        if (v == nullptr)
            return false;
#if defined(__WINDOWS_OS__)
        return (_mkdir(v) == 0);
#endif
    }

    bool RemoveFolder(const char *v)
    {
        if (v == nullptr)
            return false;
#if defined(__WINDOWS_OS__)
        return (_rmdir(v) == 0);
#endif
    }

    bool RemoveFile(const char *v)
    {
        if (v == nullptr)
            return false;
        return (remove(v) == 0);
    }

private:
    char mRenameDirectory[parameter::CURRENT_DIRECTORY_RENAME_MAX_SIZE]{};
    DirectoryInfo mDirectoryInfo{};

    FileOperator() = default;

    int SearchAllFile(const char *path, FileInfo *files, int filesLen)
    {
        int step = 0;

        intptr_t opt = 0;
        _finddata_t fileData{};

        if ((opt = _findfirst(this->mRenameDirectory, &fileData)) == -1)
            return this->mDirectoryInfo.size;

        do
        {
            if (fileData.attrib == _A_SUBDIR)
                continue;
            if (Strings::Instance().Equals(fileData.name, parameter::PARENT_DIRCTORY_SYMBOL))
                continue;
            if (Strings::Instance().Equals(fileData.name, parameter::CURRENT_DIRECTORY_SYMBOL))
                continue;

            if ((files != nullptr) || (step < filesLen))
            {
                files[step].size = static_cast<int>(fileData.size);
                snprintf(
                        files[step].path,
                        parameter::CURRENT_DIRECTORY_RENAME_MAX_SIZE,
                        parameter::CURRENT_COMPLEX_FILE_AND_DIRECTORY,
                        path,
                        fileData.name
                );
            }

            ++step;
        } while (_findnext(opt, &fileData) == 0);

        _findclose(opt);
        return step;
    }
};

}

#endif //EPUB_FILE_OPERATOR_HPP
