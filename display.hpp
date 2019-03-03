#ifndef EPUB_DISPLAY_HPP
#define EPUB_DISPLAY_HPP

#include "kernel.hpp"

namespace bw::display
{

namespace parameter
{

constexpr int CHARACTER_CACHE_MAX_SIZE = 4096;

#if defined(__WINDOWS_OS__)
constexpr char CLEAN_TERMINAL_COMMAND[] = "cls";
#else
constexpr char CLEAN_TERMINAL_COMMAND[] = "clear";
#endif

}

enum class DisplayMod
{
    LOG, CMD, NOT_SHOW,
};

class Display
{
public:
    typedef enum
    {
        INFO, DEBUG, ERROR,
    } LogMod;

    static Display &Instance()
    {
        static Display *obj = nullptr;
        if (obj == nullptr)
            obj = new Display();
        return *obj;
    }

    template<LogMod _mod, typename... Args>
    Display &Show(const char *v, Args...args)
    {
        if (this->mMod == DisplayMod::NOT_SHOW)
            return *this;
        if (v == nullptr)
            return *this;
        if (snprintf(this->mCache, parameter::CHARACTER_CACHE_MAX_SIZE, v, args...) <= 0)
            return *this;
        if (this->mMod == DisplayMod::CMD)
            this->DisplayOutput(this->mCache);
        else if (this->mMod == DisplayMod::LOG)
            this->LogOutput<_mod>(this->mCache);
        return *this;
    }

    Display &Clean()
    {
        if (this->mMod == DisplayMod::CMD)
            system(parameter::CLEAN_TERMINAL_COMMAND);
        return *this;
    }

private:
    char mCache[parameter::CHARACTER_CACHE_MAX_SIZE]{};
    DisplayMod mMod = DisplayMod::CMD;

    void DisplayOutput(const char *v)
    {
        printf("%s\r", v);
    }

    template<LogMod _mod>
    void LogOutput(const char *v)
    {
        switch (_mod)
        {
            case INFO:
                printf("[INFO]: [%s]\r\n", v);
                break;
            case DEBUG:
                printf("[DEBUG]: [%s]\r\n", v);
                break;
            case ERROR:
                printf("[ERROR]: [%s]\r\n", v);
                break;
        }
    }
};

}

#endif //EPUB_DISPLAY_HPP
