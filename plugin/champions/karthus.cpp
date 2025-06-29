#include "karthus.hpp"
#include <unordered_map>

// TODO: add killsteal Q, FIX E(dead enemys & when not in combo anymore still disable)

namespace karthus {


   // Config variables for menu settings
    bool enable_q = true; // Enable/disable abilities
    bool enable_w = true;
    bool enable_e = true;
    bool enable_r = true;
        
    // Hitchance settings for skillshots
    int hitchance_q = 1;


    float last_w_time = 0.f;
    float last_e_time = 0.f;

    bool draw_q = true;
    bool draw_w = true;
    bool draw_e = true;
    bool draw_r = true;

    void create_menu() {
            sdk::menu_api::create_separator("Karthus Settings");
            sdk::menu_api::create_checkbox("Enable Q", &enable_q);
            sdk::menu_api::create_combo_box(
                "Q Hitchance", &hitchance_q, sdk::menu_api::hitchance_names, std::size(sdk::menu_api::hitchance_names)
            );
            sdk::menu_api::create_checkbox("Enable W", &enable_w);
            sdk::menu_api::create_checkbox("Enable E", &enable_e);
            //sdk::menu_api::create_checkbox("Enable R ", &enable_r);

            sdk::menu_api::create_separator("Drawings");

            sdk::menu_api::create_checkbox("Draw Q Range", &draw_q);
            sdk::menu_api::create_checkbox("Draw W Range", &draw_w);
            sdk::menu_api::create_checkbox("Draw E Range", &draw_e);
            sdk::menu_api::create_checkbox("Show R Kill-Info", &draw_r);
        }



    static float calculate_r_damage(sdk::Object* local, int r_level) {
        const std::vector<float> r_base_damage = {0.f, 200.f, 350.f, 500.f};
        return r_base_damage[r_level] + 0.7f * local->ability_power();
    }
    static void draw_r_killable() {
        auto local = sdk::object_manager::get_local();
        if (!local) {
            return;
        }

        auto r_slot = local->get_spell_slot(sdk::ESpellSlot::r);
        if (!r_slot) {
            return;
        }
        if (r_slot->level() <= 0) {
            return;
        }

        float r_damage = calculate_r_damage(local, r_slot->level());
        float currentTime = sdk::get_time();

        static std::unordered_map<uint32_t, float> lastVisibleTimes;

        auto enemies = sdk::object_manager::get_enemy_heroes();
        std::vector<sdk::Object*> killable;
        for (auto enemy: enemies) {
            if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                continue;
            }

            bool visible = !enemy->is_invisible();
            if (visible) {
                lastVisibleTimes[enemy->network_id()] = currentTime;
            }

            float lastVisible = 0.f;
            if (lastVisibleTimes.find(enemy->network_id()) != lastVisibleTimes.end()) {
                lastVisible = lastVisibleTimes[enemy->network_id()];
            }

            if (currentTime - lastVisible > 8.0f) {
                continue;
            }

            if (enemy->health() <= r_damage) {
                killable.push_back(enemy);
            }
        }

        if (killable.empty()) {
            return;
        }

        sdk::Vec2 screen_size = sdk::utils::get_screen_size();
        const int icon_size = 100;
        const int spacing = 10;
        int total_width = static_cast<int>(killable.size()) * icon_size + (static_cast<int>(killable.size()) - 1) * spacing;
        sdk::Vec2 start_pos((screen_size.x - total_width) / 2, 200);

        sdk::Vec2 text_pos(start_pos.x, start_pos.y - 30);
        sdk::render::text(text_pos, "Press Ultimate to Kill:", 30, sdk::Color::red(), true);

        for (size_t i = 0; i < killable.size(); i++) {
            sdk::Object* enemy = killable[i];
            sdk::Vec2 icon_pos = start_pos + sdk::Vec2(i * (icon_size + spacing), 0);
            void* texture = enemy->get_square_texture();
            sdk::render::texture(icon_pos, sdk::Vec2(icon_size, icon_size), texture);
        }
    }
    static float calculate_q_damage_lc(sdk::Object* local) {
        const std::vector<float> q_base_damage = {0.f, 40.f, 59.f, 78.f, 97.f, 116.f};
        auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
        int q_level = q_slot ? q_slot->level() : 0;
        return q_base_damage[q_level] + 0.35f * local->ability_power();
    }
    bool handle_q_enemy(sdk::Object* local, float Q_RANGE, float Q_SPEED, float Q_RADIUS, float Q_DELAY, auto target) {
        if (target) {
            float distance = local->position().dist_to(target->position());
            if (distance <= Q_RANGE) {
                auto prediction = sdk::prediction::predict_skillshot(target, Q_RANGE, Q_SPEED, Q_RADIUS, Q_DELAY, local->position(), false);
                if (prediction.valid && static_cast<int>(prediction.hitchance) >= hitchance_q) {
                    if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, prediction.position)) {
                        sdk::utils::console_debug_message("WOOOW", "Q", sdk::Color::green());
                        return true;
                    }
                }
            }
        }
        return false;
    }
    bool handle_q_lasthit(sdk::Object* local, float Q_RANGE, float Q_DELAY_MIN) {
        bool casted = false;
        auto minions = sdk::object_manager::get_enemy_minions();
        for (auto minion: minions) {
            if (!minion || minion->is_dead() || minion->team() == local->team() || !minion->is_lane_minion() || !minion->is_jungle_monster()) {
                continue;
            }
            if (local->position().dist_to(minion->position()) > Q_RANGE) {
                continue;
            }

            float q_damage = calculate_q_damage_lc(local);
            float predicted_health = sdk::prediction::predict_minion_health(minion, Q_DELAY_MIN);
            if (predicted_health <= q_damage) {
                auto predictedPosOpt = sdk::prediction::simple_predict(minion, Q_DELAY_MIN);
                if (!predictedPosOpt.has_value()) {
                    continue;
                }
                auto predictedPos = predictedPosOpt.value();
                if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, predictedPos)) {
                    casted = true;
                    break;
                }
            }
        }
        return casted;
    }
    void handle_q(sdk::Object* local) {

        static float last_q_time = 0.0f;
        static float lastAttackTime = 0.0f;
        static bool q_is_casting = false;
        float current_time = sdk::get_time();
        auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
        if (!enable_q || current_time - last_q_time <= 0.4f) {
            return;
        }
        if (!q_slot || q_slot->level() == 0 || !q_slot->is_ready()) {
            return;
        }
        if (q_is_casting && current_time - last_q_time < 0.4f) {
            return;
        } else if (q_is_casting) {
            q_is_casting = false;
        }
        if (local->mana() < q_slot->manacost()) {
            return;
        }
        if (sdk::orbwalker::is_winding_up() || (sdk::get_time() - lastAttackTime < 0.08f)) {
            return;
        }
        constexpr float Q_RANGE = 875.f;
        constexpr float Q_RADIUS = 160.f;
        constexpr float Q_DELAY = 1.f;
        constexpr float Q_SPEED = 0;
        constexpr float Q_DELAY_MIN = 0.53f;

        // instead of is_ready() check Q_Charges
        if (q_slot && q_slot->charges() > 1) {
            //modes
            if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::combo) {
                auto target = sdk::target_selector::get_primary_target();
                q_is_casting = handle_q_enemy(local, Q_RANGE, Q_SPEED, Q_RADIUS, Q_DELAY, target);
                if (q_is_casting) {
                    last_q_time = current_time;
                }
            }
            if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::harass) {

                bool casted = handle_q_lasthit(local, Q_RANGE, Q_DELAY_MIN);
                if (casted){
                    last_q_time = current_time;
                } else {
                    auto target = sdk::target_selector::get_primary_target();
                    q_is_casting = handle_q_enemy(local, Q_RANGE, Q_SPEED, Q_RADIUS, Q_DELAY, target);
                    if (q_is_casting) {
                        last_q_time = current_time;
                    }
                }
            }
            if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::lasthit) {
                if (handle_q_lasthit(local, Q_RANGE, Q_DELAY_MIN)) {
                    last_q_time = current_time;
                }
            }
            if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::laneclear) {
                bool casted = false;
                if (handle_q_lasthit(local, Q_RANGE, Q_DELAY_MIN)) {
                    casted = true;
                } else {
                    sdk::Object* bestMinion = nullptr;
                    float lowestHealth = std::numeric_limits<float>::max();
                    auto minions = sdk::object_manager::get_enemy_minions();
                    for (auto minion: minions) {
                        if (!minion || minion->is_dead() || minion->team() == local->team()) {
                            continue;
                        }
                        if (!minion->is_lane_minion() && !minion->is_jungle_monster()) {
                            continue;
                        }
                        float dist = local->position().dist_to(minion->position());
                        if (dist > Q_RANGE) {
                            continue;
                        }


                        if (minion->health() < lowestHealth) {
                            lowestHealth = minion->health();
                            bestMinion = minion;
                        }
                    }

                    if (bestMinion) {
                        auto predictedPosOpt = sdk::prediction::simple_predict(bestMinion, Q_DELAY_MIN);
                        if (predictedPosOpt.has_value()) {
                            auto predictedPos = predictedPosOpt.value();
                            if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, predictedPos)) {
                                casted = true;
                            }
                        }
                    }
                }
                if (casted) {
                    last_q_time = current_time;
                    q_is_casting = true;
                    return;
                }
            }


        }
    }
    void handle_w(sdk::Object* local) {
        if (!enable_w) {
            return;
        }
        auto mode = sdk::orbwalker::get_mode();
        if (mode != sdk::EOrbwalkerMode::combo) {
            return;
        }
        auto w_slot = local->get_spell_slot(sdk::ESpellSlot::w);
        float current_time = sdk::get_time();
        if (w_slot->level() <= 0) {
            return;
        }
        if (current_time - last_w_time <= 0.4f) {
            return; // Prevent spam casting
        }
        if (local->mana() < w_slot->manacost()) {
            return;
        }


        if (w_slot && w_slot->is_ready()) {
            auto target = sdk::target_selector::get_primary_target();
            if (target) {
                constexpr float W_RANGE = 1000.f;
                constexpr float W_DELAY = 0.25f;

                float distance = local->position().dist_to(target->position());
                if (distance <= W_RANGE) {
                    auto prediction = sdk::prediction::predict_skillshot(target, W_RANGE, 0.f, 0.f, W_DELAY, local->position(), false);
                    if (prediction.valid && prediction.hitchance >= sdk::EHitchance::medium) {
                        auto cast_pos = prediction.position.extend(local->position(), 100.f);
                        sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::w, cast_pos);
                        last_w_time = current_time;
                    }
                }
            }
        }
    }
    bool is_enemy_nearby_e(sdk::Object* local,float E_RANGE) {
        bool has_enemy_nearby = false;
        for (auto enemy: sdk::object_manager::get_enemy_heroes()) {
            if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                continue;
            }
            if (local->position().dist_to(enemy->position()) <= E_RANGE) {
                has_enemy_nearby = true;
                return has_enemy_nearby;
            }
        }
        return has_enemy_nearby;
    }
    bool is_minion_nearby_e(sdk::Object* local, float E_RANGE) {
        bool has_minion_nearby = false;
        for (auto minion: sdk::object_manager::get_enemy_minions()) {
            if (!minion || minion->is_dead() || minion->team() == local->team()) {
                continue;
            }
            if (!minion->is_lane_minion() && !minion->is_jungle_monster()) {
                continue;
            }
            float dist = local->position().dist_to(minion->position());
            if (dist > E_RANGE) {
                continue;
            }
            has_minion_nearby = true;
        }

        return has_minion_nearby;
    }
    void handle_e(sdk::Object* local) {
        if (!enable_e) {
            return;
        }
        float current_time = sdk::get_time();
        auto e_slot = local->get_spell_slot(sdk::ESpellSlot::e);
        if (!e_slot || !e_slot->is_ready() || e_slot->level() <= 0) {
            return;
        }
        if (current_time - last_e_time <= 0.4f) {
            return; // Prevent spam casting
        }

        if (local->mana() < e_slot->manacost()) {
            return;
        }
        auto mode = sdk::orbwalker::get_mode();
        constexpr float E_RANGE = 550.f;
        bool is_e_active = false;
        std::vector<sdk::BuffInstance*> allBuffs;
        local->get_buff_manager()->get_all(&allBuffs);
        for (auto buff: allBuffs) {
            if (buff->_get_name() == "KarthusDefile") {
                is_e_active = true;
                break;
            }
        }

  /*      if (!(e_slot && (e_slot->is_ready() || is_e_active))) {
            return;
        }*/
        if (mode == sdk::EOrbwalkerMode::recalling) {
            return;
        }

        bool has_enemy_nearby = is_enemy_nearby_e(local, E_RANGE);
        bool has_minion_nearby = is_minion_nearby_e(local, E_RANGE);

        static float lastEnemySeenTime = sdk::get_time();
        static bool last_enemy_state = false;

        if (has_enemy_nearby) {
            lastEnemySeenTime = sdk::get_time();
        }
        if (mode == sdk::EOrbwalkerMode::combo) {
            if (has_enemy_nearby != last_enemy_state) {
                if (has_enemy_nearby && !is_e_active) {
                    sdk::spellbook::cast_spell(sdk::ESpellSlot::e);
                    last_e_time = current_time;
                    is_e_active = true;
                }
                last_enemy_state = has_enemy_nearby;
            }
            if (!has_enemy_nearby && is_e_active) {
                if (sdk::get_time() - lastEnemySeenTime > 2.0f) {
                    sdk::spellbook::cast_spell(sdk::ESpellSlot::e);
                    is_e_active = false;
                    lastEnemySeenTime = sdk::get_time();
                    last_e_time = current_time;
                }
            }
        }
        if (mode == sdk::EOrbwalkerMode::laneclear) {
            int target_count = 0;
            for (auto enemy: sdk::object_manager::get_enemy_minions()) {
                if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                    continue;
                }
                if ((enemy->is_lane_minion() || enemy->is_jungle_monster()) && local->position().dist_to(enemy->position()) <= E_RANGE) {
                    target_count++;
                }
            }
            if (target_count >= 3) {
                if (!is_e_active) {
                    sdk::spellbook::cast_spell(sdk::ESpellSlot::e);
                    is_e_active = true;
                    last_e_time = current_time;
                   
                }
            }


        }
        // deactivate again if no enemy in range for 2s
        if (!has_enemy_nearby && !has_minion_nearby && is_e_active) {
            if (sdk::get_time() - lastEnemySeenTime > 2.0f) {
                sdk::spellbook::cast_spell(sdk::ESpellSlot::e);
                is_e_active = false;
                lastEnemySeenTime = sdk::get_time();
                last_e_time = current_time;
            }
        }
    }
    void on_update(sdk::Object* local) {
        if (!local) {
            return;
        }
        if (local->is_dead()) {
            bool canCast = false;
            std::vector<sdk::BuffInstance*> buffs;
            local->get_buff_manager()->get_all(&buffs);
            for (auto buff: buffs) {
                if (buff->_get_name() == "KarthusDeathDefiedBuff") {
                    canCast = true;
                    break;
                }
            }
            if (!canCast) {
                return;
            }
        }
        if (sdk::orbwalker::get_mode() != sdk::EOrbwalkerMode::none && sdk::orbwalker::get_mode() != sdk::EOrbwalkerMode::flee &&
            sdk::orbwalker::get_mode() != sdk::EOrbwalkerMode::freeze && sdk::orbwalker::get_mode() != sdk::EOrbwalkerMode::recalling) {
            handle_q(local);
            handle_w(local);
        }
        handle_e(local);

    }
    void on_draw(sdk::Object* local) {
        if (!local) {
            return;
        }
        const auto pos = local->position();
        //const auto screen_pos = sdk::world_to_screen(pos);
        //if (!screen_pos) {
        //    return;
        //}
        if (draw_q) {
            sdk::render::circle_3d(pos, sdk::Color::green(), 875.f, 2, 64, 2.f, 360.f, sdk::Vec3(0, 0, 1));
        }
        if (draw_w) {
            sdk::render::circle_3d(pos, sdk::Color::blue(), 1000.f, 2, 64, 2.f, 360.f, sdk::Vec3(0, 0, 1));
        }
        if (draw_e) {
            sdk::render::circle_3d(pos, sdk::Color::white(), 550.f, 2, 64, 2.f, 360.f, sdk::Vec3(0, 0, 1));
        }
        if (draw_r) {
            draw_r_killable();
        }
    }
} 
