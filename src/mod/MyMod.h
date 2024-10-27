#pragma once

#include "ll/api/mod/NativeMod.h"
#include "Config.h"

#include <fmt/format.h>
#include <functional>
#include <ll/api/Config.h>
#include <ll/api/Logger.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/data/KeyValueDB.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>
#include <ll/api/form/ModalForm.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/service/Bedrock.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/registry/ItemStack.h>
#include <memory>
#include <stdexcept>

namespace my_mod {

class MyMod {

public:
    config config;
    static MyMod& getInstance();

    //构造函数
    MyMod(ll::mod::NativeMod& self) : mSelf(self),logger("MyMod") {
        logger.setFile("logs/mymod.log", true); // 设置日志文件路径，并以追加模式写入
        // logger.setPlayerOutputFunc([](std::string_view msg) {
        //     // 这里将日志信息输出到玩家
        //     std::cout << msg << std::endl;
        // });
    }
    
    

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();
    
private:
    ll::mod::NativeMod& mSelf;
    ll::Logger logger;//日志
    std::unique_ptr<ll::data::KeyValueDB> playerDb;//KVDB数据库指针
    ll::event::ListenerPtr playerJoinEventListener;//监听-玩家进服
    ll::event::ListenerPtr playerUseItemEventListener;//监听-玩家使用物品
};

} // namespace my_mod
