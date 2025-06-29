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

  
}
