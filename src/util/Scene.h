#pragma once
#include "Serializable.h"
#include "../EntityManager.h"
#include <vector>
#include <iostream>

class Scene : public Serializable {
public:
    void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) override {
        writer.StartObject();
        writer.Key("assets");
        AssetManager::GetInstance().serialize(writer);
        writer.Key("entities");
        writer.StartObject();
        for (auto entity : EntityManager::GetInstance().getEntities()) {
            entity.get()->serialize(writer);
        }
        writer.EndObject();
        writer.EndObject();
    }

    void deserialize(const rapidjson::Value& value) override {
        for (auto it = value["entities"].MemberBegin(); it != value["entities"].MemberEnd(); it++) {
            auto entity = EntityManager::GetInstance().addEntity(it->name.GetString());
            entity->deserialize(it->value);
        }
        EntityManager::GetInstance().update(); // maybe remove 
    }

};