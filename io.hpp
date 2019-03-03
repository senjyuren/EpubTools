#ifndef EPUB_IO_HPP
#define EPUB_IO_HPP

namespace bw::io
{

namespace parameter
{

constexpr int READER_MAX_SIZE = 256;
constexpr int ROW_MAX_SIZE = 10000;

typedef struct
{
    int rowMaxLen;
    int rowLen;
    char row[ROW_MAX_SIZE];
    bool effect;
} IORowStorage;

typedef struct
{
    int rowsLen;
    IORowStorage *storage;
} IOStorage;

constexpr char IO_READ_MODE[] = "rb";
constexpr char TEXT_WINDOWS_STYLE_WRAP_SYMBOL[] = "\r\n";

}

class IO
{
    using IOStorage = parameter::IOStorage;
    using IORowStorage = parameter::IORowStorage;

    using ReadFn = void (*)(int id, char v);

public:
    static IO &Instance()
    {
        static IO *obj = nullptr;
        if (obj == nullptr)
            obj = new IO();
        return *obj;
    }

    IO &Init()
    {
        if (this->mIOStorage.storage != nullptr)
            delete[](this->mIOStorage.storage);

        memset(this, 0, sizeof(IO));
        return *this;
    }

    IO &Process()
    {
        int i = 0;

        if (!this->isOk)
            return *this;
        if (!this->CalculationRowsSize())
            return *this;

        if ((this->mIOStorage.storage = new IORowStorage[this->mIOStorage.rowsLen]) == nullptr)
        {
            this->mIOStorage.rowsLen = 0;
            return *this;
        }

        this->ReadAllRow();
        for (i = 0; i < this->mIOStorage.rowsLen; ++i)
        {
            this->mIOStorage.storage[i].rowMaxLen = parameter::ROW_MAX_SIZE;
            this->mIOStorage.storage[i].rowLen = static_cast<int>(strlen(this->mIOStorage.storage[i].row));
            if (this->mIOStorage.storage[i].rowLen <= 0)
                continue;
            this->mIOStorage.storage[i].effect = true;
        }

        if (this->mFile != nullptr)
            fclose(this->mFile);
        this->mFile = nullptr;
        return *this;
    }

    IO &SetFilePath(const char *v)
    {
        if (v == nullptr)
            return *this;
        if ((this->mFile = fopen(v, parameter::IO_READ_MODE)) == nullptr)
            return *this;
        this->isOk = true;
        return *this;
    }

    IOStorage &GetIOStorage()
    {
        return this->mIOStorage;
    }

private:
    bool isOk = false;

    char mReader[parameter::READER_MAX_SIZE]{};

    FILE *mFile = nullptr;
    IOStorage mIOStorage{};

    void ReadAllRow()
    {
        int i = 0;
        int readStep = 0;
        int retLen = 0;
        bool carryOn = false;
        bool hypothesis = false;

        this->mIOStorage.rowsLen = 0;
        fseek(this->mFile, 0, SEEK_SET);

        while (true)
        {
            retLen = static_cast<int>(fread(this->mReader, 1, parameter::READER_MAX_SIZE, this->mFile));
            if (retLen <= 0)
                break;
            if (retLen < parameter::READER_MAX_SIZE)
                carryOn = true;

            for (i = 0; i < retLen; ++i)
            {
                if (this->mReader[i] == parameter::TEXT_WINDOWS_STYLE_WRAP_SYMBOL[0])
                {
                    ++this->mIOStorage.rowsLen;
                    readStep = 0;
                    hypothesis = true;
                    continue;
                } else
                {
                    if (hypothesis)
                    {
                        hypothesis = false;
                        continue;
                    }

                    if (this->mReader[i] == parameter::TEXT_WINDOWS_STYLE_WRAP_SYMBOL[1])
                    {
                        ++this->mIOStorage.rowsLen;
                        readStep = 0;
                        continue;
                    }

                    if (readStep >= parameter::ROW_MAX_SIZE)
                        continue;
                    this->mIOStorage.storage[this->mIOStorage.rowsLen].row[readStep] = this->mReader[i];
                    ++readStep;
                }
            }

            if (carryOn)
                break;
        }
    }

    int CalculationRowsSize()
    {
        int i = 0;
        int retLen = 0;
        bool carryOn = false;
        bool hypothesis = false;

        fseek(this->mFile, 0, SEEK_SET);

        while (true)
        {
            retLen = static_cast<int>(fread(this->mReader, 1, parameter::READER_MAX_SIZE, this->mFile));
            if (retLen <= 0)
                break;
            if (retLen < parameter::READER_MAX_SIZE)
                carryOn = true;

            for (i = 0; i < retLen; ++i)
            {
                if (this->mReader[i] == parameter::TEXT_WINDOWS_STYLE_WRAP_SYMBOL[0])
                {
                    ++this->mIOStorage.rowsLen;
                    hypothesis = true;
                    continue;
                } else
                {
                    if (hypothesis)
                    {
                        hypothesis = false;
                        continue;
                    }

                    if (this->mReader[i] == parameter::TEXT_WINDOWS_STYLE_WRAP_SYMBOL[1])
                    {
                        ++this->mIOStorage.rowsLen;
                        continue;
                    }
                }
            }

            if (carryOn)
                break;
        }

        return (++this->mIOStorage.rowsLen);
    }
};

}

#endif //EPUB_IO_HPP
