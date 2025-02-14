/**
 * File: CName.h
 *
 *  The CName class is a specialized component that stores the name of an entity within a game's entity-component-system (ECS).
 */

#pragma once

#include "Component.h"

class CName : public Component {
 public:
    DECLARE_COMPONENT_NAME("Name");
    std::string name;
    CName() : name("Default") {}
    CName(const std::string& n) : name(n) {}

    void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) override {
        writer.StartObject();
        writer.Key("name");
        writer.String(name.c_str());
        writer.EndObject();
    }

    void deserialize(const rapidjson::Value& value) override { name = value["name"].GetString(); }
};