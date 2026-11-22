// original mod by alphalaneous
// source inherited & now maintained by me !!!
#include <cw.h>

#include <rhatt/prelude.h>

#include <alphalaneous.alphas_geode_utils/Utils.hpp>
#include <alphalaneous.alphas_geode_utils/ObjectModify.hpp>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;
using namespace cw::previews;

namespace cw::previews {
    namespace main {
        static utils::StringSet& forgejo() noexcept {
            static utils::StringSet inst;
            return inst;
        }
    };
};

struct RepoData final {
    std::string rawURL;
    std::string repo;
};

class $nodeModify(PreviewsModPopup, ModPopup) {
    struct Fields final {
        utils::StringMap<async::TaskHolder<web::WebResponse>> listeners;

        std::unordered_map<uint8_t, Ref<cw::WebSprite>> previewSprites;
        std::map<uint8_t, Ref<Button>> previewButtons;

        Ref<CCNode> imagesContainer = nullptr;
        std::vector<Ref<cw::WebSprite>> sprites;

        CCNode* imagesList = nullptr;
        CCNode* showAllMenu = nullptr;

        bool hasShownImages = false;
        bool isShowingDescription = false;
        bool isShowingABanner = false;

        TaskHolder<web::WebResponse> urlTask;

        std::string url;
    };

    // as per fod's request, check everything and return to prevent side effects
    bool isSafe() {
#ifdef CW_TESTING
        log::trace("Checking if popup is safe to modify");
#endif

        if (auto self = reinterpret_cast<FLAlertLayer*>(this)) {
            if (auto githubBtn = self->m_mainLayer->getChildByIDRecursive("github")) {
                auto firstNodeOpt = alpha::utils::cocos::getChildByClassName(self->m_mainLayer, "cocos2d::CCNode", 0);
                if (!firstNodeOpt.has_value()) return false;

                if (auto url = typeinfo_cast<CCString*>(githubBtn->getUserObject("url"))) {
                    std::string githubURL = url->getCString();
                    if (githubURL.empty()) return false;
                } else {
                    log::warn("Repository URL is missing");
                    return false;
                };

                //check if sprites exist ig
                if (!CCSprite::createWithSpriteFrameName("edit_addCBtn_001.png")) return false;

                CW_IS_GEODE_THEME(auto geodeTheme);

                auto base = geodeTheme ? CircleBaseColor::DarkPurple : CircleBaseColor::Green;

                if (base == CircleBaseColor::DarkPurple && !CCSprite::createWithSpriteFrameName("baseCircle_Medium_DarkPurple.png"_gode)) return false;
                if (base == CircleBaseColor::Green && !CCSprite::createWithSpriteFrameName("baseCircle_Medium_Green.png"_gode)) return false;

#ifdef CW_TESTING
                log::debug("Popup is safe to modify, yay!!!");
#endif
                return true;
            } else {
                log::warn("GitHub button is missing");
            };
        } else {
            log::warn("Popup is not derived from FLAlertLayer");
        };

        log::warn("Popup is not safe to modify");
        return false;
    }

    void checkIfMain(ZStringView url, CopyableFunction<void(bool)>&& callback) {
        auto f = m_fields.self();

        auto req = cw::request::base();

        f->listeners[url].spawn(
            req.get(url),
            [this, callback = std::move(callback)](web::WebResponse resp) {
                callback(resp.ok());
            }
        );
    }

    void modify() {
        auto f = m_fields.self();

        if (!isSafe()) return;

        auto githubBtn = getChildByIDRecursive("github");

        f->imagesContainer = CCNode::create();
        f->imagesContainer->setID("images-container"_spr);
        f->imagesContainer->setVisible(false);
        f->imagesContainer->setContentSize({ 267.5f, 59.0f });
        f->imagesContainer->setPosition({ 152.5f, 0.0f });
        f->imagesContainer->setAnchorPoint({ 0.0f, 0.0f });

        f->imagesList = CCNode::create();
        f->imagesList->setZOrder(1);
        f->imagesList->setContentSize({ 262.0f, 54.0f });
        f->imagesList->setAnchorPoint(cw::anchor::center);
        f->imagesList->ignoreAnchorPointForPosition(false);

        f->imagesContainer->addChild(f->imagesList);

        f->showAllMenu = CCNode::create();
        f->showAllMenu->setContentSize({ 20.0f, 54.0f });
        f->showAllMenu->setPosition({ f->imagesContainer->getContentWidth() - 2.5f, f->imagesContainer->getContentHeight() / 2.0f });
        f->showAllMenu->ignoreAnchorPointForPosition(false);
        f->showAllMenu->setAnchorPoint({ 1.0f, 0.5f });
        f->showAllMenu->setZOrder(2);
        f->showAllMenu->setVisible(false);

        CW_IS_GEODE_THEME(auto geodeTheme);

        auto moreSpr = CircleButtonSprite::createWithSpriteFrameName(
            "edit_addCBtn_001.png",
            0.85f,
            (geodeTheme ? CircleBaseColor::DarkPurple : CircleBaseColor::Green)
        );

        auto showAllBtn = Button::createWithNode(moreSpr, [this](auto) { showPopup(1); });
        showAllBtn->setPosition(f->showAllMenu->getContentSize() / 2.0f);
        showAllBtn->setScale(0.4f);

        f->showAllMenu->addChild(showAllBtn);
        f->imagesContainer->addChild(f->showAllMenu);

        f->imagesList->setPosition(f->imagesContainer->getContentSize() / 2.0f);

        auto background = NineSlice::create("square02b_001.png");
        background->setContentSize(f->imagesContainer->getContentSize() / 0.5f);
        background->setScale(0.5f);
        background->setOpacity(75);
        background->setColor({ 0, 0, 0 });
        background->setPosition(f->imagesContainer->getContentSize() / 2.0f);

        f->imagesContainer->addChild(background);

        auto url = static_cast<CCString*>(githubBtn->getUserObject("url"));

        std::string sourceURL = url->getCString();
        if (sourceURL.empty()) return;

        utils::string::toLowerIP(sourceURL);

        getRepoData(sourceURL, [this, sourceURL](Result<RepoData> dataRes) {
            if (dataRes.isErr()) {
                log::error("{}", dataRes.unwrapErr());
                return;
            };

#ifdef CW_TESTING
            log::debug("Retrieved repository data for {}", sourceURL);
#endif

            auto data = std::move(dataRes).unwrap();

            auto const modJsonUrl = fmt::format("{}/main/mod.json", data.rawURL);

            checkIfMain(modJsonUrl, [this, data](bool main) {
                std::string mainBranch{ "main" };
                if (!main) mainBranch = "master";

                auto f = m_fields.self();

                f->url = fmt::format("{}/{}/previews/preview-", data.rawURL, mainBranch);

                for (uint8_t i = 1; i <= 10; i++) {
                    auto previewURL = fmt::format("{}{}.png", f->url, i);

                    auto spr = cw::WebSprite::create({ 100.0f, 50.0f });
                    f->sprites.push_back(spr);

                    spr->load(previewURL, [this, i, spr](cw::WebSprite::LoadResult res) {
                        if (res.isOk()) {
                            onImageDownloadFinish(i, spr);
                        } else {
#ifdef CW_TESTING
                            log::trace("Failed to load preview image {}: {}", i, res.unwrapErr());
#endif
                        };
                    });
                };
            });
        });
    }

    void getRepoData(ZStringView url, Function<void(Result<RepoData>)>&& callback) {
        auto split = utils::string::split(url, "://");
        if (split.size() < 2) {
            callback(Err("Invalid HTTPS URL"));
            return;
        };

        auto& postURL = split[1];
        std::string prefixWWW{ "www." };

        if (postURL.starts_with(prefixWWW)) postURL.erase(0, prefixWWW.size());

        auto splitSlash = utils::string::split(postURL, "/");
        if (splitSlash.size() < 2) {
            callback(Err("Invalid URL"));
            return;
        };

        auto& platformURL = splitSlash[0];
        auto repo = postURL.substr(platformURL.size() + 1);

        std::string rawURL;

        if (platformURL == "github.com") rawURL = fmt::format("https://raw.githubusercontent.com/{}", repo);
        if (platformURL == "gitlab.com") rawURL = fmt::format("https://gitlab.com/{}/-/raw", repo);
        if (platformURL == "codeberg.org") rawURL = fmt::format("https://codeberg.org/{}/raw/branch", repo);  // already known, hardcoded for faster fetching

        if (auto it = main::forgejo().find(platformURL); it != main::forgejo().end()) rawURL = fmt::format("https://{}/{}/raw/branch", platformURL, repo);

        if (!rawURL.empty()) return callback(
            Ok(
                RepoData{
                    .rawURL = rawURL,
                    .repo = repo,
                }
            )
        );

        auto req = cw::request::base();

        auto fullBaseURL = fmt::format("https://{}/api/v1/version", platformURL);
#ifdef CW_TESTING
        log::trace("Sending request to {} to check Git instance version", fullBaseURL);
#endif

        m_fields->urlTask.spawn(
            req.get(fullBaseURL),
            [platformURL = std::move(platformURL), repo = std::move(repo), fullBaseURL = std::move(fullBaseURL), callback = std::move(callback)](web::WebResponse res) mutable {
                if (!res.ok()) return callback(Err(fmt::format("Failed to get response from {}: {}", fullBaseURL, res.errorMessage())));

                auto jsonRes = res.json();
                if (jsonRes.isErr()) return callback(Err(fmt::format("Failed to parse JSON from {}: {}", fullBaseURL, jsonRes.unwrapErr())));

                auto json = std::move(jsonRes).unwrap();

                auto versionRes = json["version"].asString();
                if (versionRes.isErr()) return callback(Err(fmt::format("Failed to get version from {}: {}", fullBaseURL, jsonRes.unwrapErr())));

                auto version = std::move(versionRes).unwrap();

#ifdef CW_TESTING
                log::trace("Git instance version: {}", version);
#endif

                if (utils::string::contains(version, "+gitea-")) {
                    auto raw = fmt::format("https://{}/{}/raw/branch", platformURL, repo);
                    log::info("Detected Forgejo instance at {}, using {}", platformURL, raw);

                    main::forgejo().insert(std::move(platformURL));

                    return callback(
                        Ok(
                            RepoData{
                                .rawURL = raw,
                                .repo = repo,
                            }
                        )
                    );
                };

                callback(Err(fmt::format("Unsupported platform for URL: {}", fullBaseURL)));
            }
        );
    }

    void resizeForBanner(CCNode* banner) {
        resizeDescription(getChildByIDRecursive("description-container"));
        m_fields->imagesContainer->setPositionY(30.0f);
    }

    void resizeDescription(CCNode* description) {
#ifdef CW_TESTING
        log::debug("Resizing description container...");
#endif

        description->setContentHeight(192.0f);
        description->setPositionY(description->getParent()->getContentHeight() - 30.0f);
        description->setAnchorPoint({ 0.5f, 1.0f });

        if (auto textArea = typeinfo_cast<MDTextArea*>(description->getChildByID("textarea"))) {
#ifdef CW_TESTING
            log::trace("Inner text area found");
#endif

            textArea->setContentHeight(description->getContentHeight());
            textArea->setAnchorPoint({ 0.5f, 1.0f });
            textArea->setPositionY(description->getContentHeight());

            textArea->getScrollLayer()->setContentHeight(description->getContentHeight());
            textArea->getScrollLayer()->scrollToTop();

            if (auto bg = textArea->getChildByType<NineSlice>()) bg->setContentHeight(description->getContentHeight());

            textArea->updateLayout();
        };
    }

    void listenForDescription(float dt) {
        auto f = m_fields.self();

        auto description = getChildByIDRecursive("description-container");

        if (!f->isShowingDescription && description) {
            f->imagesContainer->setVisible(true);

            resizeDescription(description);

            f->isShowingDescription = true;
        }

        if (f->isShowingDescription && !description) {
            f->imagesContainer->setVisible(false);
            f->isShowingDescription = false;
        };
    }

    void listenForBanner(float dt) {
        auto f = m_fields.self();

        if (auto modtoberBanner = getChildByIDRecursive("modtober-banner")) {
            f->isShowingABanner = true;

            resizeForBanner(modtoberBanner);
            unschedule(schedule_selector(PreviewsModPopup::listenForBanner));
        };
    }

    CCSprite* createSprite(CCImage* img) {
        auto texture = new CCTexture2D();
        CCSprite* spr = nullptr;

        if (texture->initWithImage(img)) spr = CCSprite::createWithTexture(texture);
        texture->release();

        return spr;
    }

    bool showImages() {
        auto f = m_fields.self();

        if (auto self = reinterpret_cast<FLAlertLayer*>(this)) {
            auto firstNodeOpt = alpha::utils::cocos::getChildByClassName(self->m_mainLayer, "cocos2d::CCNode", 0);
            if (!firstNodeOpt.has_value() || !firstNodeOpt.value()) {
                log::error("Mod popup main container node unavailable");
                return false;
            };

            if (!f->imagesContainer) {
                log::error("Unable to show mod previews: preview container is null");
                return false;
            };

            auto firstNode = firstNodeOpt.value();
            firstNode->addChild(f->imagesContainer);

            schedule(schedule_selector(PreviewsModPopup::listenForDescription));
            schedule(schedule_selector(PreviewsModPopup::listenForBanner));

            return true;
        };

        return false;
    }

    void showPopup(uint8_t page) {
        auto f = m_fields.self();

        auto popup = PreviewPopup::create(page, f->previewSprites.size(), f->url);
        popup->show();
    }

    void onImageDownloadFinish(uint8_t id, cw::WebSprite* spr) {
        auto f = m_fields.self();

        f->previewSprites[id] = spr;

        auto scale = 54.0f / spr->getContentHeight();
        spr->setScale(scale);

        auto btn = Button::createWithNode(spr, [this, id](auto) { showPopup(id); });
        btn->ignoreAnchorPointForPosition(true);
        btn->setScaleMultiplier(1.1f);
        f->previewButtons[id] = btn;

        if (!f->hasShownImages) {
            auto shown = showImages();
            f->hasShownImages = true;

            if (!shown) return;
        };

        btn->setID(fmt::format("preview-{}", id));

        if (!f->imagesList) {
            if (!f->hasShownImages) {
                log::error("Unable to arrange mod previews: preview list is null");
                f->hasShownImages = true;
            };

            return;
        };

        f->imagesList->removeAllChildren();

        auto gap = 2.5f;
        auto totalWidth = 0.0f;

        Button* lastButton = nullptr;

        for (auto& [k, v] : f->previewButtons) {
            if (lastButton) {
                auto pos = lastButton->getPositionX() + lastButton->getContentWidth();

                if (pos + v->getContentWidth() >= 262.0f) {
                    f->showAllMenu->setVisible(true);
                    break;
                };

                v->setPositionX(pos + gap);
            };

            totalWidth += v->getContentWidth() + gap;
            lastButton = v;

            f->imagesList->addChild(v);
        };

        totalWidth -= gap;

        f->imagesList->setContentWidth(totalWidth);

        if (f->showAllMenu->isVisible()) f->imagesList->setPositionX((f->imagesContainer->getContentWidth() - f->showAllMenu->getContentWidth()) / 2);
    }
};