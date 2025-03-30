#include "karthus.hpp"
#include <unordered_map>

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
        static const sdk::SmallStr hitchances[] = {"Low", "Medium", "High", "Very High"};

        if (sdk::menu_api::create_sub_menu("Karthus Settings", true)) {
            if (sdk::menu_api::create_sub_menu("Q Settings", true)) {
                sdk::menu_api::create_checkbox("Enable Q", &enable_q);
                sdk::menu_api::create_combo_box("q Hitchance", &hitchance_q, hitchances, 4);
                sdk::menu_api::end_sub_menu();
            }

            if (sdk::menu_api::create_sub_menu("W Settings", true)) {
                sdk::menu_api::create_checkbox("Enable W", &enable_w);
                sdk::menu_api::end_sub_menu();
            }

            if (sdk::menu_api::create_sub_menu("E Settings", true)) {
                sdk::menu_api::create_checkbox("Enable E", &enable_e);
                sdk::menu_api::end_sub_menu();
            }

            if (sdk::menu_api::create_sub_menu("R Settings", true)) {
                sdk::menu_api::create_checkbox("not implementet yet", &enable_r);
                sdk::menu_api::end_sub_menu();
            }


            if (sdk::menu_api::create_sub_menu("Drawings", true)) {
                sdk::menu_api::create_checkbox("Enable Q - Drawing", &draw_q);
                sdk::menu_api::create_checkbox("Enable W - Drawing", &draw_w);
                sdk::menu_api::create_checkbox("Enable E - Drawing", &draw_e);
                sdk::menu_api::create_checkbox("Enable R Killinfo", &draw_r);
                sdk::menu_api::end_sub_menu();
            }

            sdk::menu_api::end_sub_menu();
        }
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

    //combo
    void handle_q_combo(sdk::Object* local) {

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



        // instead of is_ready() check Q_Charges
        if (q_slot && q_slot->charges() > 1) {
            auto target = sdk::target_selector::get_primary_target();

            if (target) {
                constexpr float Q_RANGE = 875.f;
                constexpr float Q_RADIUS = 160.f;
                constexpr float Q_DELAY = 1.f;
                constexpr float Q_SPEED = 0;

                float distance = local->position().dist_to(target->position());
                if (distance <= Q_RANGE) {
                    auto prediction =
                        sdk::prediction::predict_skillshot(target, Q_RANGE, Q_SPEED, Q_RADIUS, Q_DELAY, local->position(), false);
                    if (prediction.valid && static_cast<int>(prediction.hitchance) >= hitchance_q) {
                        if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, prediction.position)) {
                            last_q_time = current_time;
                            q_is_casting = true;
                        }
                    }
                }
            }
        }
    }

    //combo
    void handle_w_combo(sdk::Object* local) {
        if (!enable_w) {
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
    //combo
    void handle_e_combo(sdk::Object* local) {
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

        bool is_e_active = false;
        std::vector<sdk::BuffInstance*> allBuffs;
        local->get_buff_manager()->get_all(&allBuffs);
        for (auto buff: allBuffs) {
            if (buff->get_name() == "KarthusDefile") {
                is_e_active = true;
                break;
            }
        }

        if (!(e_slot && (e_slot->is_ready() || is_e_active))) {
            return;
        }

        constexpr float E_RANGE = 550.f;
        bool has_enemy_nearby = false;
        for (auto enemy: sdk::object_manager::get_enemy_heroes()) {
            if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                continue;
            }
            if (local->position().dist_to(enemy->position()) <= E_RANGE) {
                has_enemy_nearby = true;
                break;
            }
        }

        static float lastEnemySeenTime = sdk::get_time();
        static bool last_enemy_state = false;

        if (has_enemy_nearby) {
            lastEnemySeenTime = sdk::get_time();
        }

        if (has_enemy_nearby != last_enemy_state) {
            if (has_enemy_nearby && !is_e_active) {
                sdk::spellbook::cast_spell(sdk::ESpellSlot::e); 
                last_e_time = current_time;
            }
            last_enemy_state = has_enemy_nearby;
        }

        // deactivate again if no enemy in range for 2s
        if (!has_enemy_nearby && is_e_active) {
            if (sdk::get_time() - lastEnemySeenTime > 2.0f) {
                sdk::spellbook::cast_spell(sdk::ESpellSlot::e); 
                lastEnemySeenTime = sdk::get_time();
                last_e_time = current_time;
            }
        }
    }

    void handle_e_other(sdk::Object* local) {

        float current_time = sdk::get_time();
        static float last_e_time_lc = 0.0f;
        auto e_slot = local->get_spell_slot(sdk::ESpellSlot::e);
        if (!enable_e) {
            return;
        }
        if (!e_slot || !e_slot->is_ready() || e_slot->level() <= 0) {
            return;
        }
        if (current_time - last_e_time_lc <= 0.4f) {
            return;
        }
        if (local->mana() < e_slot->manacost()) {
            return;
        }

        constexpr float E_RANGE = 550.f;
        int target_count = 0;
        for (auto enemy: sdk::object_manager::get_enemy_minions()) {
            if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
                continue;
            }
            if ((enemy->is_lane_minion() || enemy->is_jungle_monster()) && local->position().dist_to(enemy->position()) <= E_RANGE) {
                target_count++;
            }
        }
        bool is_e_active = false;
        std::vector<sdk::BuffInstance*> buffs;
        local->get_buff_manager()->get_all(&buffs);
        for (auto buff: buffs) {
            if (buff->get_name() == "KarthusDefile") {
                is_e_active = true;
                break;
            }
        }
        if (target_count >= 3) {
            if (!is_e_active) {
                sdk::spellbook::cast_spell(sdk::ESpellSlot::e);
                last_e_time_lc = current_time;

            }
        } else {
            if (is_e_active) {
                sdk::spellbook::cast_spell(sdk::ESpellSlot::e);
                last_e_time_lc = current_time;

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
                if (buff->get_name() == "KarthusDeathDefiedBuff") {
                    canCast = true;
                    break;
                }
            }
            if (!canCast) {
                return;
            }
        }
        if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::combo) {
            handle_q_combo(local);
            handle_w_combo(local);
            handle_e_combo(local);

        }

        if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::laneclear) {
            handle_e_other(local);
        }


    }


    //TODO: add laneclear Q, lasthit Q, add killsteal Q
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
