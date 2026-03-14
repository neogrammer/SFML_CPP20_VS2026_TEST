#ifndef ENTITY_H__
#define ENTITY_H__
#include <memory>
#include <vector>
#include <utility>
#include "Component.h"
class Entity
{
	std::vector<std::shared_ptr<Component>> mComponents;

public:
	void addComponent(std::shared_ptr<Component> component);
	template <typename T>
	std::shared_ptr<T> getComponent();
};


#endif