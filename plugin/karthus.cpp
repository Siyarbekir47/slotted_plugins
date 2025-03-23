// karthus.cpp
#include "karthus.hpp"

namespace karthus {

    // Standardeinstellungen
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
        sdk::menu_api::create_checkbox("R - Drawings", &draw_r);
        
    }

    void on_update(sdk::Object* local) {
        // Dummy-Logik (Beispiel)
        if (!local || local->is_dead()) {
            return;
        }
        const float time = sdk::get_time();
        static float last_move_time = 0;
        if (enable_q) {
            auto q_slot = local->get_spell_slot(sdk::ESpellSlot::q);
            if (q_slot->is_ready()) {
                // Hol dir primäres Ziel automatisch (z.B. aus dem SDK target_selector)
                auto target = sdk::target_selector::get_primary_target();
                if (target && !target->is_dead() && target->is_hero() && target->team() != local->team()) {
                    // Q Parameter von Karthus (laut LoL):
                    constexpr float Q_RANGE = 875.f;    // range 
                    constexpr float Q_RADIUS = 80.f;    // wiki radius/2
                    constexpr float Q_DELAY = 0.25f;    // windup
                    constexpr float Q_SPEED = 0; // Instant

                    auto distance = local->position().dist_to(target->position());

                    if (distance <= Q_RANGE) {
                        // Prediction verwenden, um die Position vorherzusagen
                        auto prediction =
                            sdk::prediction::predict_skillshot(target, Q_RANGE, Q_SPEED, Q_RADIUS, Q_DELAY, local->position(), false);

                        // Prüfen der Treffergenauigkeit (mindestens hoch)
                        if (prediction.hitchance >= sdk::EHitchance::medium && prediction.valid) {
                            sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, prediction.position);
                            sdk::utils::console_debug_message("Casting Q on predicted position!", "[Karthus-Q]", sdk::Color::green());
                        }
                    }
                }
            }
        }

        if (enable_w && local->get_spell_slot(sdk::ESpellSlot::w)->is_ready()) {
            // W Logik hier einfügen
        }

        if (enable_e && local->get_spell_slot(sdk::ESpellSlot::e)->is_ready()) {
            // E Logik hier einfügen
        }

        if (enable_r && local->get_spell_slot(sdk::ESpellSlot::r)->is_ready()) {
            // R Logik hier einfügen
        }
    }

    void on_draw(sdk::Object* local) {
        if (!local) {
            return;
        }
        // showing the range of the spells

        const auto pos = local->position();
        // get screen position of the player
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

        if (draw_r && local->get_spell_slot(sdk::ESpellSlot::r)->is_ready()) {
            sdk::render::text(sdk::Vec2(pos.x, pos.y), "Ultimate bereit!", 25, sdk::Color::red(), true);
        }
    }

} // namespace karthus
