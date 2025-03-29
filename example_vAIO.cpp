#include <windows.h>
#define SDK_IMPLEMENTATION
#include "sdk.hpp"
#include <cmath>
#include <iostream>
//#include "altxtarget.hpp"
#include "ease.hpp"
#include "tools.cpp"
#include "drawing.hpp"
#include "brolaf.cpp"
#include "mundo.cpp"
#include "twitch.hpp"
//#include "jinx.hpp"
#include "kindred.cpp"
#include "lux.hpp"
#include "brandv2.hpp"
#include "irelia.cpp"
#include "kaisa.cpp"
#include "samira.cpp"
#include "jhin.hpp"
#include "draven.cpp"
#include "aurora.cpp"
#include "veigarv2.hpp"
#include "trundle.hpp"
#include "pyke.cpp"
#include "sett.hpp"
#include "mordekaiser.cpp"
#include "sylas.cpp"
#include "milio.hpp"
#include "teemo.cpp"
#include "yasuo.hpp"
#include "karthusv2.hpp"
#include "caitlynv2.hpp"
#include "hecarim.hpp"
#include "morgana.hpp"


// vAIO
#include "vUtils.hpp"
#include "Thresh.cpp"
#include "Nautilus.cpp"

// end vAIO

// #include "syndra.cpp"
// #include "custom_menu.cpp"

// #include "veigar.cpp"
// #include "tahmkench.cpp"

namespace script {
    static bool master_enabled = true;

    static void create_master_menu() {
        if (sdk::menu_api::create_sub_menu("Altx0 AIO", true)) {
            sdk::menu_api::create_checkbox("Enable Script", &master_enabled);
            sdk::menu_api::end_sub_menu();
        }
    }

    // vAIO
    static bool vaio_enabled = true;

    void create_vaio_menu() {
        if (sdk::menu_api::create_sub_menu("[vAIO] Settings", true)) {
            vaio_enabled = sdk::menu_api::init_bool("[vAIO] Enabled", true);
            sdk::menu_api::create_checkbox("Enable [vAIO]", &vaio_enabled);

            // Always create menus for supported champions
            auto local = sdk::object_manager::get_local();
            if (local) {
                const std::string champ_name = local->object_name();
                if (champ_name == "Nautilus") {
                    Nautilus::CreateMenu();
                } else if (champ_name == "Thresh") {
                    Thresh::CreateMenu();
                }
            }

            sdk::menu_api::end_sub_menu();
        }
    }

    // end vAIO
} // namespace script

static void on_draw(void*) {
    if (!script::master_enabled) {
        return;
    }

    auto local = sdk::object_manager::get_local();
    if (!local) {
        return;
    }

    // if (sdk::utils::is_key_clicked(VK_F3)) {
    // custom_menu::toggle_menu_visibility();
    //}

    // custom_menu::render_menus();

    drawings::handle_ui_drag();
    drawings::watermark();
    drawings::hotkey_status();
    drawings::update_animations();
    drawings::update_glow_circles();

    if (local->object_name() == "Twitch") {
        twitch::draw_stealth_timer(local);
    }

    if (local->object_name() == "Kaisa") {
        kaisa::draw_e_cast_time(local);
    }

    if (local->object_name() == "Brand") {
        brand::on_draw();
    }

    if (local->object_name() == "Caitlyn") {
        caitlyn::on_draw();
    }

    if (local->object_name() == "Lux") {
        lux::on_draw();
    }

    if (local->object_name() == "Milio") {
        milio::on_draw();
    }

    if (local->object_name() == "Mordekaiser") {
        mordekaiser::on_draw();
    }

    if (local->object_name() == "Sett") {
        sett::on_draw();
    }

    if (local->object_name() == "Sylas") {
        sylas::on_draw();
    }

    if (local->object_name() == "Teemo") {
        teemo::on_draw();
    }

    if (local->object_name() == "Trundle") {
        trundle::on_draw();
    }

    if (local->object_name() == "Veigar") {
        veigar::on_draw();
    } else if (local->object_name() == "Hecarim") {
        hecarim::on_draw();
    }

    if (local->object_name() == "Morgana") {
        morgana::on_draw();
    }
}

static void on_update_callback(void*) {
    if (!script::master_enabled) {
        return;
    }

    // vAIO
    if (!script::vaio_enabled) {
        return;
    }
    // end vAIO

    auto local = sdk::object_manager::get_local();
    if (!local) {
        return;
    }

    //altxtarget::target_selector();
    tools::handle_auto_smite(local);

    if (sdk::evade::is_active()) {
        return;
    }

    static float last_attack_time = 0.0f;
    float current_time = sdk::get_time();
    if (current_time - last_attack_time > 2.0f) {
        sdk::orbwalker::on_cast();
        last_attack_time = current_time;
    }

    const std::string champ_name = local->object_name();

    // if (sdk::utils::is_key_down('O')) {
    // tools::check_local_buffs(local);
    // }

    if (champ_name == "Olaf") {
        brolaf::handle_olaf_q(local);
        brolaf::handle_olaf_w(local);
        brolaf::handle_olaf_e(local);
        brolaf::handle_olaf_r(local);
        brolaf::handle_olaf_q_harass(local);
    } else if (champ_name == "DrMundo") {
        mundo::handle_mundo_q(local);
        mundo::handle_mundo_e(local);
    } else if (champ_name == "Twitch") {
        twitch::handle_twitch_q(local);
        twitch::handle_twitch_w(local);
        twitch::handle_twitch_e(local);
    }  else if (champ_name == "Kindred") {
        kindred::handle_kindred_q(local);
        kindred::handle_kindred_w(local);
        kindred::handle_kindred_e(local);
        kindred::handle_kindred_r(local);
        kindred::handle_kindred_jungle(local);
    } else if (champ_name == "TahmKench") {
        //      tahmkench::handle_tahm_q(local);
        // tahmkench::handle_tahm_e(local);
    } else if (champ_name == "Lux") {
        lux::update_spell_states(local);
        lux::handle_combo(local);
        lux::handle_lux_e_minion(local);
    } else if (champ_name == "Brand") {
        brand::update_spell_states(local);
        brand::handle_combo(local);
        brand::handle_lane_clear(local);
    } else if (champ_name == "Irelia") {
        irelia::update_spell_states(local);
        irelia::handle_irelia_q_minion(local);
        irelia::handle_lane_clear(local);
        irelia::handle_combo_mode_v2(local);
        irelia::handle_flee_mode(local);
    } else if (champ_name == "Veigar") {
        veigar::update_spell_states(local);
        veigar::handle_last_hit_q(local);
        veigar::handle_lane_clear(local);
        veigar::handle_combov2(local);
        veigar::handle_harass(local);
    } else if (champ_name == "Trundle") {
        trundle::update_spell_states(local);
        trundle::handle_lane_clear(local);
        trundle::handle_combo(local);
        trundle::handle_harass(local);
    } else if (champ_name == "Sett") {
        sett::update_spell_states(local);
        sett::handle_combo(local);
        sett::handle_lane_clear(local);
    } else if (champ_name == "Mordekaiser") {
        mordekaiser::update_spell_states(local);
        mordekaiser::handle_combo(local);
        mordekaiser::handle_lane_clear(local);
    } else if (champ_name == "Yasuo") {
        yasuo::update_spell_states(local);
        yasuo::handle_combo(local);
        yasuo::handle_harass(local);
        yasuo::handle_lane_clear(local);
        yasuo::handle_last_hit(local);
        yasuo::handle_flee(local);
    } else if (champ_name == "Karthus") {
        karthus::update_spell_states(local);
        karthus::handle_combo(local);
        karthus::handle_lane_clear(local);
        karthus::handle_flee(local);
        karthus::handle_r(local);
    } else if (champ_name == "Morgana") {
        morgana::update_spell_states(local);
        morgana::handle_combo(local);
    }
    
    else if (champ_name == "Sylas") {
        sylas::update_spell_states(local);
        sylas::handle_combo(local);
        sylas::handle_lane_clear(local);
        sylas::handle_harass(local);
    } else if (champ_name == "Teemo") {
        teemo::update_spell_states(local);
        teemo::handle_combo(local);
        teemo::handle_lane_clear(local);
        teemo::handle_flee(local);
    }

    else if (champ_name == "Pyke") {
        pyke::update_spell_states(local);
        pyke::handle_combo(local);
        pyke::test_q_mode(local);

    } else if (champ_name == "Milio") {
        milio::update_spell_states(local);
        milio::handle_combo(local);
        milio::handle_lane_clear(local);
        milio::handle_flee(local);
       // milio::handle_utility(local);
    } else if (champ_name == "Caitlyn") {
        caitlyn::update_spell_states(local);
        caitlyn::handle_combo(local);
        caitlyn::handle_lane_clear(local);
        caitlyn::handle_flee(local);
    }
    
    else if (champ_name == "Hecarim") {
        hecarim::update_spell_states(local);
        hecarim::handle_combo(local);
        hecarim::handle_lane_clear(local);
    }
    
    else if (champ_name == "Kaisa") {
        kaisa::update(local);
        kaisa::handle_kaisa_q(local);
        kaisa::handle_kaisa_w(local);
        kaisa::handle_kaisa_e(local);
        kaisa::handle_kaisa_r(local);

        if (kaisa::can_move_during_cast(local)) {
            static float last_move_time = 0;
            const float time = sdk::get_time();
            if (time - last_move_time < 0.3f) {
                return;
            }
            sdk::move_to(sdk::get_cursor());
            last_move_time = time;
        }

    } else if (champ_name == "Samira") {
        samira::handle_samira_r(local);
        samira::handle_e_chain_kills(local);
        samira::handle_stack_building(local);
        samira::handle_samira_q_harass(local);
        samira::handle_laneclear(local);

        if (samira::can_move_during_cast(local)) {
            static float last_move_time = 0;
            const float time = sdk::get_time();
            if (time - last_move_time < 0.5f) {
                return;
            }
            sdk::move_to(sdk::get_cursor());
            last_move_time = time;
        }

    } else if (champ_name == "Jhin") {
        jhin::handle_jhin_q(local);
        jhin::handle_jhin_w(local);
        jhin::handle_jhin_e(local);
        jhin::handle_jhin_r(local);
    } else if (champ_name == "Draven") {
        draven::handle_draven_q(local);
    } else if (champ_name == "Aurora") {
        aurora::handle_combo(local);
    } // else if (champ_name == "Syndra") {
      // syndra::handle_syndra_q(local);
      ////syndra::handle_syndra_e(local);
    ////}
    ///
    // vAIO
    else if (champ_name == "Nautilus") {
        Nautilus::Update();
    } else if (champ_name == "Thresh") {
        Thresh::Update();
    }
}

static void on_update_menu(void*) {
    script::create_master_menu();
    drawings::create_drawing_menu();
    //altxtarget::create_menu();

    // vAIO
    script::create_vaio_menu();
    // end vAIO

    auto local = sdk::object_manager::get_local();
    if (!local) {
        return;
    }

    if (local->object_name() == "Twitch") {
        twitch::create_twitch_menu();
    } else if (local->object_name() == "Olaf") {
        brolaf::create_olaf_menu();
    } else if (local->object_name() == "Kaisa") {
        kaisa::create_kaisa_menu();
    } else if (local->object_name() == "Samira") {
        samira::create_samira_menu();
    } else if (local->object_name() == "Jhin") {
        jhin::create_jhin_menu();
    } else if (local->object_name() == "Draven") {
        draven::create_draven_menu();
    } else if (local->object_name() == "Aurora") {
        aurora::create_aurora_menu();
    } else if (local->object_name() == "Teemo") {
        teemo::create_teemo_menu();
    } else if (local->object_name() == "Sylas") {
        sylas::create_sylas_menu();
    } else if (local->object_name() == "Mordekaiser") {
        mordekaiser::create_mordekaiser_menu();
    } else if (local->object_name() == "Yasuo") {
        yasuo::create_yasuo_menu();
    } else if (local->object_name() == "Brand") {
        brand::create_brand_menu();
    } else if (local->object_name() == "Karthus") {
        karthus::create_karthus_menu();
    } else if (local->object_name() == "Milio") {
        milio::create_milio_menu();
    } else if (local->object_name() == "Sett") {
        sett::create_sett_menu();
    } else if (local->object_name() == "Veigar") {
        veigar::create_veigar_menu();
    } else if (local->object_name() == "Hecarim") {
        hecarim::create_hecarim_menu();
    } else if (local->object_name() == "Morgana") {
        morgana::create_morgana_menu();
    }
    
    // else if (local->object_name() == "Syndra") {
      // syndra::create_syndra_menu();
    //}

    else if (local->object_name() == "Caitlyn") {
        caitlyn::create_caitlyn_menu();
    }

    else if (local->object_name() == "Lux") {
        lux::create_lux_menu();
    }

    else if (local->object_name() == "Trundle") {
        trundle::create_trundle_menu();
    }
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, sdk::FnLoadFunction load_function) {
    sdk::init(load_function);

    if (reason == DLL_PROCESS_ATTACH) {
        auto local = sdk::object_manager::get_local();
        if (!local) {
            return TRUE;
        }

        const std::string champ_name = local->object_name();
        std::string display_name;
        std::string hotkeys;

        const std::string global_hotkeys = "Global Hotkeys:\nHold Mouse4: Auto Smite\n\n";

        struct ChampConfig {
            std::string display;
            std::string keys;
        };

        const std::unordered_map<std::string, ChampConfig> champ_configs = {
            {"Olaf", {"Olaf", "M: E Mode\nL: Change Q Hitchance"}},
            {"DrMundo", {"Dr.Mundo", "none"}},
            {"Twitch",
             {"Twitch", "M: Toggle E Mode (2+ enemies killable)\nAuto Q after AA on champions\nAuto W on predicted position\nAuto E execute"
             }},
            {"Kindred", {"Kindred", "none"}},
            {"Kaisa", {"Kaisa", "none"}},
            {"Samira", {"Samira", "SPACE, V laneclear, C harass"}},
            {"Jhin", {"Jhin", "none"}},
            {"Aurora", {"Aurora", "Q, E"}},
            {"Draven", {"Draven", "none"}},
            {"Veigar", {"Veigar", "Supports all modes"}},
            {"Irelia", {"Irelia", "Supports all modes"}},
            {"Trundle", {"Trundle", "Supports all modes"}},
            {"Pyke", {"Pyke", "Has Combo mode only (Not working)"}},
            {"Caitlyn", {"Caitlyn", "Supports Combo, Lane Clear and Flee"}},
            {"Sett", {"Sett", "Supports Combo and Lane clear"}},
            {"Lux", {"Lux", "Supports Combo and Lane clear"}},
            {"Brand", {"Brand", "Supports Combo and Lane clear + jungle"}},
            {"Mordekaiser", {"Mordekaiser", "Supports Combo and Lane clear + jungle"}},
            {"Sylas", {"Sylas", "Supports combo and Harass and Lane clear"}},
            {"Milio", {"Milio", "Supports Q and 1 E stack"}},
            {"Teemo", {"Teemo", "Supports Combo and Harass and Lane Clear and Flee"}},
            {"Yasuo", {"Yasuo", "Supports Como + LaneClear + Harass + Flee"}},
            {"Yasuo", {"Yasuo", "Supports Combo and Harass and Lane Clear and Flee"}},
            {"Hecarim", {"Hecarim", "Supports Combo and Lane Clear + jungle"}},
            {"Morgana", {"Morgana", "Coding"}}
        };

        auto config_it = champ_configs.find(champ_name);
        if (config_it != champ_configs.end()) {
            display_name = config_it->second.display;
            hotkeys = global_hotkeys + config_it->second.keys;
        } else {
            display_name = "Core Champion";
            hotkeys = global_hotkeys;
        }

        auto position = local->position();
        position.y -= 200.0f;

        sdk::utils::create_world_message(
            position, "Altx0 Script Loaded", display_name + " Active\n" + hotkeys, sdk::Color(128, 0, 128), 25.0f
        );

        if (champ_name == "Jhin") {
            jhin::enable_q = sdk::menu_api::init_bool("jhin:enable_q", true);
            jhin::enable_w = sdk::menu_api::init_bool("jhin:enable_w", true);
            jhin::enable_e = sdk::menu_api::init_bool("jhin:enable_e", true);
            jhin::enable_r = sdk::menu_api::init_bool("jhin:enable_r", true);
            jhin::w_only_on_root = sdk::menu_api::init_bool("jhin:w_only_root", false);
            jhin::w_killsteal = sdk::menu_api::init_bool("jhin:w_killsteal", true);
            jhin::q_only_reload = sdk::menu_api::init_bool("jhin:q_only_reload", true);

            jhin::w_hitchance = sdk::menu_api::init_int("jhin:w_hitchance", 1);
            jhin::e_hitchance = sdk::menu_api::init_int("jhin:e_hitchance", 1);
            jhin::r_hitchance = sdk::menu_api::init_int("jhin:r_hitchance", 1);

            /* custom_menu::init_jhin_menu(
                 jhin::enable_q, jhin::enable_w, jhin::enable_e, jhin::enable_r,
                 jhin::w_only_on_root, jhin::w_killsteal, jhin::q_only_reload,
                 jhin::w_hitchance, jhin::e_hitchance, jhin::r_hitchance
             );*/
        } else if (champ_name == "Teemo") {
            teemo::enable_teemo = sdk::menu_api::init_bool("teemo:enable_script", true);

            // Ability Toggles
            teemo::enable_q = sdk::menu_api::init_bool("teemo:enable_q", true);
            teemo::enable_w = sdk::menu_api::init_bool("teemo:enable_w", true);
            teemo::enable_r = sdk::menu_api::init_bool("teemo:enable_r", true);

            // Combo Settings
            teemo::use_q_combo = sdk::menu_api::init_bool("teemo:use_q_combo", true);
            teemo::w_hp_threshold = sdk::menu_api::init_int("teemo:w_hp_threshold", 30);              // Default: 30%
            teemo::r_mushroom_proximity = sdk::menu_api::init_int("teemo:r_mushroom_proximity", 150); // Default: 150

            // Harass Settings
            teemo::use_q_harass = sdk::menu_api::init_bool("teemo:use_q_harass", true);

            // Lane Clear Settings
            teemo::enable_r_laneclear = sdk::menu_api::init_bool("teemo:enable_r_laneclear", false);

            // Range Percentage Settings
            teemo::q_range_percentage = sdk::menu_api::init_int("teemo:q_range_percentage", 100); // Default 100% (max range)

            // Add drawing settings initialization
            teemo::draw_q_range = sdk::menu_api::init_bool("teemo:draw_q_range", true);
            teemo::draw_r_range = sdk::menu_api::init_bool("teemo:draw_r_range", true);
        } else if (champ_name == "Sett") {
            // Main toggle
            sett::enable_sett = sdk::menu_api::init_bool("sett:enable_script", true);
            
            // Combo Settings
            sett::enable_q_combo = sdk::menu_api::init_bool("sett:enable_q_combo", true);
            sett::enable_w_combo = sdk::menu_api::init_bool("sett:enable_w_combo", true);
            sett::enable_e_combo = sdk::menu_api::init_bool("sett:enable_e_combo", true);
            sett::w_hp_threshold = sdk::menu_api::init_int("sett:w_hp_threshold", 30);  // Default 30%
            
            // Lane Clear Settings
            sett::enable_q_clear = sdk::menu_api::init_bool("sett:enable_q_clear", true);
            
            // Ability Settings
            sett::w_range_percent = sdk::menu_api::init_int("sett:w_range_percent", 85);
            sett::e_range_percent = sdk::menu_api::init_int("sett:e_range_percent", 85);
            sett::w_hitchance = sdk::menu_api::init_int("sett:w_hitchance", 3);
            sett::e_hitchance = sdk::menu_api::init_int("sett:e_hitchance", 3);
            
            // Add drawing settings initialization
            sett::draw_w_range = sdk::menu_api::init_bool("sett:draw_w_range", true);
            sett::draw_e_range = sdk::menu_api::init_bool("sett:draw_e_range", true);
            sett::draw_r_range = sdk::menu_api::init_bool("sett:draw_r_range", true);
        }
        
        else if (champ_name == "Sylas") {
            sylas::enable_sylas = sdk::menu_api::init_bool("sylas:enable_script", true);

            sylas::enable_q = sdk::menu_api::init_bool("sylas:enable_q", true);
            sylas::enable_w = sdk::menu_api::init_bool("sylas:enable_w", true);
            sylas::enable_e = sdk::menu_api::init_bool("sylas:enable_e", true);

            sylas::use_q_combo = sdk::menu_api::init_bool("sylas:use_q_combo", true);
            sylas::use_w_combo = sdk::menu_api::init_bool("sylas:use_w_combo", true);
            sylas::use_e1_combo = sdk::menu_api::init_bool("sylas:use_e1_combo", true);
            sylas::use_e2_combo = sdk::menu_api::init_bool("sylas:use_e2_combo", true);
            
            sylas::use_e2_under_turret = sdk::menu_api::init_bool("sylas:use_e2_under_turret", false);
            sylas::enemy_hp_under_turret = sdk::menu_api::init_int("sylas:enemy_hp_under_turret", 30);
            sylas::local_hp_under_turret = sdk::menu_api::init_int("sylas:local_hp_under_turret", 50);

            sylas::use_q_harass = sdk::menu_api::init_bool("sylas:use_q_harass", true);

            sylas::use_q_lane_clear = sdk::menu_api::init_bool("sylas:use_q_lane_clear", true);
            sylas::use_w_lane_clear = sdk::menu_api::init_bool("sylas:use_w_lane_clear", true);

            sylas::q_hitchance = sdk::menu_api::init_int("sylas:q_hitchance", 3);   // Default high
            sylas::e1_hitchance = sdk::menu_api::init_int("sylas:e1_hitchance", 3); // Default medium
            sylas::e2_hitchance = sdk::menu_api::init_int("sylas:e2_hitchance", 3); // Default medium

            sylas::q_range_percentage = sdk::menu_api::init_int("sylas:q_range_percentage", 100);   // Default 100% (max range)
            sylas::w_range_percentage = sdk::menu_api::init_int("sylas:w_range_percentage", 100);   // Default 100% (max range)
            sylas::e1_range_percentage = sdk::menu_api::init_int("sylas:e1_range_percentage", 100); // Default 100% (max range)
            sylas::e2_range_percentage = sdk::menu_api::init_int("sylas:e2_range_percentage", 100); // Default 100% (max range)

            sylas::draw_q_range = sdk::menu_api::init_bool("sylas:draw_q_range", true);
            sylas::draw_w_range = sdk::menu_api::init_bool("sylas:draw_w_range", true);
            sylas::draw_e1_range = sdk::menu_api::init_bool("sylas:draw_e1_range", true);
            sylas::draw_e2_range = sdk::menu_api::init_bool("sylas:draw_e2_range", true);
        } else if (champ_name == "Brand") {
            brand::enable_brand = sdk::menu_api::init_bool("brand:enable_script", true);
            
            // Combo settings
            brand::use_q_combo = sdk::menu_api::init_bool("brand:use_q_combo", true);
            brand::use_w_combo = sdk::menu_api::init_bool("brand:use_w_combo", true);
            brand::use_e_combo = sdk::menu_api::init_bool("brand:use_e_combo", true);
            brand::use_r_combo = sdk::menu_api::init_bool("brand:use_r_combo", true);
            brand::use_e_bounce_combo = sdk::menu_api::init_bool("brand:use_e_bounce_combo", true);

            // Harass settings
            brand::use_q_harass = sdk::menu_api::init_bool("brand:use_q_harass", true);
            brand::use_w_harass = sdk::menu_api::init_bool("brand:use_w_harass", true);
            brand::use_e_harass = sdk::menu_api::init_bool("brand:use_e_harass", true);

            // Lane Clear settings
            brand::use_w_lane_clear = sdk::menu_api::init_bool("brand:use_w_lane_clear", true);
            brand::use_e_lane_clear = sdk::menu_api::init_bool("brand:use_e_lane_clear", true);

            // Jungle Clear settings
            brand::use_w_jungle_clear = sdk::menu_api::init_bool("brand:use_w_jungle_clear", true);
            brand::use_q_jungle_clear = sdk::menu_api::init_bool("brand:use_q_jungle_clear", true);
            brand::use_e_jungle_clear = sdk::menu_api::init_bool("brand:use_e_jungle_clear", true);

            // Ability settings
            brand::q_hitchance = sdk::menu_api::init_int("brand:q_hitchance", 3);  // Default high
            brand::w_hitchance = sdk::menu_api::init_int("brand:w_hitchance", 3);  // Default high
            brand::r_hp_threshold_low = sdk::menu_api::init_int("brand:r_hp_threshold_low", 20);  // Default 20%
            brand::r_hp_threshold_high = sdk::menu_api::init_int("brand:r_hp_threshold_high", 50); // Default 50%

            // Range settings
            brand::q_range = sdk::menu_api::init_int("brand:q_range", 1100); // Max Q range
            brand::w_range = sdk::menu_api::init_int("brand:w_range", 900);  // Max W range
            
            brand::draw_q_range = sdk::menu_api::init_bool("brand:draw_q_range", true);
            brand::draw_w_range = sdk::menu_api::init_bool("brand:draw_w_range", true);
            brand::draw_e_range = sdk::menu_api::init_bool("brand:draw_e_range", true);
            brand::draw_r_range = sdk::menu_api::init_bool("brand:draw_r_range", true);

        } else if (champ_name == "Milio") {
            // Main toggle
            milio::enable_milio = sdk::menu_api::init_bool("milio:enable_script", true);
            
            // Combo settings
            milio::enable_q_combo = sdk::menu_api::init_bool("milio:enable_q_combo", true);
            milio::enable_w_combo = sdk::menu_api::init_bool("milio:enable_w_combo", true);
            milio::enable_e_combo = sdk::menu_api::init_bool("milio:enable_e_combo", true);
            milio::ally_e_health_threshold = sdk::menu_api::init_int("milio:ally_e_health_threshold", 30);

            // Lane Clear settings
            milio::enable_q_laneclear = sdk::menu_api::init_bool("milio:enable_q_laneclear", true);

            // Flee settings
            milio::enable_q_flee = sdk::menu_api::init_bool("milio:enable_q_flee", true);
            
            // Ability settings
            milio::q_hitchance = sdk::menu_api::init_int("milio:q_hitchance", 3);  // Default high
            milio::q_range = sdk::menu_api::init_int("milio:q_range", 850);        // Default max range
            
            // Drawing settings
            milio::draw_q_range = sdk::menu_api::init_bool("milio:draw_q_range", true);
            milio::draw_w_range = sdk::menu_api::init_bool("milio:draw_w_range", true);
            milio::draw_e_range = sdk::menu_api::init_bool("milio:draw_e_range", true);
            milio::draw_r_range = sdk::menu_api::init_bool("milio:draw_r_range", true);
            
        } else if (champ_name == "Hecarim") {
            // Main toggle
            hecarim::enable_hecarim = sdk::menu_api::init_bool("hecarim:enable_script", true);
            
            // Combo settings
            hecarim::enable_q_combo = sdk::menu_api::init_bool("hecarim:enable_q_combo", true);
            hecarim::enable_w_combo = sdk::menu_api::init_bool("hecarim:enable_w_combo", true);
            hecarim::enable_e_combo = sdk::menu_api::init_bool("hecarim:enable_e_combo", true);
            hecarim::enable_r_combo = sdk::menu_api::init_bool("hecarim:enable_r_combo", true);
            hecarim::mana_check = sdk::menu_api::init_int("hecarim:mana_check", 10);

            // E HP thresholds
            hecarim::e_local_hp_threshold = sdk::menu_api::init_int("hecarim:e_local_hp_threshold", 40);  // Default 40%
            hecarim::e_enemy_hp_threshold = sdk::menu_api::init_int("hecarim:e_enemy_hp_threshold", 30);  // Default 30%

            // Ability range settings
            hecarim::q_range_percent = sdk::menu_api::init_int("hecarim:q_range_percent", 85); // Default 85%
            hecarim::w_range_percent = sdk::menu_api::init_int("hecarim:w_range_percent", 85); // Default 85%

            // Lane Clear settings
            hecarim::enable_q_laneclear = sdk::menu_api::init_bool("hecarim:enable_q_laneclear", true);
            hecarim::enable_w_laneclear = sdk::menu_api::init_bool("hecarim:enable_w_laneclear", true);

            // Drawing settings
            hecarim::draw_q_range = sdk::menu_api::init_bool("hecarim:draw_q_range", true);
            hecarim::draw_w_range = sdk::menu_api::init_bool("hecarim:draw_w_range", true);
            hecarim::draw_e_range = sdk::menu_api::init_bool("hecarim:draw_e_range", true);
            hecarim::draw_r_range = sdk::menu_api::init_bool("hecarim:draw_r_range", true);

            hecarim::laneclear_mana_check = sdk::menu_api::init_int("hecarim:laneclear_mana_check", 10);  // Default 50%

            hecarim::w_minion_count = sdk::menu_api::init_int("hecarim:w_minion_count", 3);  // Default 3 minions

            
        
        } else if (champ_name == "Mordekaiser") {
            mordekaiser::enable_mordekaiser = sdk::menu_api::init_bool("mordekaiser:enable_script", true);
            mordekaiser::enable_q = sdk::menu_api::init_bool("mordekaiser:enable_q", true);
            mordekaiser::enable_w = sdk::menu_api::init_bool("mordekaiser:enable_w", true);
            mordekaiser::enable_e = sdk::menu_api::init_bool("mordekaiser:enable_e", true);
            mordekaiser::enable_r = sdk::menu_api::init_bool("mordekaiser:enable_r", true);

            mordekaiser::use_q_combo = sdk::menu_api::init_bool("mordekaiser:use_q_combo", true);
            mordekaiser::use_w_combo = sdk::menu_api::init_bool("mordekaiser:use_w_combo", true);
            mordekaiser::use_e_combo = sdk::menu_api::init_bool("mordekaiser:use_e_combo", true);
            mordekaiser::use_r_combo = sdk::menu_api::init_bool("mordekaiser:use_r_combo", true);

            mordekaiser::r_hp_threshold = sdk::menu_api::init_int("mordekaiser:r_hp_threshold", 20); // Default 20%
            mordekaiser::w_hp_threshold = sdk::menu_api::init_int("mordekaiser:w_hp_threshold", 30); // Default 30%

            mordekaiser::use_q_harass = sdk::menu_api::init_bool("mordekaiser:use_q_harass", true);
            mordekaiser::use_w_harass = sdk::menu_api::init_bool("mordekaiser:use_w_harass", false);
            mordekaiser::use_e_harass = sdk::menu_api::init_bool("mordekaiser:use_e_harass", true);

            mordekaiser::use_q_lane_clear = sdk::menu_api::init_bool("mordekaiser:use_q_lane_clear", true);
            mordekaiser::use_e_lane_clear = sdk::menu_api::init_bool("mordekaiser:use_e_lane_clear", true);

            mordekaiser::q_hitchance = sdk::menu_api::init_int("mordekaiser:q_hitchance", 3); // Default high
            mordekaiser::e_hitchance = sdk::menu_api::init_int("mordekaiser:e_hitchance", 2); // Default medium

            mordekaiser::q_range_percentage = sdk::menu_api::init_int("mordekaiser:q_range_percentage", 100); // Default 100% (max range)
            mordekaiser::e_range_percentage = sdk::menu_api::init_int("mordekaiser:e_range_percentage", 100); // Default 100% (max range)
            mordekaiser::r_range_percentage = sdk::menu_api::init_int("mordekaiser:r_range_percentage", 100); // Default 100% (max range)

            // Drawing settings - only initialize here
            mordekaiser::draw_q_range = sdk::menu_api::init_bool("mordekaiser:draw_q_range", true);
            mordekaiser::draw_w_range = sdk::menu_api::init_bool("mordekaiser:draw_w_range", true);
            mordekaiser::draw_e_range = sdk::menu_api::init_bool("mordekaiser:draw_e_range", true);
            mordekaiser::draw_r_range = sdk::menu_api::init_bool("mordekaiser:draw_r_range", true);
        } else if (champ_name == "Yasuo") {
            yasuo::enable_yasuo = sdk::menu_api::init_bool("yasuo:enable_script", true);
            
            // Combo settings
            yasuo::enable_q_combo = sdk::menu_api::init_bool("yasuo:enable_q_combo", true);
            yasuo::enable_q1_q2_combo = sdk::menu_api::init_bool("yasuo:enable_q1_q2_combo", true);
            yasuo::enable_q3_combo = sdk::menu_api::init_bool("yasuo:enable_q3_combo", true);
            yasuo::enable_r_combo = sdk::menu_api::init_bool("yasuo:enable_r_combo", true);
            yasuo::dash_under_turret = sdk::menu_api::init_bool("yasuo:dash_under_turret", true);
            yasuo::enemy_hp_to_dash = sdk::menu_api::init_int("yasuo:enemy_hp_to_dash", 50);
            yasuo::local_hp_to_dash = sdk::menu_api::init_int("yasuo:local_hp_to_dash", 30);
            yasuo::r_under_turret = sdk::menu_api::init_bool("yasuo:r_under_turret", true);
            yasuo::enemy_hp_to_r = sdk::menu_api::init_int("yasuo:enemy_hp_to_r", 30);
            yasuo::local_hp_to_r = sdk::menu_api::init_int("yasuo:local_hp_to_r", 50);

            // Harass settings
            yasuo::enable_q_harass = sdk::menu_api::init_bool("yasuo:enable_q_harass", true);
            yasuo::enable_q1_q2_harass = sdk::menu_api::init_bool("yasuo:enable_q1_q2_harass", true);
            yasuo::enable_q3_harass = sdk::menu_api::init_bool("yasuo:enable_q3_harass", true);
            yasuo::enable_q_farm_minion = sdk::menu_api::init_bool("yasuo:enable_q_farm_minion", true);
            yasuo::enemy_hp_threshold_r = sdk::menu_api::init_int("yasuo:enemy_hp_threshold_r", 50);

            // Lane Clear settings
            yasuo::enable_q1_q2_lane_clear = sdk::menu_api::init_bool("yasuo:enable_q1_q2_lane_clear", true);
            yasuo::enable_q3_lane_clear = sdk::menu_api::init_bool("yasuo:enable_q3_lane_clear", true);

            // Flee settings
            yasuo::dash_degree = sdk::menu_api::init_int("yasuo:dash_degree", 100);

            // Ability settings
            yasuo::q1_q2_range = sdk::menu_api::init_int("yasuo:q1_q2_range", 450);
            yasuo::q3_range = sdk::menu_api::init_int("yasuo:q3_range", 1150);
            yasuo::q3_hitchance = sdk::menu_api::init_int("yasuo:q3_hitchance", 3);
            yasuo::r_range = sdk::menu_api::init_int("yasuo:r_range", 1400);

            // Combo settings
            yasuo::enable_e_combo = sdk::menu_api::init_bool("yasuo:enable_e_combo", true);
            yasuo::e_range = sdk::menu_api::init_int("yasuo:e_range", 475);

            // Combo Dash Degree
            yasuo::combo_dash_degree = sdk::menu_api::init_int("yasuo:combo_dash_degree", 30);
        } else if (champ_name == "Karthus") {
            // Initialize menu variables
            karthus::enable_karthus = sdk::menu_api::init_bool("karthus:enable_script", true);
            karthus::enable_q = sdk::menu_api::init_bool("karthus:enable_q", true);
            karthus::enable_w = sdk::menu_api::init_bool("karthus:enable_w", true);
            karthus::enable_e = sdk::menu_api::init_bool("karthus:enable_e", true);
            karthus::enable_r = sdk::menu_api::init_bool("karthus:enable_r", true);

            karthus::enable_q_farm = sdk::menu_api::init_bool("karthus:enable_q_farm", true);
            karthus::enable_w_farm = sdk::menu_api::init_bool("karthus:enable_w_farm", false);
            karthus::enable_e_farm = sdk::menu_api::init_bool("karthus:enable_e_farm", false);
            karthus::farm_mana_check = sdk::menu_api::init_int("karthus:farm_mana_check", 50); // Default 50%

            karthus::enable_w_flee = sdk::menu_api::init_bool("karthus:enable_w_flee", true);

            karthus::q_hitchance = sdk::menu_api::init_int("karthus:q_hitchance", 3); // Default high
            karthus::w_hitchance = sdk::menu_api::init_int("karthus:w_hitchance", 3); // Default high

            karthus::q_range = sdk::menu_api::init_int("karthus:q_range", 875);   // Default max range
            karthus::w_range = sdk::menu_api::init_int("karthus:w_range", 1000); // Default max range
            karthus::e_range = sdk::menu_api::init_int("karthus:e_range", 550);   // Default max range
        } else if (champ_name == "Lux") {
            // Main toggle
            lux::enable_lux = sdk::menu_api::init_bool("lux:enable_script", true);
            
            // Combo Settings
            lux::enable_q_combo = sdk::menu_api::init_bool("lux:enable_q_combo", true);
            lux::enable_e_combo = sdk::menu_api::init_bool("lux:enable_e_combo", true);
            lux::enable_r_combo = sdk::menu_api::init_bool("lux:enable_r_combo", true);
            
            // Lane Clear Settings
            lux::enable_e_laneclear = sdk::menu_api::init_bool("lux:enable_e_laneclear", true);
            
            // Ability Settings - Hitchance
            lux::q_hitchance = sdk::menu_api::init_int("lux:q_hitchance", 3);  // Default high
            lux::e_hitchance = sdk::menu_api::init_int("lux:e_hitchance", 3);  // Default high
            lux::r_hitchance = sdk::menu_api::init_int("lux:r_hitchance", 3);  // Default high
            
            // Ability Settings - Range Percentages
            lux::q_range_percent = sdk::menu_api::init_int("lux:q_range_percent", 85);
            lux::w_range_percent = sdk::menu_api::init_int("lux:w_range_percent", 85);
            lux::e_range_percent = sdk::menu_api::init_int("lux:e_range_percent", 85);
            lux::r_range_percent = sdk::menu_api::init_int("lux:r_range_percent", 90);

            // Drawing Settings
            lux::draw_q_range = sdk::menu_api::init_bool("lux:draw_q_range", true);
            lux::draw_w_range = sdk::menu_api::init_bool("lux:draw_w_range", false);
            lux::draw_e_range = sdk::menu_api::init_bool("lux:draw_e_range", true);
            lux::draw_r_range = sdk::menu_api::init_bool("lux:draw_r_range", true);
        } else if (champ_name == "Trundle") {
            // Main toggle
            trundle::enable_trundle = sdk::menu_api::init_bool("trundle:enable_script", true);
            
            // Combo Settings
            trundle::enable_q_combo = sdk::menu_api::init_bool("trundle:enable_q_combo", true);
            trundle::enable_w_combo = sdk::menu_api::init_bool("trundle:enable_w_combo", true);
            trundle::enable_e_combo = sdk::menu_api::init_bool("trundle:enable_e_combo", true);
            trundle::enable_r_combo = sdk::menu_api::init_bool("trundle:enable_r_combo", true);
            trundle::r_hp_threshold = sdk::menu_api::init_int("trundle:r_hp_threshold", 50);  // Default 50%

            // Harass Settings
            trundle::enable_q_harass = sdk::menu_api::init_bool("trundle:enable_q_harass", true);
            trundle::enable_w_harass = sdk::menu_api::init_bool("trundle:enable_w_harass", true);
            trundle::enable_e_harass = sdk::menu_api::init_bool("trundle:enable_e_harass", true);

            // Ability Settings
            trundle::e_hitchance = sdk::menu_api::init_int("trundle:e_hitchance", 3);  // Default high
            trundle::w_range_percentage = sdk::menu_api::init_int("trundle:w_range_percentage", 100);  // Default 100%
            trundle::e_range_percentage = sdk::menu_api::init_int("trundle:e_range_percentage", 100);  // Default 100%
            trundle::r_range_percentage = sdk::menu_api::init_int("trundle:r_range_percentage", 100);  // Default 100%

            // Drawing Settings
            trundle::draw_w_range = sdk::menu_api::init_bool("trundle:draw_w_range", true);
            trundle::draw_e_range = sdk::menu_api::init_bool("trundle:draw_e_range", true);
            trundle::draw_r_range = sdk::menu_api::init_bool("trundle:draw_r_range", true);

            // Lane Clear Settings
            trundle::enable_q_clear = sdk::menu_api::init_bool("trundle:enable_q_clear", true);
            trundle::enable_w_clear = sdk::menu_api::init_bool("trundle:enable_w_clear", true);
        } else if (champ_name == "Caitlyn") {
            caitlyn::enable_caitlyn = sdk::menu_api::init_bool("caitlyn:enable_script", true);
            
            // Combo settings
            caitlyn::enable_q_combo = sdk::menu_api::init_bool("caitlyn:enable_q_combo", true);
            caitlyn::enable_w_combo = sdk::menu_api::init_bool("caitlyn:enable_w_combo", true);
            caitlyn::enable_e_combo = sdk::menu_api::init_bool("caitlyn:enable_e_combo", true);
            caitlyn::enable_r_combo = sdk::menu_api::init_bool("caitlyn:enable_r_combo", true);
            caitlyn::mana_check = sdk::menu_api::init_int("caitlyn:mana_check", 30);

            // Lane Clear settings
            caitlyn::enable_q_lane_clear = sdk::menu_api::init_bool("caitlyn:enable_q_lane_clear", true);
            caitlyn::q_min_minions = sdk::menu_api::init_int("caitlyn:q_min_minions", 3);

            // Flee settings
            caitlyn::enable_e_flee = sdk::menu_api::init_bool("caitlyn:enable_e_flee", true);

            // Ability settings
            caitlyn::q_hitchance = sdk::menu_api::init_int("caitlyn:q_hitchance", 3);
            caitlyn::w_hitchance = sdk::menu_api::init_int("caitlyn:w_hitchance", 3);
            caitlyn::e_hitchance = sdk::menu_api::init_int("caitlyn:e_hitchance", 3);
            caitlyn::q_range = sdk::menu_api::init_int("caitlyn:q_range", 70);
            caitlyn::w_range = sdk::menu_api::init_int("caitlyn:w_range", 71);
            caitlyn::e_range = sdk::menu_api::init_int("caitlyn:e_range", 50);
            caitlyn::r_range = sdk::menu_api::init_int("caitlyn:r_range", 100);

            // Drawing settings
            caitlyn::draw_q_range = sdk::menu_api::init_bool("caitlyn:draw_q_range", true);
            caitlyn::draw_w_range = sdk::menu_api::init_bool("caitlyn:draw_w_range", true);
            caitlyn::draw_e_range = sdk::menu_api::init_bool("caitlyn:draw_e_range", true);
            caitlyn::draw_r_range = sdk::menu_api::init_bool("caitlyn:draw_r_range", true);
        } else if (champ_name == "Morgana") {
            morgana::enable_morgana = sdk::menu_api::init_bool("morgana:enable_script", true);
            
            // Combo settings
            morgana::enable_q_combo = sdk::menu_api::init_bool("morgana:enable_q_combo", true);
            morgana::enable_w_combo = sdk::menu_api::init_bool("morgana:enable_w_combo", true);
            morgana::enable_e_combo = sdk::menu_api::init_bool("morgana:enable_e_combo", true);
            morgana::enable_r_combo = sdk::menu_api::init_bool("morgana:enable_r_combo", true);
            morgana::e_damage_threshold = sdk::menu_api::init_int("morgana:e_damage_threshold", 20);
            
            // Ability settings
            morgana::q_hitchance = sdk::menu_api::init_int("morgana:q_hitchance", 3);
            morgana::w_hitchance = sdk::menu_api::init_int("morgana:w_hitchance", 3);
            morgana::q_range_percent = sdk::menu_api::init_int("morgana:q_range_percent", 100);
            morgana::w_range_percent = sdk::menu_api::init_int("morgana:w_range_percent", 100);
            morgana::e_range_percent = sdk::menu_api::init_int("morgana:e_range_percent", 100);
            morgana::r_range_percent = sdk::menu_api::init_int("morgana:r_range_percent", 100);

            // Drawing settings
            morgana::draw_q_range = sdk::menu_api::init_bool("morgana:draw_q_range", true);
            morgana::draw_w_range = sdk::menu_api::init_bool("morgana:draw_w_range", true);
            morgana::draw_e_range = sdk::menu_api::init_bool("morgana:draw_e_range", true);
            morgana::draw_r_range = sdk::menu_api::init_bool("morgana:draw_r_range", true);
        }

        else if (champ_name == "Veigar") {
            veigar::enable_veigar = sdk::menu_api::init_bool("veigar:enable_script", true);
            veigar::enable_q_combo = sdk::menu_api::init_bool("veigar:enable_q_combo", true);
            veigar::enable_w_combo = sdk::menu_api::init_bool("veigar:enable_w_combo", true);
            veigar::enable_e_combo = sdk::menu_api::init_bool("veigar:enable_e_combo", true);
            veigar::enable_r_combo = sdk::menu_api::init_bool("veigar:enable_r_combo", true);
            
            // Lane Clear settings
            veigar::enable_q_laneclear = sdk::menu_api::init_bool("veigar:enable_q_laneclear", true);
            veigar::enable_w_laneclear = sdk::menu_api::init_bool("veigar:enable_w_laneclear", true);

            // Harass settings
            veigar::enable_q_harass = sdk::menu_api::init_bool("veigar:enable_q_harass", true);
            veigar::enable_w_harass = sdk::menu_api::init_bool("veigar:enable_w_harass", true);

            // Last Hit settings
            veigar::enable_q_lasthit = sdk::menu_api::init_bool("veigar:enable_q_lasthit", true);

            // Ability Settings
            veigar::q_hitchance = sdk::menu_api::init_int("veigar:q_hitchance", 3); // Default high
            veigar::w_hitchance = sdk::menu_api::init_int("veigar:w_hitchance", 3); // Default high
            veigar::e_hitchance = sdk::menu_api::init_int("veigar:e_hitchance", 3); // Default high
            
            // Range Settings (default 100%)
            veigar::q_range = sdk::menu_api::init_int("veigar:q_range", 85);
            veigar::w_range = sdk::menu_api::init_int("veigar:w_range", 85);
            veigar::e_range = sdk::menu_api::init_int("veigar:e_range", 85);
            veigar::r_range = sdk::menu_api::init_int("veigar:r_range", 100);

            // Drawing Settings
            veigar::draw_q_range = sdk::menu_api::init_bool("veigar:draw_q_range", true);
            veigar::draw_w_range = sdk::menu_api::init_bool("veigar:draw_w_range", true);
            veigar::draw_e_range = sdk::menu_api::init_bool("veigar:draw_e_range", true);
            veigar::draw_r_range = sdk::menu_api::init_bool("veigar:draw_r_range", true);
        }

        else if (champ_name == "Aurora") {
            aurora::enable_q = sdk::menu_api::init_bool("aurora:enable_q", true);
            aurora::enable_e = sdk::menu_api::init_bool("aurora:enable_e", true);
            aurora::enable_q_multihit = sdk::menu_api::init_bool("aurora:enable_q_multihit", true);
            aurora::q_hitchance = sdk::menu_api::init_int("aurora:q_hitchance", 2);
            aurora::e_hitchance = sdk::menu_api::init_int("aurora:e_hitchance", 1);
            aurora::low_hp_threshold = sdk::menu_api::init_int("aurora:low_hp_threshold", 2);
            aurora::multihit_wait_time = sdk::menu_api::init_int("aurora:multihit_wait_time", 1);
        }

        drawings::watermark_pos_x = sdk::menu_api::init_int("drawing:watermark_x", 10);
        drawings::watermark_pos_y = sdk::menu_api::init_int("drawing:watermark_y", 10);
        drawings::hotkey_status_pos_x = sdk::menu_api::init_int("drawing:hotkey_x", 10);
        drawings::hotkey_status_pos_y = sdk::menu_api::init_int("drawing:hotkey_y", 30);

        }
    

    sdk::register_plugin(
        "supamegascript!",
        {
            {sdk::EventType::OnUpdate, on_update_callback},
            {sdk::EventType::OnDraw, on_draw},
            {sdk::EventType::OnUpdateMenu, on_update_menu},
        }
    );
    return TRUE;
}