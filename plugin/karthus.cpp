// karthus.cpp
#include "karthus.hpp"

namespace karthus {

    // Standardeinstellungen
    bool enable_q = true;
    bool enable_w = true;
    bool enable_e = true;
    bool enable_r = true;


    void create_menu() {
        sdk::menu_api::create_text("Karthus Einstellungen");
        sdk::menu_api::create_new_line();
        sdk::menu_api::create_separator();
        sdk::menu_api::create_checkbox("Q aktivieren", &enable_q);
        sdk::menu_api::create_checkbox("W aktivieren", &enable_w);
        sdk::menu_api::create_checkbox("E aktivieren", &enable_e);
        sdk::menu_api::create_checkbox("R aktivieren", &enable_r);
        
    }

    void on_update(sdk::Object* local) {
        // Dummy-Logik (Beispiel)
        if (!local || local->is_dead()) {
            return;
        }
        const float time = sdk::get_time();
        static float last_move_time = 0;
        if (enable_q && local->get_spell_slot(sdk::ESpellSlot::q)->is_ready()) {
            // Q Logik hier einfügen

            if (time - last_move_time > 0.5f) {
                sdk::move_to(sdk::get_cursor());
                last_move_time = time;
                sdk::utils::console_debug_message("move", "[Karthus]", sdk::Color::black());
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
        const auto pos = local->position();
        auto pos_2d = sdk::world_to_screen(pos);
        // Beispiel Zeichnungen (z.B. Fähigkeitenreichweiten anzeigen)
        if (enable_q) {

            
        }

        if (enable_w) {
            sdk::render::circle_3d(pos, sdk::Color(0,255,0), 1000.f, 2, 64, 2.f, 0.f, sdk::Vec3(0, 0, 1));
        }

        if (enable_e) {
            sdk::render::circle_3d(pos, sdk::Color::white(), 425.f, 2, 64, 2.f, 0.f, sdk::Vec3(1, 0, 0));
        }

    if (enable_r && local->get_spell_slot(sdk::ESpellSlot::r)->is_ready()) {
            sdk::render::text(sdk::Vec2(500, 500), "Ultimate bereit!", 25, sdk::Color::red(), true);
        }
    }

} // namespace karthus
