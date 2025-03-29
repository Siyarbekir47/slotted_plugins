#include "jhin.hpp"
//#include "altxtarget.hpp"
#include <algorithm>
#include <cmath>

namespace jhin {
    // Config variables for menu settings
    bool* enable_q = nullptr;  // Enable/disable abilities
    bool* enable_w = nullptr;
    bool* enable_e = nullptr;
    bool* enable_r = nullptr;
    bool* w_only_on_root = nullptr;  // Only use W when target is marked
    bool* w_killsteal = nullptr;     // Use W for killstealing
    bool* q_only_reload = nullptr;   // Only use Q during reload animation

    // Hitchance settings for skillshots
    int* w_hitchance = nullptr;
    int* e_hitchance = nullptr;
    int* r_hitchance = nullptr;

    // Cooldown tracking
    float last_q_time = 0.0f;
    float last_w_time = 0.0f;
    float last_e_time = 0.0f;
    float last_r_time = 0.0f;
    bool in_ult = false;  // Track if we're in R state

    // Ability ranges from leaguewiki
    const float q_range = 550.f;
    const float w_range = 2520.f;
    const float e_range = 750.f;
    const float r_range = 3500.f;

    // Base damage values for abilities
    const std::vector<float> q_damage = {0.f, 40.f, 65.f, 90.f, 115.f, 140.f};
    const std::vector<float> w_damage = {0.f, 60.f, 95.f, 130.f, 165.f, 200.f};

    // R shot visualization
    std::vector<sdk::Vec3> r_shot_positions;  // Store start/end positions of R shots
    float last_shot_time = 0.0f;
    const float SHOT_DURATION = 0.5f;  // How long to show the shot trail

    // Check if Jhin is in reload animation and get remaining duration
    bool is_reloading(sdk::Object* local) {
        auto buff_manager = local->get_buff_manager();
        if (!buff_manager) return false;

        auto reload_buff = buff_manager->get_buff(sdk::spell_hash("jhinpassivereload"));
        if (!reload_buff || !reload_buff->is_active()) return false;

        return true;
    }

    // Add this new function to get reload end time
    float get_reload_end_time(sdk::Object* local) {
        auto buff_manager = local->get_buff_manager();
        if (!buff_manager) return 0.0f;

        auto reload_buff = buff_manager->get_buff(sdk::spell_hash("jhinpassivereload"));
        if (!reload_buff || !reload_buff->is_active()) return 0.0f;

        return reload_buff->end_time();
    }

    // Calculate W damage including AD scaling
    static float calculate_w_damage(sdk::Object* local, sdk::Object* target, int spell_level) {
        float damage = w_damage[spell_level] + (local->attack_damage() * 0.5f);
        return sdk::utils::calculate_real_damage(target, damage, true);
    }

    // Get best target
    static sdk::Object* get_target(sdk::Object* local, float range) {
        if (!local) return nullptr;

            auto target = sdk::target_selector::get_primary_target();
            if (target && local->position().dist_to(target->position()) <= range && 
                !sdk::target_selector::is_bad_target(target)) {
                return target;
            }
        return nullptr;
    }

    static float calculate_q_damage(sdk::Object* local, sdk::Object* target, int spell_level) {
        float damage = q_damage[spell_level] + (local->ability_power() * 0.4f);
        return sdk::utils::calculate_real_damage(target, damage, false);
    }

    void handle_jhin_q(sdk::Object* local) {
        // Basic checks (enabled, not ulting, cooldown)
        if (!*enable_q || in_ult || sdk::get_time() - last_q_time <= 0.4f) return;
        if (!sdk::utils::is_key_down(VK_SPACE)) return;  // Only in combo mode

        auto q_spell = local->get_spell_slot(sdk::ESpellSlot::q);
        if (!q_spell || !q_spell->is_ready()) return;

        // Check reload setting
        if (*q_only_reload && !is_reloading(local)) return;

        // Find and cast on target
        auto target = get_target(local, q_range);
        if (!target) return;

        if (sdk::spellbook::cast_spell_on_target(sdk::ESpellSlot::q, target->network_id())) {
            last_q_time = sdk::get_time();
        }
    }

    // W handler - Root check and killsteal
    void handle_jhin_w(sdk::Object* local) {
        if (!*enable_w || in_ult) return;
        
        float current_time = sdk::get_time();
        if (current_time - last_w_time <= 0.4f) return;  // Prevent spam casting

        auto w_spell = local->get_spell_slot(sdk::ESpellSlot::w);
        if (!w_spell || !w_spell->is_ready()) return; // Get spell slot and check if ready

        auto target = get_target(local, w_range);
        if (!target) return; // Get target and check if valid

        // Check for Jhin's passive mark (Dancing Grenade or AA)
        if (*w_only_on_root) {
            auto buff_manager = target->get_buff_manager();
            if (!buff_manager || !buff_manager->has_buff(sdk::spell_hash("jhinespotteddebuff"))) return;
        }

        // Killsteal logic - works without combo key
        if (*w_killsteal) {
            float w_dmg = calculate_w_damage(local, target, w_spell->level());
            if (target->health() <= w_dmg) {
                // Use higher speed for killstea
                auto pred = sdk::prediction::predict_skillshot(
                    target, w_range, 5000.f, 45.f, 0.75f, local->position(), true
                );
                
                if (pred.valid && static_cast<int>(pred.hitchance) >= *w_hitchance) { // Check hitchance and if valid and cast
                    if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::w, pred.position)) {
                        last_w_time = current_time; // Set last cast time to current time to prevent spam casting
                    }
                }
                return;
            }
        }

        // Normal W cast requires combo key
        if (!sdk::utils::is_key_down(VK_SPACE)) return;

        auto pred = sdk::prediction::predict_skillshot(
            target, w_range, 5000.f, 45.f, 0.75f, local->position(), false
        );

        if (pred.valid && static_cast<int>(pred.hitchance) >= *w_hitchance) {
            if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::w, pred.position)) {
                last_w_time = current_time;
            }
        }
    }

    // E handler - Basic skillshot
    void handle_jhin_e(sdk::Object* local) {
        if (!*enable_e || in_ult || sdk::get_time() - last_e_time <= 0.4f) return;
        if (!sdk::utils::is_key_down(VK_SPACE)) return;

        auto e_spell = local->get_spell_slot(sdk::ESpellSlot::e);
        if (!e_spell || !e_spell->is_ready()) return;

        auto target = get_target(local, e_range);
        if (!target) return;

        auto pred = sdk::prediction::predict_skillshot(
            target, e_range, 0.f, 80.f, 1.25f, local->position(), false
        );

        if (pred.valid && static_cast<int>(pred.hitchance) >= *e_hitchance) {
            if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::e, pred.position)) {
                last_e_time = sdk::get_time();
            }
        }
    }

    // R handler - Cursor-based targeting
    void handle_jhin_r(sdk::Object* local) {
        if (!*enable_r) return;
        
        float current_time = sdk::get_time();
        
        // Clean up old shot trails
        r_shot_positions.erase(
            std::remove_if(r_shot_positions.begin(), r_shot_positions.end(),
                [](const sdk::Vec3& pos) {
                    return sdk::get_time() - last_shot_time > SHOT_DURATION;
                }
            ),
            r_shot_positions.end()
        );

        if (!*enable_r || current_time - last_r_time <= 0.4f) return;
        if (!sdk::utils::is_key_down(VK_SPACE)) return;

        auto r_spell = local->get_spell_slot(sdk::ESpellSlot::r);
        if (!r_spell || !r_spell->is_ready()) return;

        // Track R state by checking cast info
        auto cast_info = local->get_spell_cast_info();
        in_ult = cast_info && cast_info->slot() == 3;

        if (!in_ult) return;

        // Find closest enemy to cursor within 500 range
        auto cursor = sdk::get_cursor();
        sdk::Object* best_target = nullptr;
        float closest_distance = 99999.f;

        auto enemies = sdk::object_manager::get_enemy_heroes();
        for (const auto& enemy : enemies) {
            // Skip invalid targets
            if (!enemy || enemy->is_dead() || enemy->is_invisible() || // sdk::target_selector::is_bad_target() already checks for is dead is invisible etc
                sdk::target_selector::is_bad_target(enemy)) { // So generally u don't want to manually check for these
                continue;
            }

            // Check cursor range (500 units around cursor)
            float dist_to_cursor = enemy->position().dist_to(cursor);
            if (dist_to_cursor > 500.f) continue;

            if (dist_to_cursor < closest_distance) {
                best_target = enemy;
                closest_distance = dist_to_cursor;
            }
        }

        // Validate target and range
        if (!best_target || best_target->position().dist_to(local->position()) > r_range) return;

        // Predict with high speed for accuracy
        auto pred = sdk::prediction::predict_skillshot(
            best_target,
            r_range,
            5000.f,  // Fast projectile
            80.f,    // Width
            0.25f,   // Cast time
            local->position(),
            true     // Check range from edge
        );

        if (pred.valid && static_cast<int>(pred.hitchance) >= *r_hitchance) {
            if (sdk::spellbook::cast_spell_to_position(sdk::ESpellSlot::r, pred.position)) {
                last_r_time = current_time;
                last_shot_time = current_time;
                // Store positions for drawing trail
                r_shot_positions = {local->position(), pred.position};
            }
        }
    }

    // Draw R shots and targeting reticle
    void draw_r_shots() {
        auto local = sdk::object_manager::get_local();
        if (!local) return;

        // Draw the trail of R shots
        if (r_shot_positions.size() >= 2) {
            float time_since_shot = sdk::get_time() - last_shot_time;
            float alpha = 255.0f * (1.0f - (time_since_shot / SHOT_DURATION));  // Fade out over time
            
            if (alpha > 0) {
                const int glow_layers = 8;      // Number of layers for glow effect
                const float base_width = 5.0f;  // Core line width
                
                // Draw multiple layers of lines with decreasing alpha for glow effect
                for (int i = glow_layers; i > 0; i--) {
                    float glow_size = base_width + (float)i * 3.0f;  // Each layer gets progressively larger
                    float layer_alpha = (1.0f - (float)i / glow_layers) * alpha * 0.4f;  // Outer layers are more transparent
                    
                    // Draw purple glow layer
                    sdk::render::line_3d(
                        r_shot_positions[0],  // Shot start position
                        r_shot_positions[1],  // Shot end position
                        sdk::Color(128, 0, 128, static_cast<uint8_t>(layer_alpha)),
                        glow_size
                    );
                }

                // Create pulsing gradient effect for core line
                float progress = std::fmod(time_since_shot * 2.0f, 1.0f);
                sdk::Color gradient_color(
                    255,  // R
                    128 + static_cast<uint8_t>(127 * std::sin(progress * 6.28f)),  // Pulsing G
                    255,  // B
                    static_cast<uint8_t>(alpha)  // Fade out
                );

                // Draw core line with gradient
                sdk::render::line_3d(
                    r_shot_positions[0],
                    r_shot_positions[1],
                    gradient_color,
                    base_width
                );
            }
        }

        // Only draw targeting reticle when in R state
        auto cast_info = local->get_spell_cast_info();
        if (!cast_info || cast_info->slot() != 3) return;

        auto cursor = sdk::get_cursor();
        const int glow_layers = 8;

        // Create crosshair lines around cursor
        float line_length = 50.f;  // Length of crosshair lines
        sdk::Vec3 up = cursor + sdk::Vec3(0, 0, line_length);     // Vertical line top
        sdk::Vec3 down = cursor - sdk::Vec3(0, 0, line_length);   // Vertical line bottom
        sdk::Vec3 right = cursor + sdk::Vec3(line_length, 0, 0);  // Horizontal line right
        sdk::Vec3 left = cursor - sdk::Vec3(line_length, 0, 0);   // Horizontal line left

        // Draw glowing crosshair
        for (int i = glow_layers; i > 0; i--) {
            float glow_size = 1.0f + (float)i * 0.5f;  // Progressive line thickness
            float alpha = (1.0f - (float)i / glow_layers) * 120;  // Fade out for glow effect
            sdk::Color glow_color(128, 0, 128, static_cast<uint8_t>(alpha));  // Purple glow

            // Draw vertical and horizontal lines of crosshair
            sdk::render::line_3d(up, down, glow_color, glow_size);
            sdk::render::line_3d(left, right, glow_color, glow_size);
        }
    }

    // Create menu with all settings
    void create_jhin_menu() {
        static const sdk::SmallStr hitchances[] = {"Low", "Medium", "High", "Very High"};

        if (sdk::menu_api::create_sub_menu("Jhin Settings", true)) {
            if (sdk::menu_api::create_sub_menu("Q Settings", true)) {
                sdk::menu_api::create_checkbox("Enable Q", enable_q);
                sdk::menu_api::create_checkbox("Only Q During Reload", q_only_reload);
                sdk::menu_api::end_sub_menu();
            }

            if (sdk::menu_api::create_sub_menu("W Settings", true)) {
                sdk::menu_api::create_checkbox("Enable W", enable_w);
                sdk::menu_api::create_checkbox("Only W on Root", w_only_on_root);
                sdk::menu_api::create_checkbox("Use W Killsteal", w_killsteal);
                sdk::menu_api::create_combo_box("W Hitchance", w_hitchance, hitchances, 4);
                sdk::menu_api::end_sub_menu();
            }

            if (sdk::menu_api::create_sub_menu("E Settings", true)) {
                sdk::menu_api::create_checkbox("Enable E", enable_e);
                sdk::menu_api::create_combo_box("E Hitchance", e_hitchance, hitchances, 4);
                sdk::menu_api::end_sub_menu();
            }

            if (sdk::menu_api::create_sub_menu("R Settings", true)) {
                sdk::menu_api::create_checkbox("Enable R", enable_r);
                sdk::menu_api::create_combo_box("R Hitchance", r_hitchance, hitchances, 4);
                sdk::menu_api::end_sub_menu();
            }

            sdk::menu_api::end_sub_menu();
        }
    }

}