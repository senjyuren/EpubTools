#ifndef EPUB_XHTML_HPP
#define EPUB_XHTML_HPP

#include "kernel.hpp"
#include "io.hpp"
#include "strings.hpp"
#include "file_operator.hpp"
#include "display.hpp"

namespace bw::xhtml
{

namespace parameter
{

constexpr int RENAME_FILE_PATH_MAX_SIZE = 256;
constexpr int REWRITE_ROW_MAX_SIZE = 10000;

constexpr char FILE_SUFFIX_MARK[] = ".";
constexpr char FILE_MOD[] = "wb";
constexpr char FILE_SUFFIX[] = "xhtml";

constexpr char TITLE_NO_1_MARK_SYMBOL[] = "##h1##";
constexpr char TITLE_NO_2_MARK_SYMBOL[] = "##h2##";
constexpr char TITLE_NO_3_MARK_SYMBOL[] = "##h3##";
constexpr char TITLE_NO_4_MARK_SYMBOL[] = "##h4##";
constexpr char TITLE_NO_5_MARK_SYMBOL[] = "##h5##";
constexpr char TITLE_NO_6_MARK_SYMBOL[] = "##h6##";

constexpr char TITLE_NO1_FIRST_SYMBOL[] = "<h1>";
constexpr char TITLE_NO2_FIRST_SYMBOL[] = "<h2>";
constexpr char TITLE_NO3_FIRST_SYMBOL[] = "<h3>";
constexpr char TITLE_NO4_FIRST_SYMBOL[] = "<h4>";
constexpr char TITLE_NO5_FIRST_SYMBOL[] = "<h5>";
constexpr char TITLE_NO6_FIRST_SYMBOL[] = "<h6>";
constexpr char ROW_FIRST_SYMBOL[] = "<p>";

constexpr char TITLE_NO1_LAST_SYMBOL[] = "</h1>";
constexpr char TITLE_NO2_LAST_SYMBOL[] = "</h2>";
constexpr char TITLE_NO3_LAST_SYMBOL[] = "</h3>";
constexpr char TITLE_NO4_LAST_SYMBOL[] = "</h4>";
constexpr char TITLE_NO5_LAST_SYMBOL[] = "</h5>";
constexpr char TITLE_NO6_LAST_SYMBOL[] = "</h6>";
constexpr char ROW_LAST_SYMBOL[] = "</p>";

constexpr char WRAP_SYMBOL[] = "\r\n";
constexpr char XHTML_HEAD[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
                              "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\r\n"
                              "  \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\r\n"
                              "\r\n"
                              "<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n"
                              "<head>\r\n"
                              "<title></title>\r\n"
                              "</head>\r\n"
                              "<body>\r\n";

constexpr static char XHTML_FOOT[] = "</body>\r\n"
                                     "</html>";

}

using bw::io::IO;
using bw::io::parameter::IOStorage;
using bw::file::FileOperator;
using bw::file::parameter::DirectoryInfo;
using bw::strings::Strings;
using bw::display::Display;

enum class XhtmlMod
{
    DIR, FILE,
};

class Xhtml
{
public:
    static Xhtml &Instance()
    {
        static Xhtml *obj = nullptr;
        if (obj == nullptr)
            obj = new Xhtml();
        return *obj;
    }

    Xhtml &Process()
    {
        Display::Instance().Clean();

        if (!(this->isOk = this->CheckParameter()))
            return *this;

        if (this->mXhtmlMod == XhtmlMod::DIR)
        {
            if (!(this->isOk = this->DirProcess()))
                return *this;
        } else if (this->mXhtmlMod == XhtmlMod::FILE)
        {
            if (!(this->isOk = this->FileProcess()))
                return *this;
        }

        Display::Instance().Clean();
        return *this;
    }

    Xhtml &SetNameSpace(const char *v)
    {
        auto vLen = static_cast<int>(strlen(v));
        if (vLen == 0)
            return *this;
        if (vLen == 5)
            this->mNameSpace = false;
        else if (vLen == 4)
            this->mNameSpace = true;
        return *this;
    }

    Xhtml &SetOutDirPath(const char *v)
    {
        this->mOutDirPath = (strlen(v) == 0 ? nullptr : v);
        return *this;
    }

    Xhtml &SetXhtmlMod(XhtmlMod mod)
    {
        this->mXhtmlMod = mod;
        return *this;
    }

    bool IsOk()
    {
        return this->isOk;
    }

private:
    bool isOk = false;
    bool mNameSpace = false;

    const char *mOutDirPath = nullptr;
    char mRewriteRow[parameter::REWRITE_ROW_MAX_SIZE]{};
    char mRenameFilePathCache[parameter::RENAME_FILE_PATH_MAX_SIZE]{};

    IOStorage mIOStorage{};
    DirectoryInfo mDirInfo{};
    XhtmlMod mXhtmlMod = XhtmlMod::DIR;

    Xhtml() = default;

    bool DirProcess()
    {
        int i = 0;
        int j = 0;
        int newLen = 0;
        FILE *newFile = nullptr;

        auto wrapSymbolLen = strlen(parameter::WRAP_SYMBOL);
        this->mDirInfo = FileOperator::Instance().Init().FileListInFolder(this->mOutDirPath);

        for (i = 0; i < this->mDirInfo.size; ++i)
        {
            Display::Instance().Show<Display::INFO>("Processing number %d to convert xhtml", i);

            if (this->RenameFilePath(
                    this->mDirInfo.infos[i].path,
                    parameter::FILE_SUFFIX,
                    this->mRenameFilePathCache,
                    parameter::RENAME_FILE_PATH_MAX_SIZE
            ) == 0)
                return false;

            this->mIOStorage = IO::Instance().Init()
                    .SetFilePath(this->mDirInfo.infos[i].path)
                    .Process()
                    .GetIOStorage();

            if ((newFile = fopen(this->mRenameFilePathCache, parameter::FILE_MOD)) == nullptr)
                return false;

            if (this->mNameSpace)
                fwrite(parameter::XHTML_HEAD, 1, strlen(parameter::XHTML_HEAD), newFile);

            for (j = 0; j < this->mIOStorage.rowsLen; ++j)
            {
                if (!this->mIOStorage.storage[j].effect)
                    continue;

                if (Strings::Instance().Equals(this->mIOStorage.storage[j].row, parameter::TITLE_NO_1_MARK_SYMBOL))
                    newLen = this->RewriteRow(
                            this->mIOStorage.storage[++j].row,
                            parameter::TITLE_NO1_FIRST_SYMBOL,
                            parameter::TITLE_NO1_LAST_SYMBOL,
                            this->mRewriteRow,
                            parameter::REWRITE_ROW_MAX_SIZE
                    );
                else if (Strings::Instance().Equals(this->mIOStorage.storage[j].row, parameter::TITLE_NO_2_MARK_SYMBOL))
                    newLen = this->RewriteRow(
                            this->mIOStorage.storage[++j].row,
                            parameter::TITLE_NO2_FIRST_SYMBOL,
                            parameter::TITLE_NO2_LAST_SYMBOL,
                            this->mRewriteRow,
                            parameter::REWRITE_ROW_MAX_SIZE
                    );
                else if (Strings::Instance().Equals(this->mIOStorage.storage[j].row, parameter::TITLE_NO_3_MARK_SYMBOL))
                    newLen = this->RewriteRow(
                            this->mIOStorage.storage[++j].row,
                            parameter::TITLE_NO3_FIRST_SYMBOL,
                            parameter::TITLE_NO3_LAST_SYMBOL,
                            this->mRewriteRow,
                            parameter::REWRITE_ROW_MAX_SIZE
                    );
                else if (Strings::Instance().Equals(this->mIOStorage.storage[j].row, parameter::TITLE_NO_4_MARK_SYMBOL))
                    newLen = this->RewriteRow(
                            this->mIOStorage.storage[++j].row,
                            parameter::TITLE_NO4_FIRST_SYMBOL,
                            parameter::TITLE_NO4_LAST_SYMBOL,
                            this->mRewriteRow,
                            parameter::REWRITE_ROW_MAX_SIZE
                    );
                else if (Strings::Instance().Equals(this->mIOStorage.storage[j].row, parameter::TITLE_NO_5_MARK_SYMBOL))
                    newLen = this->RewriteRow(
                            this->mIOStorage.storage[++j].row,
                            parameter::TITLE_NO5_FIRST_SYMBOL,
                            parameter::TITLE_NO5_LAST_SYMBOL,
                            this->mRewriteRow,
                            parameter::REWRITE_ROW_MAX_SIZE
                    );
                else if (Strings::Instance().Equals(this->mIOStorage.storage[j].row, parameter::TITLE_NO_6_MARK_SYMBOL))
                    newLen = this->RewriteRow(
                            this->mIOStorage.storage[++j].row,
                            parameter::TITLE_NO6_FIRST_SYMBOL,
                            parameter::TITLE_NO6_LAST_SYMBOL,
                            this->mRewriteRow,
                            parameter::REWRITE_ROW_MAX_SIZE
                    );
                else
                    newLen = this->RewriteRow(
                            this->mIOStorage.storage[j].row,
                            parameter::ROW_FIRST_SYMBOL,
                            parameter::ROW_LAST_SYMBOL,
                            this->mRewriteRow,
                            parameter::REWRITE_ROW_MAX_SIZE
                    );


                fwrite(this->mRewriteRow, 1, static_cast<size_t>(newLen), newFile);
                fwrite(parameter::WRAP_SYMBOL, 1, static_cast<size_t>(wrapSymbolLen), newFile);
            }

            if (this->mNameSpace)
                fwrite(parameter::XHTML_FOOT, 1, strlen(parameter::XHTML_FOOT), newFile);

            fclose(newFile);
            FileOperator::Instance().RemoveFile(this->mDirInfo.infos[i].path);
        }

        return true;
    }

    bool FileProcess()
    {
        return false;
    }

    int RewriteRow(const char *row, const char *symbolFirst, const char *symbolLast, char *v, int vLen)
    {
        int step = 0;
        int length = 0;

        if ((row == nullptr) || (symbolFirst == nullptr) || (symbolLast == nullptr) || (v == nullptr) || (vLen <= 0))
            return 0;

        memset(v, 0, static_cast<size_t>(vLen));
        length = static_cast<int>(strlen(symbolFirst));
        memcpy(&v[step], symbolFirst, static_cast<size_t>(length));
        if ((step += length) > vLen)
            return 0;

        length = static_cast<int>(strlen(row));
        memcpy(&v[step], row, static_cast<size_t>(length));
        if ((step += length) > vLen)
            return 0;

        length = static_cast<int>(strlen(symbolLast));
        memcpy(&v[step], symbolLast, static_cast<size_t>(length));
        return (step + length);
    }

    int RenameFilePath(const char *path, const char *newSuffix, char *newPath, int newPathLen)
    {
        int i = 0;
        int ret = 0;
        int suffix = 0;

        auto pathLen = static_cast<int>(strlen(path));
        auto newSuffixLen = static_cast<int>(strlen(newSuffix));
        if ((path == nullptr) || (newSuffix == nullptr) || (newPath == nullptr) || (newPathLen <= 0))
            return 0;

        for (i = 0; i < pathLen; ++i)
        {
            if (path[i] == parameter::FILE_SUFFIX_MARK[0])
                suffix = i;
        }

        ++suffix;
        ret = static_cast<int>(&path[suffix] - &path[0]);
        memset(newPath, 0, static_cast<size_t>(newPathLen));
        memcpy(newPath, path, static_cast<size_t>(ret));
        if ((ret + newSuffixLen) >= newPathLen)
            return 0;
        memcpy(&newPath[ret], newSuffix, static_cast<size_t>(newSuffixLen));
        return (ret + newSuffixLen);
    }

    bool CheckParameter()
    {
        return this->mOutDirPath != nullptr;
    }
};

}

#endif //EPUB_XHTML_HPP
