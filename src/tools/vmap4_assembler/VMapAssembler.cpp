////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2018 TrinityCore    ///
///   <https://www.trinitycore.org/>     ///
///   Copyright 2005-2011 MaNGOS         ///
///   <http://getmangos.com/>            ///
///                                      ///
////////////////////////////////////////////

#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#include "TileAssembler.h"
#include "Banner.h"

int main(int argc, char* argv[])
{
    unsigned int hwCores = std::thread::hardware_concurrency();
    unsigned int usedThreads = hwCores > 0 ? hwCores : 4;
    std::cout << "\n  Extractor Tools v1.0.5 - Copyright (C)2026 Apheleos\n  - Multicore/Multithreading support\n  - Legion 7.3.5 (build 26972)\n\n  Hardware: " << hwCores << " logical processors detected\n  Using " << usedThreads << " threads for extraction\n" << std::endl;
    for (int i = 3; i > 0; --i) { std::cout << "  Starting in " << i << "...\r" << std::flush; std::this_thread::sleep_for(std::chrono::seconds(1)); }
    std::cout << "                    " << std::endl;

    std::string src = "ClientData/Buildings";
    std::string dest = "ClientData/vmaps";

    if (argc > 3)
    {
        std::cout << "usage: " << argv[0] << " <raw data dir> <vmap dest dir>" << std::endl;
        return 1;
    }
    else
    {
        if (argc > 1)
            src = argv[1];
        if (argc > 2)
            dest = argv[2];
    }

    std::cout << "using " << src << " as source directory and writing output to " << dest << std::endl;

    VMAP::TileAssembler* ta = new VMAP::TileAssembler(src, dest);

    if (!ta->convertWorld2())
    {
        std::cout << "exit with errors" << std::endl;
        delete ta;
        return 1;
    }

    delete ta;
    std::cout << "Ok, all done" << std::endl;
    return 0;
}
