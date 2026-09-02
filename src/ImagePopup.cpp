#include "ImagePopup.h"

#include "Macros.h"

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace cw::previews;

bool ImagePopup::init(uint8_t page, uint8_t size, std::string url) {
    m_page = page;
    m_size = size;
    m_url = std::move(url);

    IS_GEODE_THEME(auto geodeTheme);

    if (!Popup::init(380.f, 250.f, geodeTheme ? "geode.loader/GE_square01.png" : "GJ_square01.png")) return false;

    setTitle("Preview");
    setCloseButtonSpr(CircleButtonSprite::createWithSpriteFrameName("geode.loader/close.png", .85f, (geodeTheme ? CircleBaseColor::DarkPurple : CircleBaseColor::Green)));

    auto prevButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"), this, menu_selector(ImagePopup::onPrev));
    prevButton->setPosition({-20.f, m_buttonMenu->getContentHeight() / 2});

    auto nextArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    nextArrow->setFlipX(true);

    auto nextButton = CCMenuItemSpriteExtra::create(nextArrow, this, menu_selector(ImagePopup::onNext));
    nextButton->setPosition({m_buttonMenu->getContentWidth() + 20.f, m_buttonMenu->getContentHeight() / 2.f});

    m_buttonMenu->addChild(prevButton, 1);
    m_buttonMenu->addChild(nextButton, 1);

    m_imageCount = Label::create(fmt::format("Image {}/{}", m_page, m_size), "goldFont.fnt");
    m_imageCount->setAnchorPoint({1, 1});
    m_imageCount->setScale(0.4f);
    m_imageCount->setPosition({m_mainLayer->getContentWidth() - 8.f, m_mainLayer->getContentHeight() - 8.f});

    m_mainLayer->addChild(m_imageCount);

    showImage(page);

    return true;
};

void ImagePopup::showImage(uint8_t page) {
    if (m_currentImage) m_currentImage->removeFromParent();

    std::string previewURL = fmt::format("{}{}.png", m_url, page);

    LazySprite* spr = nullptr;

    if (auto sprite = m_sprites[page]) {
        spr = sprite;
        onLoad(spr);
    } else {
        spr = LazySprite::create({100.f, 50.f});
        m_sprites[page] = spr;

        spr->setLoadCallback([this, spr](Result<> res) {
            if (res.isOk()) onLoad(spr);
        });

        spr->loadFromUrl(previewURL);
    };

    m_currentImage = spr;

    m_mainLayer->addChild(spr);
};

void ImagePopup::onLoad(LazySprite* spr) {
    auto maxWidth = 340.f;
    auto maxHeight = 210.f;

    auto const originalSize = spr->getContentSize();

    auto scaleX = maxWidth / originalSize.width;
    auto scaleY = maxHeight / originalSize.height;

    auto scale = std::min(scaleX, scaleY);

    spr->setScale(scale);
    spr->setPosition(m_mainLayer->getContentSize() / 2.f);
    spr->setPositionY(spr->getPositionY() - 10.f);

    auto imageSize = spr->getScaledContentSize() + CCSize{6.f, 6.f};

    m_imageCount->setText(fmt::format("Image {}/{}", m_page, m_size));
};

void ImagePopup::onPrev(CCObject* sender) {
    m_page--;
    if (m_page < 1) m_page = m_size;

    showImage(m_page);
};

void ImagePopup::onNext(CCObject* sender) {
    m_page++;
    if (m_page > m_size) m_page = 1;

    showImage(m_page);
};

ImagePopup* ImagePopup::create(uint8_t page, uint8_t size, std::string url) {
    auto ret = new ImagePopup();
    if (ret->init(page, size, std::move(url))) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};