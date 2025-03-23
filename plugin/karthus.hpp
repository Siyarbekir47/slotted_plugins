// karthus.hpp
#pragma once
#include "sdk.hpp"

namespace karthus {

    // Einstellungen
    extern bool enable_q;
    extern bool enable_w;
    extern bool enable_e;
    extern bool enable_r;

    // Menü-Erstellung
    void create_menu();

    // Spiellogik
    void on_update(sdk::Object* local);

    // Zeichnungen
    void on_draw(sdk::Object* local);

} // namespace karthus
