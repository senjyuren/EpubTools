#ifndef EPUB_COMMANDS_HPP
#define EPUB_COMMANDS_HPP

#include "kernel.hpp"

namespace bw::command
{

namespace parameter
{

constexpr int COMMAND_STORAGE_MAX_SIZE = 128;
constexpr int COMMAND_USER_VALUE_MAX_SIZE = 2048;

typedef struct
{
    const char *userCommand;
    int userCommandLen;

    char userValue[COMMAND_USER_VALUE_MAX_SIZE];
} CommandStorage;

constexpr char COMMAND_POINT_SYMBOL[] = "-";
constexpr char COMMAND_LINK_SYMBOL[] = "=";

}

class Command
{
    using CommandStorage = parameter::CommandStorage;

    using VoidFn = void (*)(CommandStorage *v, int vLen);
public:
    static Command &Instance()
    {
        static Command *obj = nullptr;
        if (obj == nullptr)
            obj = new Command();
        return *obj;
    }

    Command &Init(const CommandStorage *commands, int commandLen)
    {
        int i = 0;

        if ((commands == nullptr) || (commandLen <= 0))
            return *this;

        this->mCommandStorageLen = commandLen;
        memcpy(this->mCommandStorage, commands, (commandLen * sizeof(CommandStorage)));
        for (i = 0; i < this->mCommandStorageLen; ++i)
            this->mCommandStorage[i].userCommandLen = static_cast<int>(strlen(this->mCommandStorage[i].userCommand));

        this->isOk = true;
        return *this;
    }

    template<typename T>
    Command &Register(Type<T> result)
    {
        if (!this->isOk)
            return *this;

        this->mVoidFn = result;
        return *this;
    }

    void Run(int argc, char *args[])
    {
        int i = 0;
        int j = 0;
        int step = 0;

        int cmdValueLen = 0;
        int cmdNameLen = 0;
        const char *cmdName = nullptr;

        if ((argc <= 0) || (args == nullptr) || (!this->isOk))
            return;

        for (i = 0; i < argc; ++i)
        {
            step = 0;
            if (args[i][step] != parameter::COMMAND_POINT_SYMBOL[step])
                continue;

            cmdName = &args[i][++step];
            cmdNameLen = static_cast<int>(strlen(cmdName));
            for (j = 0; j < cmdNameLen; ++j)
            {
                if (args[i][step] != parameter::COMMAND_LINK_SYMBOL[0])
                    ++step;
                else
                    break;
            }

            cmdNameLen = static_cast<int>(&args[i][step] - cmdName);
            ++step;
            for (j = 0; j < this->mCommandStorageLen; ++j)
            {
                if (this->mCommandStorage[j].userCommandLen != cmdNameLen)
                    continue;
                if (memcmp(this->mCommandStorage[j].userCommand, cmdName, static_cast<size_t>(cmdNameLen)) != 0)
                    continue;

                cmdValueLen = static_cast<int>(strlen(&args[i][step]));
                memcpy(this->mCommandStorage[j].userValue, &args[i][step], static_cast<size_t>(cmdValueLen));
            }
        }

        if (this->mVoidFn != nullptr)
            this->mVoidFn(this->mCommandStorage, this->mCommandStorageLen);
    }

private:
    bool isOk = false;

    VoidFn mVoidFn = nullptr;
    int mCommandStorageLen = 0;
    CommandStorage mCommandStorage[parameter::COMMAND_STORAGE_MAX_SIZE]{};

    Command() = default;
};

}

#endif //EPUB_COMMANDS_HPP
