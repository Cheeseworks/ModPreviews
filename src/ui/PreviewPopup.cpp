#include "PreviewPopup.hpp"

#include <cw.h>

#include <rhatt/prelude.h>

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cw::previews;

bool PreviewPopup::init(uint8_t page, uint8_t size, std::string url) {
    m_page = page;
    m_size = size;
    m_url = std::move(url);

    if (!GeodePopup::init(380.0f, 250.0f)) return false;

    setTitle("Preview");

    auto prevBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"),
        this,
        menu_selector(PreviewPopup::onPrev)
    );
    prevBtn->setID("previous-btn");
    prevBtn->setPosition({ -20.0f, m_buttonMenu->getContentHeight() / 2.0f });

    auto nextArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    nextArrow->setFlipX(true);

    auto nextBtn = CCMenuItemSpriteExtra::create(
        nextArrow,
        this,
        menu_selector(PreviewPopup::onNext)
    );
    nextBtn->setID("next-btn");
    nextBtn->setPosition({ m_buttonMenu->getContentWidth() + 20.0f, m_buttonMenu->getContentHeight() / 2.0f });

    m_buttonMenu->addChild(prevBtn, 1);
    m_buttonMenu->addChild(nextBtn, 1);

    m_imageCount = Label::create(fmt::format("Image {}/{}", m_page, m_size), "goldFont.fnt");
    m_imageCount->setScale(0.425f);
    m_imageCount->setAnchorPoint({ 1, 1 });
    m_imageCount->setAlignment(Label::Alignment::Right);

    m_mainLayer->addChildAtPosition(m_imageCount, Anchor::TopRight, { -7.5f, -7.5f }, false);

    showImage(page);

    return true;
}

void PreviewPopup::showImage(uint8_t page) {
    if (m_currentImage) m_currentImage->removeFromParent();

    std::string previewURL = fmt::format("{}{}.png", m_url, page);

    cw::WebSprite* spr;

    if (auto sprite = m_sprites[page]) {
        spr = sprite;
        onLoad(spr);
    } else {
        spr = cw::WebSprite::create({ 100.0f, 50.0f });
        m_sprites[page] = spr;

        spr->load(previewURL, [this, spr](cw::WebSprite::LoadResult res) {
            if (res.isOk()) onLoad(spr);
        });
    };

    m_currentImage = spr;

    m_mainLayer->addChild(spr);
}

void PreviewPopup::onLoad(cw::WebSprite* spr) {
    auto maxWidth = 340.0f;
    auto maxHeight = 210.0f;

    auto const originalSize = spr->getContentSize();

    auto scaleX = maxWidth / originalSize.width;
    auto scaleY = maxHeight / originalSize.height;

    auto scale = std::min(scaleX, scaleY);

    spr->setScale(scale);
    spr->setPosition(m_mainLayer->getContentSize() / 2.0f);
    spr->setPositionY(spr->getPositionY() - 10.0f);

    auto imageSize = spr->getScaledContentSize() + CCSize{ 6.0f, 6.0f };

    m_imageCount->setText(fmt::format("Image {}/{}", m_page, m_size));
}

void PreviewPopup::onPrev(CCObject* sender) {
    m_page--;
    if (m_page < 1) m_page = m_size;

    showImage(m_page);
}

void PreviewPopup::onNext(CCObject* sender) {
    m_page++;
    if (m_page > m_size) m_page = 1;

    showImage(m_page);
}

PreviewPopup* PreviewPopup::create(uint8_t page, uint8_t size, std::string url) {
    auto ret = new PreviewPopup();
    if (ret->init(page, size, std::move(url))) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
}