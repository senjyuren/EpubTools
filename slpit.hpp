#ifndef EPUB_SLPIT_HPP
#define EPUB_SLPIT_HPP

#include "kernel.hpp"
#include "io.hpp"
#include "strings.hpp"
#include "file_operator.hpp"
#include "display.hpp"

namespace bw::slpit
{

using bw::io::IO;
using bw::io::parameter::IOStorage;
using bw::strings::Strings;
using bw::file::FileOperator;
using bw::display::Display;

namespace parameter
{

constexpr int COMPLEX_FILE_PATH_MAX_SIZE = 4096;

#if defined(__WINDOWS_OS__)
constexpr char COMPLEX_FILE_PATH_LINK_SYMBOL[] = "%s\\%s.%s";
#else
constexpr char COMPLEX_FILE_PATH_LINK_SYMBOL[] = "%s/%s.%s";
#endif

constexpr int SPLIT_FILE_DEFAULT_NAME_MAX_SIZE = 256;
constexpr char SPLIT_FILE_DEFAULT_NAME_LINK_SYMBOL[] = "%s_%d";
constexpr char SPLIT_FILE_DEFAULT_NAME[] = "text";
constexpr char SPLIT_FILE_DEFAULT_SUFFIX[] = "txt";

constexpr char FILE_OPERATOR_MODEL[] = "wb";
constexpr char WRAP_SYMBOL[] = "\r\n";

}

class Slpit
{
public:
    static Slpit &Instance()
    {
        static Slpit *obj = nullptr;
        if (obj == nullptr)
            obj = new Slpit();
        return *obj;
    }

    Slpit &Process()
    {
        Display::Instance().Clean();

        if (!(this->isOk = this->CheckIsOk()))
            return *this;

        this->mFileAllRow = IO::Instance().Init()
                .SetFilePath(this->mInFilePath)
                .Process()
                .GetIOStorage();
        if (this->mFileAllRow.rowsLen == 0)
        {
            this->isOk = false;
            return *this;
        }

        this->isOk = this->SplitFileByMark();
        Display::Instance().Clean();
        return *this;
    }

    Slpit &SetSlpitSymbol(const char *v)
    {
        this->mSlpitSymbol = (strlen(v) == 0 ? nullptr : v);
        return *this;
    }

    Slpit &SetInFilePath(const char *v)
    {
        this->mInFilePath = (strlen(v) == 0 ? nullptr : v);
        return *this;
    }

    Slpit &SetOutDirPath(const char *v)
    {
        this->mOutDirPath = (strlen(v) == 0 ? nullptr : v);
        return *this;
    }

    Slpit &SetFilesName(const char *v)
    {
        this->mSplitFileName = (strlen(v) == 0 ? nullptr : v);
        return *this;
    }

    Slpit &SetFilesSuffix(const char *v)
    {
        this->mSplitFileSuffix = (strlen(v) == 0 ? nullptr : v);
        return *this;
    }

    bool IsOk()
    {
        return this->isOk;
    }

private:
    bool isOk = false;

    const char *mSplitFileName = nullptr;
    const char *mSplitFileSuffix = nullptr;
    const char *mSlpitSymbol = nullptr;
    const char *mInFilePath = nullptr;
    const char *mOutDirPath = nullptr;

    char mFileName[parameter::SPLIT_FILE_DEFAULT_NAME_MAX_SIZE]{};
    char mComplexPath[parameter::COMPLEX_FILE_PATH_MAX_SIZE]{};
    IOStorage mFileAllRow{};

    Slpit() = default;

    bool CheckIsOk()
    {
        if (this->mSplitFileName == nullptr)
            this->mSplitFileName = parameter::SPLIT_FILE_DEFAULT_NAME;
        if (this->mSplitFileSuffix == nullptr)
            this->mSplitFileSuffix = parameter::SPLIT_FILE_DEFAULT_SUFFIX;
        return (
                (this->mSlpitSymbol != nullptr) && (this->mInFilePath != nullptr) && (this->mOutDirPath != nullptr)
        );
    }

    int FusionComplexPath(int number)
    {
        int size = 0;

        if ((size = snprintf(
                this->mFileName,
                parameter::SPLIT_FILE_DEFAULT_NAME_MAX_SIZE,
                parameter::SPLIT_FILE_DEFAULT_NAME_LINK_SYMBOL,
                this->mSplitFileName, number
        )) < 0)
            return size;

        if ((size = snprintf(
                this->mComplexPath,
                parameter::COMPLEX_FILE_PATH_MAX_SIZE,
                parameter::COMPLEX_FILE_PATH_LINK_SYMBOL,
                this->mOutDirPath,
                this->mFileName,
                this->mSplitFileSuffix
        )) < 0)
            return size;

        return size;
    }

    bool SplitFileByMark()
    {
        int i = 0;
        int nameMarkNumber = 0;
        bool ret = false;
        FILE *outDir = nullptr;

        if (!FileOperator::Instance().Exist(this->mOutDirPath))
        {
            if (!FileOperator::Instance().CreateFolder(this->mOutDirPath))
                goto EXIT;
        }

        this->FusionComplexPath(++nameMarkNumber);
        if ((outDir = fopen(this->mComplexPath, parameter::FILE_OPERATOR_MODEL)) == nullptr)
            goto EXIT;

        for (i = 0; i < this->mFileAllRow.rowsLen; ++i)
        {
            Display::Instance().Show<Display::INFO>("Processing to number %d file", nameMarkNumber);

            memset(this->mFileName, 0, parameter::SPLIT_FILE_DEFAULT_NAME_MAX_SIZE);
            memset(this->mComplexPath, 0, parameter::COMPLEX_FILE_PATH_MAX_SIZE);

            if (!this->mFileAllRow.storage[i].effect)
                continue;
            if (Strings::Instance().TrimSpace(
                    this->mFileAllRow.storage[i].row, this->mFileAllRow.storage[i].rowMaxLen
            ) < 0)
                continue;
            if (Strings::Instance().Equals(this->mFileAllRow.storage[i].row, this->mSlpitSymbol))
            {
                fclose(outDir);
                this->FusionComplexPath(++nameMarkNumber);
                if ((outDir = fopen(this->mComplexPath, parameter::FILE_OPERATOR_MODEL)) == nullptr)
                    goto EXIT;
            } else
            {
                fwrite(
                        this->mFileAllRow.storage[i].row,
                        1,
                        static_cast<size_t>(this->mFileAllRow.storage[i].rowLen),
                        outDir
                );
                fwrite(parameter::WRAP_SYMBOL, 1, strlen(parameter::WRAP_SYMBOL), outDir);
            }
        }

        ret = true;
        EXIT:
        if (outDir != nullptr)
            fclose(outDir);
        return ret;
    }
};

}

#endif //EPUB_SLPIT_HPP
