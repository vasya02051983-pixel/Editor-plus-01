#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/Notification.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

using namespace geode::prelude;

static void toast(std::string const& text, NotificationIcon icon = NotificationIcon::Success) {
	Notification::create(text, icon, 1.f)->show();
}

class $modify(EditorPlusUI, EditorUI) {

	bool init(LevelEditorLayer* layer) {
		if (!EditorUI::init(layer)) return false;
		if (!Mod::get()->getSettingValue<bool>("show-panel")) return true;

		auto winSize = CCDirector::get()->getWinSize();

		auto menu = CCMenu::create();
		menu->setID("editor-plus-menu"_spr);
		menu->setPosition({ winSize.width / 2, winSize.height - 20.f });
		menu->setScale(0.55f);
		menu->setContentSize({ 600.f, 40.f });
		menu->setLayout(RowLayout::create()->setGap(6.f)->setAutoScale(false));

		auto add = [&](char const* label, SEL_MenuHandler cb) {
			auto spr = ButtonSprite::create(label, "bigFont.fnt", "GJ_button_04.png", .8f);
			auto btn = CCMenuItemSpriteExtra::create(spr, this, cb);
			menu->addChild(btn);
		};

		add("Align X",  menu_selector(EditorPlusUI::onAlignX));
		add("Align Y",  menu_selector(EditorPlusUI::onAlignY));
		add("Snap",     menu_selector(EditorPlusUI::onSnap));
		add("Spread X", menu_selector(EditorPlusUI::onDistributeX));
		add("Same ID",  menu_selector(EditorPlusUI::onSelectSameID));

		menu->updateLayout();
		this->addChild(menu, 100);
		return true;
	}

	std::vector<GameObject*> currentSelection() {
		std::vector<GameObject*> out;
		auto arr = this->getSelectedObjects();
		if (!arr) return out;
		for (auto obj : CCArrayExt<GameObject*>(arr)) {
			out.push_back(obj);
		}
		return out;
	}

	void onAlignX(CCObject*) {
		auto sel = currentSelection();
		if (sel.size() < 2) return toast("Выберите минимум 2 объекта", NotificationIcon::Warning);

		float target = sel[0]->getPositionX();
		for (auto obj : sel) {
			this->moveObject(obj, { target - obj->getPositionX(), 0.f });
		}
		toast("Выровнено по X");
	}

	void onAlignY(CCObject*) {
		auto sel = currentSelection();
		if (sel.size() < 2) return toast("Выберите минимум 2 объекта", NotificationIcon::Warning);

		float target = sel[0]->getPositionY();
		for (auto obj : sel) {
			this->moveObject(obj, { 0.f, target - obj->getPositionY() });
		}
		toast("Выровнено по Y");
	}

	void onSnap(CCObject*) {
		auto sel = currentSelection();
		if (sel.empty()) return toast("Ничего не выбрано", NotificationIcon::Warning);

		float grid = static_cast<float>(Mod::get()->getSettingValue<double>("grid-size"));
		auto snap = [grid](float v) {
			return std::round((v - grid / 2.f) / grid) * grid + grid / 2.f;
		};

		for (auto obj : sel) {
			auto pos = obj->getPosition();
			this->moveObject(obj, { snap(pos.x) - pos.x, snap(pos.y) - pos.y });
		}
		toast("Привязано к сетке");
	}

	void onDistributeX(CCObject*) {
		auto sel = currentSelection();
		if (sel.size() < 3) return toast("Выберите минимум 3 объекта", NotificationIcon::Warning);

		std::sort(sel.begin(), sel.end(), [](GameObject* a, GameObject* b) {
			return a->getPositionX() < b->getPositionX();
		});

		float first = sel.front()->getPositionX();
		float last  = sel.back()->getPositionX();
		float step  = (last - first) / static_cast<float>(sel.size() - 1);

		for (size_t i = 1; i + 1 < sel.size(); ++i) {
			float target = first + step * static_cast<float>(i);
			this->moveObject(sel[i], { target - sel[i]->getPositionX(), 0.f });
		}
		toast("Распределено равномерно");
	}

	void onSelectSameID(CCObject*) {
		auto sel = currentSelection();
		if (sel.empty()) return toast("Выберите объект-образец", NotificationIcon::Warning);

		int id = sel[0]->m_objectID;
		auto result = CCArray::create();

		for (auto obj : CCArrayExt<GameObject*>(m_editorLayer->m_objects)) {
			if (obj->m_objectID == id) result->addObject(obj);
		}

		this->selectObjects(result, true);
		toast(fmt::format("Выбрано объектов: {}", result->count()));
	}
};
