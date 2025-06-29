#pragma once

namespace champion_data {
    namespace karthus {
        constexpr float q_range = 875.f;
        constexpr float q_delay = 1.0f;
        constexpr float q_radius = 160.f;
        constexpr float q_ap_ratio = 0.35f;
        constexpr float q_speed = 0;
        constexpr float q_delay_min = 0.53f;
        
    } 

    namespace kayle {
        constexpr float q_range = 900.f;
        constexpr float q_mana_cost[] = {70.f,75.f,80.f,85.f,90.f};
        constexpr float q_width = 150.f;
        constexpr float q_speed = 1600.f;
        constexpr float q_extend = 400.f;

        constexpr float w_target_range = 900.f;
        constexpr float w_cast_time = 0.25;
        constexpr float w_mana_cost[] = {70.f,75.f,80.f,85.f,90.f};
        
        constexpr float e_cast_time = 0.f;
        constexpr float e_radius = 350.f;
        
        constexpr float r_mana_cost[] = {100.f,50.f,0.f};
        constexpr float r_cast_time = 0.5f;
        constexpr float r_target_range = 900.f;
        constexpr float r_radius[] = {675.f,675.f,775.f};
        
    }

    namespace mundo {
        constexpr float q_range = 1050.f;
        constexpr float q_width = 120.f;
        constexpr float q_speed = 2000.f;
        constexpr float q_delay = 0.25f;
        constexpr float q_health_cost[] = {50.f, 60.f, 70.f, 80.f, 90.f};
        constexpr float q_min_damage[] = {80.f, 130.f, 180.f, 230.f, 280.f};
        constexpr float q_max_monster_damage[] = {350.f, 425.f, 500.f, 575.f, 650.f};

        constexpr float w_radius = 325.f;
        constexpr float w_cooldown[] = {17.f, 16.5f, 16.f, 15.5f, 15.f};
        constexpr float w_health_cost = 0.08f; // 8% current health
        constexpr float w_damage_tick[] = {5.f, 8.75f, 12.5f, 16.25f, 20.f};
        constexpr float w_recast_damage[] = {20.f, 35.f, 50.f, 65.f, 80.f}; // On detonate + 7% bonus HP

        constexpr float e_range = 650.f;
        constexpr float e_bonus_range = 50.f;
        constexpr float e_radius = 155.f; // Small AoE 
        constexpr float e_health_cost[] = {10.f, 25.f, 40.f, 55.f, 70.f};
        constexpr float e_cooldown[] = {9.f, 8.25f, 7.5f, 6.75f, 6.f};

        constexpr float r_cooldown = 120.f;
        constexpr float r_duration = 10.f;
        constexpr float r_bonus_ms[] = {0.15f, 0.25f, 0.35f};              // 15%/25%/35%
        constexpr float r_bonus_regen[] = {0.10f, 0.20f, 0.30f};           // 10%/20%/30% max health regen per second
        constexpr float r_increased_base_health[] = {0.15f, 0.20f, 0.25f}; // % missing HP as base HP
        // At R3: +5% pro enemy champion in 1200 units

        // Passive
        constexpr float passive_cd_start = 60.f;
        constexpr float passive_cd_end = 15.f;        // scales with level
        constexpr float passive_regen_start = 0.004f; // 0.4% max HP per 5s
        constexpr float passive_regen_end = 0.023f;   // 2.3% max HP per 5s
        constexpr float passive_anti_cc_cost = 0.04f; // 4% current HP

        // Misc
        constexpr float base_as = 0.67f;
        constexpr float attack_range = 125.f;
        constexpr float move_speed = 345.f;
    }

  
}
