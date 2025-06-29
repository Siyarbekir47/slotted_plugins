#pragma once
#include "../sdk.hpp"

namespace kayle {
    extern bool enable_q;
    extern bool enable_w;
    extern bool enable_e;
    extern bool enable_r;

    void create_menu();
    void on_update(sdk::Object* local);
    void on_draw(sdk::Object* local);

} 