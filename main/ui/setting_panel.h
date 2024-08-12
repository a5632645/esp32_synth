#pragma once

#include "../gui/component.h"

class SettingPanel : public Component {
public:
    SettingPanel();

    void OnEventGet(const MyEvent& e) override;
    void Resized() override;
    void DrawSelf(MyGraphic& g) override;
private:
    class OptionComponent : public Component {
    public:
        OptionComponent(uint8_t* value, std::string_view title, std::vector<std::string_view> options) :
            value_(value), title_(title), options_(std::move(options)) {}

        void DrawSelf(MyGraphic& g) override;
        void OnEventGet(const MyEvent& e) override;
    private:
        uint8_t* value_ = nullptr;
        const std::string_view title_;
        const std::vector<std::string_view> options_;
    };

    void HandleEvent(const MyEvent& e);

    uint16_t curr_setting_index = 0;
    std::vector<OptionComponent> comps_;
};