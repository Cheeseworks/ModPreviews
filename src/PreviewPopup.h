#pragma once

#include <Geode/Geode.hpp>

namespace cw::previews {
    class PreviewPopup final : public geode::Popup {
    private:
        uint8_t m_page = 1;
        uint8_t m_size = 1;

        std::string m_url;

        geode::LazySprite* m_currentImage = nullptr;
        geode::Label* m_imageCount = nullptr;

        std::unordered_map<uint8_t, geode::Ref<geode::LazySprite>> m_sprites;

    protected:
        void onPrev(CCObject* sender);
        void onNext(CCObject* sender);

        bool init(uint8_t page, uint8_t size, std::string url);

    public:
        static PreviewPopup* create(uint8_t page, uint8_t size, std::string url);

        void showImage(uint8_t page);

        void onLoad(geode::LazySprite* spr);
    };
};