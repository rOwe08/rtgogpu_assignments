#pragma once

#include <memory>
#include <vector>
#include <ranges>
#include <string>

#include "scene_object.hpp"

class SimpleScene {
public:
	void addObject(std::shared_ptr<SceneObject> aNewObject) {
		mObjects.push_back(aNewObject);
	}

	auto getObjects() const {
		return mObjects | std::views::transform([](const auto &aPtr) -> const SceneObject& { return *aPtr; });
	}

	std::shared_ptr<SceneObject> getObject(const std::string& name) const {
		for (const auto& obj : mObjects) {
			if (obj->getName() == name) {
				return obj;
			}
		}
		return nullptr;
	}

protected:
	std::vector<std::shared_ptr<SceneObject>> mObjects;
};

