#pragma once

#include <string_view>
#include <array>
#include <functional>
#include "gui/component.h"
#include "keyboard_panel.h"
#include "setting_panel.h"
#include "debug_panel.h"

class TablePanel : public Component {
public:
    TablePanel();

    void OnEventGet(const MyEvent& e) override;
    void DrawAboveChild(MyGraphic& g) override;
    void Resized() override;

    KeyboardPanel& GetKeyboardPanel() { return keyboard_; }
private:
    friend class TableTabBar;

    class TableTabBar : public Component {
    public:
        static constexpr std::array titles_ {
            std::string_view{ "setting" },
            std::string_view{ "tracker" },
            std::string_view{ "debug" }
        };

        void DrawSelf(MyGraphic& g) override;
        void OnEventGet(const MyEvent& e) override;
    private:
        uint16_t curr_select_index = 0;
    };

    void InternalSwitchComponent(uint16_t index);
    void HandleEvent(const MyEvent& e);

    TableTabBar tab_bar_;
    KeyboardPanel keyboard_;
    uint16_t comp_select_index = 0;
    SettingPanel setting_panel_;
    DebugPanel debug_panel_;
};