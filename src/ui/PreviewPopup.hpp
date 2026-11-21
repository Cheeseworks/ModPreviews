#pragma once

#include <rhatt/ui/nodes/WebSprite.hpp>

#include <Geode/ui/Style.hpp>

namespace cw::previews::ui {
    class PreviewPopup final : public geode::GeodePopup {
    private:
        uint8_t m_page = 1;
        uint8_t m_size = 1;

        std::string m_url;

        cw::ui::WebSprite* m_currentImage = nullptr;
        geode::Label* m_imageCount = nullptr;

        std::unordered_map<uint8_t, geode::Ref<cw::ui::WebSprite>> m_sprites;

    protected:
        void onPrev(CCObject* sender);
        void onNext(CCObject* sender);

        bool init(uint8_t page, uint8_t size, std::string url);

    public:
        static PreviewPopup* create(uint8_t page, uint8_t size, std::string url);

        void showImage(uint8_t page);

        void onLoad(cw::ui::WebSprite* spr);
    };
};