#include "mod/MyMod.h"

#include <cmath>
#include <memory>

#include "ll/api/mod/RegisterHelper.h"

namespace my_mod {

static std::unique_ptr<MyMod> instance;

MyMod& MyMod::getInstance() { return *instance; }

bool MyMod::load() {

    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.

    logger.info("这是一个测试");

    // 链接数据库
    const auto& playerDbPath = mSelf.getDataDir() / "players";
    playerDb                 = std::make_unique<ll::data::KeyValueDB>(playerDbPath);

    // 读取配置
    const auto& configFilePath = mSelf.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(config, configFilePath)) {
        logger.warn("没找到配置文件 {}", configFilePath);
        logger.info("读取配置文件");

        if (!ll::config::saveConfig(config, configFilePath)) {
            logger.error("Cannot save default configurations to {}", configFilePath);
        }
    }
    return true;
}

bool MyMod::enable() {

    getSelf().getLogger().debug("Enabling...");


    // 注册命令
    // 注册了一个自杀命令
    auto commandRegistry = ll::service::getCommandRegistry();
    if (!commandRegistry) {
        throw std::runtime_error("failed to get command registry");
    }

    auto& command = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand("killself", "自杀", CommandPermissionLevel::Any);
    command.overload().execute([](CommandOrigin const& origin, CommandOutput& output) {
        auto* entity = origin.getEntity();
        if (entity == nullptr || !entity->isType(ActorType::Player)) {
            output.error("Only players can commit suicide");
            return;
        }

        auto* player = static_cast<Player*>(entity); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        player->kill();
    });

    // 添加一个 玩家进入服务器 监听器

    auto& eventBus = ll::event::EventBus::getInstance();

    playerJoinEventListener = eventBus.emplaceListener<ll::event::player::PlayerJoinEvent>(
        [doGiveClockOnFirstJoin = config.doGiveClockOnFirstJoin,
         &logger                = this->logger,
         &playerDb              = playerDb](ll::event::player::PlayerJoinEvent& event) {
            if (doGiveClockOnFirstJoin) {
                auto& player = event.self();

                const auto& uuid = player.getUuid();

                // Check if the player has joined before.
                if (!playerDb->get(uuid.asString())) {

                    ItemStack itemStack("clock", 1);
                    player.add(itemStack);

                    // Must refresh inventory to see the clock.
                    player.refreshInventory();

                    // Mark the player as joined.
                    if (!playerDb->set(uuid.asString(), "true")) {
                        logger.error("Cannot mark {} as joined in database", player.getRealName());
                    }

                    logger.info("First join of {}! Giving them a clock", player.getRealName());
                }
            }
        }
    );


    // 注册监听-玩家使用物品
    // 玩家使用钟表时自杀
    playerUseItemEventListener = eventBus.emplaceListener<ll::event::PlayerUseItemEvent>(
        [enableClockMenu = config.enableClockMenu, &logger = this->logger](ll::event::PlayerUseItemEvent& event) {
            if (enableClockMenu) {
                auto& player    = event.self();
                auto& itemStack = event.item();

                if (itemStack.getRawNameId() == "clock") {
                    ll::form::ModalForm form("Warning", "Are you sure you want to kill yourself?", "Yes", "No");

                    form.sendTo(
                        player,
                        [&logger](
                            Player&                    player,
                            ll::form::ModalFormResult  result,
                            ll::form::FormCancelReason cancelReason
                        ) {
                            if (result) {
                                player.kill();

                                logger.info("{} killed themselves", player.getRealName());
                            }
                        }
                    );
                }
            }
        }
    );
    return true;
}

bool MyMod::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.

    // 注销 玩家进入服务器 监听器
    auto& eventBus = ll::event::EventBus::getInstance();
    eventBus.removeListener(playerJoinEventListener);

    return true;
}

} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::instance);
