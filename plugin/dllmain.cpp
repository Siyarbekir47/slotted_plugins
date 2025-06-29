// dllmain.cpp - SiyarAIO Project mit Karthus
#define SDK_IMPLEMENTATION
#include "sdk.hpp"
#include "champions/karthus.hpp"
#include "champions/kayle.hpp"


namespace script {
    static bool siyarAIO_enabled = true;

    void create_master_menu() {
            sdk::menu_api::create_sub_menu(
                "[SiyarAIO Settings:]",
                []() {
                    sdk::menu_api::create_checkbox("[SiyarAIO] Active:", &siyarAIO_enabled);
                    sdk::menu_api::create_new_line();
                    sdk::menu_api::create_separator();

                    auto local = sdk::object_manager::get_local();
                    if (local && local->object_name() == "Karthus") {
                        karthus::create_menu();
                    }
                    if (local && local->object_name() == "Kayle") {
                        kayle::create_menu();
                    }
                },
                "Activate and configure SiyarAIO"
            );
        }


} // namespace script

static void on_update_callback(void*) {
    if (!script::siyarAIO_enabled) return;

    auto local = sdk::object_manager::get_local();

    if (!local || local->is_dead() && local->object_name() != "Karthus") {
        return;
    }
    if (local->object_name() == "Karthus") {
        karthus::on_update(local);
    }
    if (local->object_name() == "Kayle") {
        kayle::on_update(local);
    }


}

static void on_draw(void*) {
    if (!script::siyarAIO_enabled) return;

    auto local = sdk::object_manager::get_local();
    if (!local) return;

    if (local->object_name() == "Karthus") {
        karthus::on_draw(local);
    }
    if (local->object_name() == "Kayle") {
        kayle::on_draw(local);
    }
}

static void on_update_menu(void*) {
    script::create_master_menu();
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, sdk::FnLoadFunction load_function) {
    if (reason == DLL_PROCESS_ATTACH) {
        sdk::init(load_function);

        sdk::register_plugin(
            "SiyarAIO",
            {
                {sdk::EventType::OnUpdate, on_update_callback},
                {sdk::EventType::OnDraw, on_draw},
                {sdk::EventType::OnUpdateMenu, on_update_menu},
            }
        );
    }
    return TRUE;
}