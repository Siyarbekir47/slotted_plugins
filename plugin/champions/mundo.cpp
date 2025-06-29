#include "mundo.hpp"
#include "../sdk.hpp"
#include "../data/champion_data.h"

namespace mundo {

    bool enable_q = true;
    bool enable_w = true;
    bool enable_e = true;
    bool enable_r = true;

    extern int ult_hp_threshold = 20; 
    extern int ult_enemy_count = 1; 

    int hitchance_q = 1;

    bool draw_q = true;
    bool draw_w = true;


    void create_menu() {
        sdk::menu_api::create_separator("Mundo Settings");
        sdk::menu_api::create_checkbox("Enable Q", &enable_q);
        sdk::menu_api::create_combo_box(
            "Q Hitchance", &hitchance_q, sdk::menu_api::hitchance_names, std::size(sdk::menu_api::hitchance_names)
        );
        sdk::menu_api::create_checkbox("Enable W", &enable_w);
        sdk::menu_api::create_checkbox("Enable E", &enable_e);
        sdk::menu_api::create_separator("R Settings");
        sdk::menu_api::create_checkbox("Enable R", &enable_r);
        sdk::menu_api::create_int_slider("Ult HP Threshold (%)", &ult_hp_threshold, 10, 80);
        sdk::menu_api::create_int_slider("Enemies in AA Range", &ult_enemy_count, 1, 5);

        sdk::menu_api::create_separator();

        sdk::menu_api::create_checkbox("Draw Q Range", &draw_q);
        sdk::menu_api::create_checkbox("Draw W Range", &draw_w);
    }


    float get_q_health_cost(sdk::Object* local) {
        auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
        int q_level = q_slot ? q_slot->level() : 1;
        return champion_data::mundo::q_health_cost[q_level - 1];
    }

    float calculate_q_damage(sdk::Object* local, sdk::Object* minion) {
        auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
        int q_level = q_slot ? q_slot->level() : 1;
        float current_hp_dmg = minion->health() * (0.2f + (0.025f * (q_level - 1))); // 20/22.5/25/27.5/30%
        float min_dmg = champion_data::mundo::q_min_damage[q_level - 1];
        float dmg = std::max(current_hp_dmg, min_dmg);

        // Jungle Monster Cap
        if (minion->is_jungle_monster()) {
            float monster_cap = champion_data::mundo::q_max_monster_damage[q_level - 1];
            dmg = std::min(dmg, monster_cap);
        }
        return dmg;
    }

    bool handle_q_minion_lasthit(sdk::Object* local, bool lasthit_only_out_of_range = true) {
        static float last_q_time = 0.f;
        float now = sdk::get_time();
        auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
        if (!enable_q || !q_slot || !q_slot->is_ready() || q_slot->level() <= 0) {
            return false;
        }
        if (now - last_q_time < 0.4f) {
            return false;
        }
        float health_cost = get_q_health_cost(local);
        if (local->health() <= health_cost + 35.f) {
            return false;
        }

        auto minions = sdk::object_manager::get_enemy_minions();
        for (auto minion: minions) {
            if (!minion || minion->is_dead() || minion->team() == local->team()) {
                continue;
            }
            if (!minion->is_lane_minion() && !minion->is_jungle_monster()) {
                continue;
            }
            float dist = local->position().dist_to(minion->position());
            if (dist > champion_data::mundo::q_range) {
                continue;
            }
            if (lasthit_only_out_of_range && dist < local->attack_range()) {
                continue;
            }

            float travel = champion_data::mundo::q_delay + (dist / champion_data::mundo::q_speed);
            float pred_hp = sdk::prediction::predict_minion_health(minion, travel);
            float dmg = calculate_q_damage(local, minion);

            if ((!lasthit_only_out_of_range || (pred_hp > 0 && pred_hp * 1.1f <= dmg))) {
                auto pred = sdk::prediction::simple_predict(minion, travel);
                if (!pred.has_value()) {
                    continue;
                }
                if (sdk::prediction::is_minion_in_line(
                        local->position(), pred.value(), champion_data::mundo::q_width, minion->network_id()
                    )) {
                    continue;
                }
                if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, pred.value())) {
                    last_q_time = now;
                    return true;
                }
            }
        }
        return false;
    }


    bool handle_q_champ(sdk::Object* local) {
        static float last_q_time = 0.f;
        float now = sdk::get_time();

        auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
        if (!enable_q || !q_slot || !q_slot->is_ready() || q_slot->level() <= 0) {
            return false;
        }

        if (now - last_q_time < 0.4f) {
            return false;
        }

        float health_cost = get_q_health_cost(local);
        if (local->health() <= health_cost + 35.f) {
            return false; // Safety Buffer
        }

        auto target = sdk::target_selector::get_primary_target();
        if (!target || target->is_dead() || target->team() == local->team()) {
            return false;
        }

        float dist = local->position().dist_to(target->position());
        if (dist > champion_data::mundo::q_range) {
            return false;
        }

        auto pred = sdk::prediction::predict_skillshot(
            target, champion_data::mundo::q_range, champion_data::mundo::q_speed, champion_data::mundo::q_width,
            champion_data::mundo::q_delay, local->position(), false
        );

        if (!pred.valid) {
            return false;
        }

        if (sdk::prediction::is_minion_in_line(local->position(), pred.position, champion_data::mundo::q_width, target->network_id())) {
            return false;
        }

        if (static_cast<int>(pred.hitchance) < hitchance_q) {
            return false;
        }

        bool casted = sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, pred.position);
        if (casted) {
            last_q_time = now;
        }
        return casted;
    }

    bool is_w_active(sdk::Object* local) {
        auto buffs = local->get_buff_manager();
        std::vector<sdk::BuffInstance*> buff_list;
        buffs->get_all(&buff_list);
        for (auto buff: buff_list) {
            if (buff->_get_name() == "DrMundoW") {
                return true;
            }
        }
        return false;
    }

    bool handle_w(sdk::Object* local) {
        static float last_w_time = 0.f;
        float now = sdk::get_time();
        auto w_slot = local->get_spell_slot(sdk::ESpellSlot::w);

        if (!enable_w || !w_slot || !w_slot->is_ready() || w_slot->level() <= 0) {
            return false;
        }

        if (is_w_active(local)) {
            return false;
        }

        if (now - last_w_time < 0.4f) {
            return false;
        }

        if (local->health() <= local->max_health() * 0.10f) {
            return false;
        }

        float radius = champion_data::mundo::w_radius;
        auto mode = sdk::orbwalker::get_mode();

        if (mode == sdk::EOrbwalkerMode::combo) {
            for (auto enemy: sdk::object_manager::get_enemy_heroes()) {
                if (enemy && !enemy->is_dead() && local->position().dist_to(enemy->position()) < radius) {
                    bool casted = sdk::spellbook::cast_spell(sdk::ESpellSlot::w);
                    if (casted) {
                        last_w_time = now;
                    }
                    return casted;
                }
            }
        }

        if (mode == sdk::EOrbwalkerMode::laneclear) {
            int count = 0;
            for (auto minion: sdk::object_manager::get_enemy_minions()) {
                if (minion && !minion->is_dead() && (minion->is_lane_minion() || minion->is_jungle_monster()) &&
                    local->position().dist_to(minion->position()) < radius) {
                    count++;
                    if (count >= 3) {
                        bool casted = sdk::spellbook::cast_spell(sdk::ESpellSlot::w);
                        if (casted) {
                            last_w_time = now;
                        }
                        return casted;
                    }
                }
            }
        }

        return false;
    }

    bool is_r_buff_active(sdk::Object* local) {
        auto buffs = local->get_buff_manager();
        std::vector<sdk::BuffInstance*> allBuffs;
        buffs->get_all(&allBuffs);
        for (auto buff: allBuffs) {
            if (buff && buff->_get_name() == "DrMundoR" && buff->is_active()) {
                return true;
            }
        }
        return false;
    }

    void handle_r(sdk::Object* local) {
        if (!enable_r) {
            return;
        }

        if (is_r_buff_active(local)) {
            return;
        }

        float hp_percent = (local->health() / local->max_health()) * 100.f;
        if (hp_percent > ult_hp_threshold) {
            return;
        }

        int enemy_count = 0;
        for (auto enemy: sdk::object_manager::get_enemy_heroes()) {
            if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                continue;
            }
            if (local->position().dist_to(enemy->position()) <= enemy->attack_range()) {
                enemy_count++;
            }
        }
        if (enemy_count < ult_enemy_count) {
            return;
        }

        auto r_slot = local->get_spell_slot(sdk::ESpellSlot::r);
        if (r_slot && r_slot->is_ready()) {
            sdk::spellbook::cast_spell(sdk::ESpellSlot::r);
        }
    }


    bool is_e_active(sdk::Object* local) {
        auto buffs = local->get_buff_manager();
        std::vector<sdk::BuffInstance*> buff_list;
        buffs->get_all(&buff_list);
        for (auto buff: buff_list) {
            if (buff->_get_name() == "DrMundoE") { 
                return true;
            }
        }
        return false;
    }

    float calculate_e_damage(sdk::Object* local, sdk::Object* target) {
        auto e_slot = local->get_spell_slot(sdk::ESpellSlot::e);
        int e_level = e_slot ? e_slot->level() : 1;
        float bonus_ad = 15.f + 5.f * (e_level - 1);               
        float bonus_hp = local->max_health() * 0.07f;              
        float total_e_dmg = 5.f + 10.f * (e_level - 1) + bonus_hp; 

        if (target->is_jungle_monster()) {
            total_e_dmg *= 2.0f;
        } else if (target->is_lane_minion()) {
            total_e_dmg *= 1.4f;
        }

        return total_e_dmg;
    }


    bool handle_e(sdk::Object* local) {
        static float last_e_time = 0.f;
        float now = sdk::get_time();
        auto e_slot = local->get_spell_slot(sdk::ESpellSlot::e);

        if (!enable_e || !e_slot || !e_slot->is_ready() || e_slot->level() <= 0) {
            return false;
        }

        if (is_e_active(local)) {
            return false;
        }

        if (now - last_e_time < 0.3f) {
            return false;
        }

        if (local->health() < local->max_health() * 0.10f) {
            return false;
        }

        auto mode = sdk::orbwalker::get_mode();
        float aa_range_e = local->attack_range() + 50.f; // 50 Bonus-Range

        if (mode == sdk::EOrbwalkerMode::combo) {
            for (auto enemy: sdk::object_manager::get_enemy_heroes()) {
                if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                    continue;
                }
                if (local->position().dist_to(enemy->position()) <= aa_range_e) {
                    if (sdk::spellbook::cast_spell(sdk::ESpellSlot::e)) {
                        last_e_time = now;
                        return true;
                    }
                }
            }
        }

        if (mode == sdk::EOrbwalkerMode::laneclear) {
            sdk::Object* bestTarget = nullptr;
            float bestScore = 0.f;
            for (auto minion: sdk::object_manager::get_enemy_minions()) {
                if (!minion || minion->is_dead()) {
                    continue;
                }
                if (!minion->is_lane_minion() && !minion->is_jungle_monster()) {
                    continue;
                }
                if (local->position().dist_to(minion->position()) > aa_range_e) {
                    continue;
                }
                float pred_hp = sdk::prediction::predict_minion_health(minion, 0.25f); 
                float e_dmg = calculate_e_damage(local, minion);
                bool is_big = minion->is_jungle_monster() || minion->max_health() > 1200; 

                if (pred_hp > 0 && pred_hp * 1.1f <= e_dmg) {
                    float score = is_big ? 10.f : 1.f;
                    if (score > bestScore) {
                        bestTarget = minion;
                        bestScore = score;
                    }
                }
            }
            if (bestTarget) {
                if (sdk::spellbook::cast_spell(sdk::ESpellSlot::e)) {
                    last_e_time = now;
                    return true;
                }
            }
        }

        if (mode == sdk::EOrbwalkerMode::lasthit) {
            for (auto minion: sdk::object_manager::get_enemy_minions()) {
                if (!minion || minion->is_dead() || !minion->is_lane_minion()) {
                    continue;
                }
                if (local->position().dist_to(minion->position()) > aa_range_e) {
                    continue;
                }
                float pred_hp = sdk::prediction::predict_minion_health(minion, 0.25f);
                float e_dmg = calculate_e_damage(local, minion);
                if (pred_hp > 0 && pred_hp * 1.1f <= e_dmg) {
                    if (sdk::spellbook::cast_spell(sdk::ESpellSlot::e)) {
                        last_e_time = now;
                        return true;
                    }
                }
            }
        }

        if (mode == sdk::EOrbwalkerMode::harass) {
            for (auto enemy: sdk::object_manager::get_enemy_heroes()) {
                if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                    continue;
                }
                if (local->position().dist_to(enemy->position()) <= aa_range_e) {
                    if (sdk::spellbook::cast_spell(sdk::ESpellSlot::e)) {
                        last_e_time = now;
                        return true;
                    }
                }
            }
        }

        return false;
    }





    void on_update(sdk::Object* local) {
        if (!local) {
            return;
        }

        auto mode = sdk::orbwalker::get_mode();

        if (mode == sdk::EOrbwalkerMode::combo || mode == sdk::EOrbwalkerMode::harass) {
            handle_q_champ(local);
        }

        if (mode == sdk::EOrbwalkerMode::lasthit) {
            handle_q_minion_lasthit(local, true);
        }

        if (mode == sdk::EOrbwalkerMode::laneclear) {
            handle_q_minion_lasthit(local, false);
        }

        if (mode == sdk::EOrbwalkerMode::combo || mode == sdk::EOrbwalkerMode::laneclear) {
            handle_w(local);
        }
        if (mode == sdk::EOrbwalkerMode::combo || mode == sdk::EOrbwalkerMode::laneclear) {
            handle_e(local);
        }
        if (mode == sdk::EOrbwalkerMode::combo || mode == sdk::EOrbwalkerMode::laneclear || mode == sdk::EOrbwalkerMode::flee) {
            handle_r(local);
        }
    }

    void on_draw(sdk::Object* local) {
        if (!local) {
            return;
        }
        auto pos = local->position();
        if (draw_q) {
            sdk::render::circle_3d(pos, sdk::Color::blue(), champion_data::mundo::q_range, 2, 64, 2.f, 360.f, sdk::Vec3(0, 0, 1));
        }
        if (draw_w) {
            sdk::render::circle_3d(pos, sdk::Color::green(), champion_data::mundo::w_radius, 2, 64, 2.f, 360.f, sdk::Vec3(0, 0, 1));
        }
    }

} 
