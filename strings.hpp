#ifndef EPUB_STRINGS_HPP
#define EPUB_STRINGS_HPP

#include "kernel.hpp"

namespace bw::strings
{

namespace parameter
{

constexpr int TRIM_CACHE_SIZE = 40960;
constexpr char TRIM_SPACE_SYMBOL = 0x20;

}

class Strings
{
public:
    static Strings &Instance()
    {
        static Strings *obj = nullptr;
        if (obj == nullptr)
            obj = new Strings();
        return *obj;
    }

    bool Equals(const char *a, const char *b)
    {
        int i = 0;
        int aLen = 0;
        int bLen = 0;

        if ((a == nullptr) || (b == nullptr))
            return false;

        aLen = static_cast<int>(strlen(a));
        bLen = static_cast<int>(strlen(b));
        if (aLen != bLen)
            return false;

        for (i = 0; i < aLen; ++i)
        {
            if (a[i] != b[i])
                return false;
        }

        return true;
    }

    int TrimSpace(char *v, int vLen)
    {
        int i = 0;
        int ret = 0;
        int startIdx = 0;
        int endIdx = 0;

        auto characterSize = static_cast<int>(strlen(v));
        if ((v == nullptr) || (vLen <= 0))
            return characterSize;

        for (i = 0; i < characterSize; ++i)
        {
            if (v[i] == parameter::TRIM_SPACE_SYMBOL)
                ++startIdx;
            else
                break;
        }

        endIdx = characterSize;
        for (i = characterSize; i >= 0; ++i)
        {
            if (v[i] == parameter::TRIM_SPACE_SYMBOL)
                --endIdx;
            else
                break;
        }

        ret = static_cast<int>(&v[endIdx] - &v[startIdx]);
        memset(this->mTrimCahce, 0, parameter::TRIM_CACHE_SIZE);
        memcpy(this->mTrimCahce, &v[startIdx], static_cast<size_t>(ret));
        memset(v, 0, static_cast<size_t>(vLen));
        memcpy(v, this->mTrimCahce, static_cast<size_t>(ret));
        return ret;
    }

private:
    char mTrimCahce[parameter::TRIM_CACHE_SIZE]{};

    Strings() = default;
};

}

#endif //EPUB_STRINGS_HPP
