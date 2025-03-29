#include "karthus.hpp"

namespace karthus {

    bool enable_q = true;
    bool enable_w = true;
    bool enable_e = true;
    bool enable_r = true;
    bool draw_q = true;
    bool draw_w = true;
    bool draw_e = true;
    bool draw_r = true;

    void create_menu() {
        sdk::menu_api::create_text("Karthus - Plugin");
        sdk::menu_api::create_separator();
        sdk::menu_api::create_sub_menu("Q - Settings", false);
        sdk::menu_api::create_checkbox("Q aktivieren", &enable_q);
        sdk::menu_api::create_sub_menu("W - Settings", false);
        sdk::menu_api::create_checkbox("W aktivieren", &enable_w);
        sdk::menu_api::create_sub_menu("E - Settings", false);
        sdk::menu_api::create_checkbox("E aktivieren", &enable_e);
        sdk::menu_api::create_sub_menu("R - Settings", false);
        sdk::menu_api::create_checkbox("R aktivieren", &enable_r);
        sdk::menu_api::create_separator();
        sdk::menu_api::create_sub_menu("Drawings", false);
        sdk::menu_api::create_checkbox("Q - Drawings", &draw_q);
        sdk::menu_api::create_checkbox("W - Drawings", &draw_w);
        sdk::menu_api::create_checkbox("E - Drawings", &draw_e);
        sdk::menu_api::create_checkbox("R - Killable Info", &draw_r);
    }

    static float calculate_r_damage(sdk::Object* local, int r_level) {
        const std::vector<float> r_base_damage = {0.f, 200.f, 350.f, 500.f};
        return r_base_damage[r_level] + 0.7f * local->ability_power();
    }

    static void draw_r_killable() {
        auto local = sdk::object_manager::get_local();
        if (!local || !enable_r) {
            return;
        }

        auto r_slot = local->get_spell_slot(sdk::ESpellSlot::r);
        if (!r_slot) {
            return;
        }

        int r_level = r_slot->level();
        if (r_level <= 0) {
            return;
        }

        float r_damage = calculate_r_damage(local, r_level);

        auto enemies = sdk::object_manager::get_enemy_heroes();
        std::vector<sdk::Object*> killable;
        for (auto enemy: enemies) {
            if (!enemy || enemy->is_dead() || enemy->team() == local->team()) {
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
        const int icon_size = 50; 
        const int spacing = 10;   
        int total_width = static_cast<int>(killable.size()) * icon_size + (static_cast<int>(killable.size()) - 1) * spacing;
        sdk::Vec2 start_pos((screen_size.x - total_width) / 2, 50); // 50 upper


        sdk::Vec2 text_pos(start_pos.x, start_pos.y - 30);
        sdk::render::text(text_pos, "Press Ultimate to Kill:", 25, sdk::Color::red(), true);

        // draw 
        for (size_t i = 0; i < killable.size(); i++) {
            sdk::Object* enemy = killable[i];
            sdk::Vec2 icon_pos = start_pos + sdk::Vec2(i * (icon_size + spacing), 0);
            void* texture = enemy->get_square_texture();
            sdk::render::texture(icon_pos, sdk::Vec2(icon_size, icon_size), texture);
        }
    }


    //combo
    void handle_q(sdk::Object* local) {
        if (!enable_q) {
            return;
        }
        auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
        if (q_slot && q_slot->is_ready()) {
            auto target = sdk::target_selector::get_primary_target();
            if (target && !target->is_dead() && target->is_hero() && target->team() != local->team()) {
                constexpr float Q_RANGE = 875.f;
                constexpr float Q_RADIUS = 160.f; 
                constexpr float Q_DELAY = 1.f;
                constexpr float Q_SPEED = 0;

                float distance = local->position().dist_to(target->position());
                if (distance <= Q_RANGE) {
                    auto prediction =
                        sdk::prediction::predict_skillshot(target, Q_RANGE, Q_SPEED, Q_RADIUS, Q_DELAY, local->position(), false);
                    if (prediction.valid && prediction.hitchance >= sdk::EHitchance::medium) {
                        sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, prediction.position);
                    }
                }
            }
        }
    }
    //combo
    void handle_w(sdk::Object* local) {
        if (!enable_w) {
            return;
        }
        auto w_slot = local->get_spell_slot(sdk::ESpellSlot::w);
        if (w_slot && w_slot->is_ready()) {
            auto target = sdk::target_selector::get_primary_target();
            if (target && !target->is_dead() && target->is_hero() && target->team() != local->team()) {
                constexpr float W_RANGE = 1000.f;
                constexpr float W_DELAY = 0.25f;

                float distance = local->position().dist_to(target->position());
                if (distance <= W_RANGE) {
                    auto prediction = sdk::prediction::predict_skillshot(target, W_RANGE, 0.f, 0.f, W_DELAY, local->position(), false);
                    if (prediction.valid && prediction.hitchance >= sdk::EHitchance::medium) {
                        auto cast_pos = prediction.position.extend(local->position(), 100.f);
                        sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::w, cast_pos);
                    }
                }
            }
        }
    }
    //combo
    void handle_e(sdk::Object* local) {
        if (!enable_e) {
            return;
        }
        auto e_slot = local->get_spell_slot(sdk::ESpellSlot::e);

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
            }
            last_enemy_state = has_enemy_nearby;
        }

        // deactivate again if no enemy in range for 2s
        if (!has_enemy_nearby && is_e_active) {
            if (sdk::get_time() - lastEnemySeenTime > 2.0f) {
                sdk::spellbook::cast_spell(sdk::ESpellSlot::e); 
                lastEnemySeenTime = sdk::get_time();
            }
        }
    }







    void on_update(sdk::Object* local) {
        if (!local || local->is_dead()) {
            return;
        }
        if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::combo) {
            handle_q(local);
            handle_w(local);
            handle_e(local);
        }


    }

    void on_draw(sdk::Object* local) {
        if (!local) {
            return;
        }
        const auto pos = local->position();
        const auto screen_pos = sdk::world_to_screen(pos);
        if (!screen_pos) {
            return;
        }
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
