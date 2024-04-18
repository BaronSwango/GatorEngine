/*
* File:  CTouchTrigger
*
* eeeeeeee
*/

#ifndef TOUCH_TRIGGER_H
#define TOUCH_TRIGGER_H

#include "../ActionBus.h"
#include "Component.h"

class CTouchTrigger : public Component {
public:
	DECLARE_COMPONENT_NAME("Touch Trigger");

	std::string tag = "";
	TriggerAction action = TriggerAction::None;
	Vec2 trigger_size = Vec2(50, 50);

	CTouchTrigger() {}

	void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) override {
        writer.StartObject();

        writer.Key("tag");
        writer.String(tag.c_str());

        writer.Key("action");
        writer.Int(static_cast<int>(action));

        writer.Key("triggerSize");
        writer.StartObject();
        writer.Key("x");
        writer.Double(trigger_size.x);
        writer.Key("y");
        writer.Double(trigger_size.y);
        writer.EndObject();
	}

	void deserialize(const rapidjson::Value& value) override {
        tag = value["tag"].GetString();
        action = static_cast<TriggerAction>(value["action"].GetInt());
        const rapidjson::Value& size = value["triggerSize"];
        trigger_size.x = size["x"].GetDouble();
        trigger_size.y = size["y"].GetDouble();

	}
};

#endif // TOUCH_TRIGGER_H