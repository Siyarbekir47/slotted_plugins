#include "kayle.hpp"
#include "../data/champion_data.h"

namespace kayle {
    bool enable_q = true;
    bool enable_w = true;
    bool enable_e = true;
    bool enable_r = true;

    int hitchance_q = 1;

    bool draw_q = true;
    bool draw_w = true;
    bool draw_e = true;
    bool draw_r = true;

    static float last_q_time = 0.f;

    void create_menu() {
            sdk::menu_api::create_separator("Kayle Settings");
            sdk::menu_api::create_checkbox("Enable Q", &enable_q);
            sdk::menu_api::create_combo_box(
                "Q Hitchance", &hitchance_q, sdk::menu_api::hitchance_names, std::size(sdk::menu_api::hitchance_names)
            );
            sdk::menu_api::create_checkbox("Enable E - AA Reset", &enable_e);
            sdk::menu_api::create_separator("Drawings");
            sdk::menu_api::create_checkbox("Draw Q Range", &draw_q);
        }






    void handle_kayle_q(sdk::Object* local) {
            if (!enable_q) {
                return;
            }

            float current_time = sdk::get_time();
            if (current_time - last_q_time <= 0.4f) {
                return;
            }

            auto q_spell = local->get_spell_slot(sdk::ESpellSlot::q);
            if (!q_spell || !q_spell->is_ready()) {
                return;
            }

        if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::combo || sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::harass) {
                constexpr float range = champion_data::kayle::q_range;
                constexpr float width = champion_data::kayle::q_width;
                constexpr float speed = champion_data::kayle::q_speed;
                constexpr float delay = 0.25f;
                constexpr float extend = champion_data::kayle::q_extend;

                auto target = sdk::target_selector::get_primary_target();
                if (!target || local->position().dist_to(target->position()) > range) {
                    return;
                }

                auto pred = sdk::prediction::predict_skillshot(target, range, speed, width, delay, local->position(), true);

                if (!pred.valid) {
                    return;
                }

                sdk::Vec3 dir = (pred.position - local->position()).normalize();
                bool blocked = false;
                sdk::Vec3 block_point;

                for (auto minion: sdk::object_manager::get_enemy_minions()) {
                    if (!minion || minion->is_dead()) {
                        continue;
                    }

                    sdk::Vec3 to_minion = minion->position() - local->position();
                    float dist_along_line = to_minion.dot(dir);
                    float dist_to_line = fabs(to_minion.cross(dir).length());

                    if (dist_along_line > 0 && dist_along_line < range && dist_to_line < width) {
                        blocked = true;
                        block_point = minion->position();

                        sdk::Vec3 to_target = target->position() - block_point;
                        float proj = to_target.dot(dir);

                        if (proj > 0 && proj <= extend) {
                            if (pred.valid && static_cast<int>(pred.hitchance) >= hitchance_q) {
                                if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, pred.position)) {
                                    last_q_time = current_time;
                                    return;
                                }
                            }
                        }

                        return;
                    }
                }

                if (!blocked) {
                    if (pred.valid && static_cast<int>(pred.hitchance) >= hitchance_q) {
                        if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::q, pred.position)) {
                            last_q_time = current_time;
                        }
                    }
                }
        }

    }

    void handle_kayle_e(sdk::Object* local) {
        if (!enable_e) {
            return;
        }
        auto e_slot = local->get_spell_slot(sdk::ESpellSlot::e);
        if (!e_slot || !e_slot->is_ready()) {
            return;
        }
        if (sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::combo || sdk::orbwalker::get_mode() == sdk::EOrbwalkerMode::harass) {
            auto target = sdk::target_selector::get_primary_target();
            if (!target || local->position().dist_to(target->position()) > local->attack_range()) {
                return;
            }

            if (sdk::spellbook::cast_spell(sdk::ESpellSlot::e)) {}
        }

    }



    void on_update(sdk::Object* local) {
        if (!local || local->is_dead()) {
            return;
        }
        handle_kayle_q(local);
        handle_kayle_e(local);

    }

    void on_draw(sdk::Object* local) {
        if (!local) {
            return;
        }
        if (draw_q) {
            sdk::render::circle_3d(
                local->position(), sdk::Color::red(), champion_data::kayle::q_range, 2, 64, 2.f, 360.f, sdk::Vec3(0, 0, 1)
            );
        }

    }
} // namespace kayle
