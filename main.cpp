#include "commands.hpp"
#include "slpit.hpp"
#include "xhtml.hpp"

using bw::command::Command;
using bw::command::parameter::CommandStorage;
using bw::xhtml::Xhtml;
using bw::xhtml::XhtmlMod;
using bw::slpit::Slpit;
using bw::display::Display;

constexpr char SLPIT_SYMBOL[] = "##split##";
constexpr CommandStorage COMMAND_GROUP[] = {
        {"in_file"},
        {"out_dir"},
        {"suffix"},
        {"name"},
        {"xhtml_namespace"},
};

void HandleCommand(CommandStorage *storage, int storageLen)
{
    Display::Instance().Clean().Show<Display::INFO>("Start process...");

    if (!Slpit::Instance().SetSlpitSymbol(SLPIT_SYMBOL)
            .SetFilesName(storage[3].userValue)
            .SetFilesSuffix(storage[2].userValue)
            .SetInFilePath(storage[0].userValue)
            .SetOutDirPath(storage[1].userValue)
            .Process()
            .IsOk())
    {
        Display::Instance().Show<Display::ERROR>("Split files to fail");
        return;
    } else
    {
        Display::Instance().Show<Display::INFO>("Split files to success");
    }

    if (!Xhtml::Instance().SetXhtmlMod(XhtmlMod::DIR)
            .SetNameSpace(storage[4].userValue)
            .SetOutDirPath(storage[1].userValue)
            .Process()
            .IsOk())
    {
        Display::Instance().Show<Display::ERROR>("Xhtml processing to fail");
        return;
    } else
    {
        Display::Instance().Show<Display::INFO>("Xhtml processing to success");
    }
}

int main(int argc, char *args[])
{
    Command::Instance().Init(COMMAND_GROUP, (sizeof(COMMAND_GROUP) / sizeof(CommandStorage)))
            .Register(HandleCommand)
            .Run(argc, args);
    return 0;
}