// karthus.hpp
#pragma once
#include "../sdk.hpp"

namespace karthus {

    // settings
    extern bool enable_q;
    extern bool enable_w;
    extern bool enable_e;
    extern bool enable_r;

    // Menu
    void create_menu();

    // logic
    void on_update(sdk::Object* local);

    // draw
    void on_draw(sdk::Object* local);

}
