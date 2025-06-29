#pragma once
#include "../sdk.hpp" 

namespace mundo {
    extern bool enable_q;
    extern bool enable_w;
    extern bool enable_e;
    extern bool enable_r;
    extern int hitchance_q;

    extern bool draw_q;
    extern bool draw_w;
    extern bool draw_e;
    extern bool draw_r;

    void create_menu();
    void on_update(sdk::Object* local);
    void on_draw(sdk::Object* local);
} 
