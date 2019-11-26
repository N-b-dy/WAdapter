#ifndef W_PROPERTY_H
#define W_PROPERTY_H

#include <Arduino.h>
#include "WJson.h"

enum WPropertyType {
	BOOLEAN, DOUBLE, INTEGER, LONG, BYTE, STRING
};

enum WPropertyVisibility {
	ALL, NONE, MQTT, WEBTHING
};

union WPropertyValue {
	bool asBoolean;
	double asDouble;
	int asInteger;
	unsigned long asLong;
	byte asByte;
	//String* asString;
	char* string;
};

class WProperty {
public:
	typedef std::function<void(WProperty* property)> TOnPropertyChange;

	WProperty(String id, String title, String description, WPropertyType type) {
		initialize(id, title, description, type, (type == STRING ? 32 : 0));
	}

	WProperty(String id, String title, String description, WPropertyType type, byte length) {
		initialize(id, title, description, type, length);
	}

	~WProperty() {
		if(this->value.string) {
		    delete[] this->value.string;
		}
	}

	void setOnValueRequest(TOnPropertyChange onValueRequest) {
		this->onValueRequest = onValueRequest;
	}

	void setOnChange(TOnPropertyChange onChange) {
		this->onChange = onChange;
	}

	void setDeviceNotification(TOnPropertyChange deviceNotification) {
		this->deviceNotification = deviceNotification;
	}

	String getId() {
		return id;
	}

	void setId(String id) {
		this->id = id;
	}

	String getTitle() {
		return title;
	}

	void setTitle(String title) {
		this->title = title;
	}

	String getDescription() {
		return description;
	}

	void setDescription(String description) {
		this->description = description;
	}

	WPropertyType getType() {
		return type;
	}

	byte getLength() {
		return length;
	}

	void setType(WPropertyType type) {
		this->type = type;
	}

	String getAtType() {
		return atType;
	}

	/*void setAtType(String atType) {
		this->atType = atType;
	}*/

	bool isNull() {
		return (this->valueNull);
	}

	bool isRequested() {
		return (this->requested);
	}

	void setRequested(bool requested) {
		if ((requested) && (!isNull())) {
			this->requested = true;
		} else {
			this->requested = false;
		}
	}

	void setNull() {
		this->valueNull = true;
	}

	bool parse(String value) {
		if ((!isReadOnly()) && (value != nullptr)) {
			switch (getType()) {
			case BOOLEAN: {
				setBoolean(value.equals("true"));
				return true;
			}
			case DOUBLE: {
				setDouble(value.toDouble());
				return true;
			}
			case INTEGER: {
				setInteger(value.toInt());
				return true;
			}
			case LONG: {
				setLong(value.toInt());
				return true;
			}
			case BYTE: {
				setByte(value.toInt());
				return true;
			}
			case STRING:
				setString(value);
				return true;
			}
		}
		return false;
	}

	/*void setFromJson(JsonVariant value) {
		if ((!isReadOnly()) && (value != nullptr)) {
			switch (getType()) {
			case BOOLEAN: {
				setBoolean(value.as<bool>());
				break;
			}
			case DOUBLE: {
				setDouble(value.as<double>());
				break;
			}
			case INTEGER: {
				setInteger(value.as<int>());
				break;
			}
			case BYTE: {
				setByte(value.as<byte>());
				break;
			}
			case STRING:
				setString(value.as<String>());
				break;
			}
		}
	}*/



	bool getBoolean() {
		requestValue();
		return (!this->valueNull ? this->value.asBoolean : false);
	}

	void setBoolean(bool newValue) {
		if (type != BOOLEAN) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asBoolean != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asBoolean = newValue;
			this->setValue(valueB);
		}
	}

	void toggleBoolean() {
		if (type != BOOLEAN) {
			return;
		}
		setBoolean(!getBoolean());
	}

	double getDouble() {
		requestValue();
		return (!this->valueNull ? this->value.asDouble : 0.0);
	}

	bool isEqual(double a, double b, double precision) {
        double diff = a - b;
        return ((diff < precision) && (-diff < precision));
    }

	void setDouble(double newValue) {
		if (type != DOUBLE) {
			return;
		}
		bool changed = ((this->valueNull) || (!isEqual(this->value.asDouble, newValue, 0.01)));
		if (changed) {
			WPropertyValue valueB;
			valueB.asDouble = newValue;
			this->setValue(valueB);
		}
	}

	bool equalsDouble(double number) {
		return ((!this->valueNull) && (this->value.asDouble == number));
	}

	int getInteger() {
		requestValue();
		return (!this->valueNull ? this->value.asInteger : 0);
	}

	void setInteger(int newValue) {
		if (type != INTEGER) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asInteger != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asInteger = newValue;
			this->setValue(valueB);
		}
	}

	unsigned long getLong() {
		requestValue();
		return (!this->valueNull ? this->value.asLong : 0);
	}

	void setLong(unsigned long newValue) {
		if (type != LONG) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asLong != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asLong = newValue;
			this->setValue(valueB);
		}
	}

	bool equalsInteger(int number) {
		return ((!this->valueNull) && (this->value.asInteger == number));
	}

	bool equalsLong(unsigned long number) {
		return ((!this->valueNull) && (this->value.asLong == number));
	}

	byte getByte() {
		requestValue();
		return (!this->valueNull ? this->value.asByte : 0x00);
	}

	void setByte(byte newValue) {
		if (type != BYTE) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asByte != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asByte = newValue;
			this->setValue(valueB);
		}
	}

	bool equalsByte(byte number) {
		return ((!this->valueNull) && (this->value.asByte == number));
	}

	String getString() {
		requestValue();
		return (!this->valueNull ? String(value.string) : "");
	}

	const char* c_str() {
		return (!this->valueNull ? value.string : "");
	}

	WPropertyValue getValue() {
	    return this->value;
	}

	void setString(String newValue) {
		if (type != STRING) {
			return;
		}
		int l = newValue.length();
		if (l > length) {
			l = length;
		}
		bool changed = ((this->valueNull) || (strcmp(value.string, newValue.c_str()) != 0));
		if (changed) {
			strncpy(value.string, newValue.c_str(), l);
			value.string[l] = '\0';
			this->valueNull = false;
			valueChanged();
			notify();
		}
	}

	bool isReadOnly() {
		return readOnly;
	}

	void setReadOnly(bool readOnly) {
		this->readOnly = readOnly;
	}

	String getUnit() {
		return unit;
	}

	void setUnit(String unit) {
		this->unit = unit;
	}

	double getMultipleOf() {
		return multipleOf;
	}

	void setMultipleOf(double multipleOf) {
		this->multipleOf = multipleOf;
	}

	virtual void toJsonValue(WJson* json) {
		switch (getType()) {
		case BOOLEAN:
			json->property(getId(), getBoolean());
			break;
		case DOUBLE:
			json->property(getId(), getDouble());
			break;
		case INTEGER:
			json->property(getId(), getInteger());
			break;
		case LONG:
			json->property(getId(), getLong());
			break;
		case BYTE:
			json->property(getId(), getByte());
			break;
		case STRING:
			json->property(getId(), getString());
			break;
		}
	}

	virtual void toJsonStructure(WJson* json, String deviceHRef) {
		json->beginObject(getId());
		//title
		if (this->getTitle() != "") {
			json->property("title", getTitle());
		}
		//description
		if (this->getDescription() != "") {
			json->property("description", this->getDescription());
		}
		//type
		switch (this->getType()) {
		case BOOLEAN:
			json->property("type", "boolean");
			break;
		case DOUBLE:
		case INTEGER:
		case LONG:
		case BYTE:
			json->property("type", "number");
			break;
		default:
			json->property("type", "string");
			break;
		}
		//readOnly
		if (this->isReadOnly()) {
			json->property("readOnly", true);
		}
		//unit
		if (this->getUnit() != "") {
			json->property("unit", this->getUnit());
		}
		//multipleOf
		if (this->getMultipleOf() > 0.0) {
			json->property("multipleOf", this->getMultipleOf());
		}
		//enum
		if (hasEnum()) {
			json->beginArray("enum");
			WProperty* propE = this->firstEnum;
			while (propE != nullptr) {
				switch (this->getType()) {
				case BOOLEAN:
					json->boolean(propE->getBoolean());
				   	break;
				case DOUBLE:
				case INTEGER:
				case LONG:
				case BYTE:
					json->number(propE->getByte());
					break;
				case STRING:
					json->string(propE->getString());
				  	break;
				}
				propE = propE->next;
			}
			json->endArray();
		}
		//aType
		if (this->getAtType() != "") {
			json->property("@type", this->getAtType());
		}
		toJsonStructureAdditionalParameters(json);
		json->property("href", deviceHRef + "/properties/" + this->getId());
		json->endObject();
	}

	WProperty* next;

	void addEnumBoolean(bool enumBoolean) {
		if (type != BOOLEAN) {
			return;
		}
		WProperty* valueE = new WProperty("", "", "", this->type, 0);
		valueE->setBoolean(enumBoolean);
		this->addEnum(valueE);
	}

	void addEnumNumber(double enumNumber) {
		if (type != DOUBLE) {
			return;
		}
		WProperty* valueE = new WProperty("", "", "", this->type, 0);
		valueE->setDouble(enumNumber);
		this->addEnum(valueE);
	}

	void addEnumInteger(int enumNumber) {
		if (type != INTEGER) {
			return;
		}
		WProperty* valueE = new WProperty("", "", "", this->type, 0);
		valueE->setInteger(enumNumber);
		this->addEnum(valueE);
	}

	void addEnumLong(unsigned long enumNumber) {
		if (type != LONG) {
			return;
		}
		WProperty* valueE = new WProperty("", "", "", this->type, 0);
		valueE->setLong(enumNumber);
		this->addEnum(valueE);
	}

	void addEnumByte(byte enumByte) {
		if (type != BYTE) {
			return;
		}
		WProperty* valueE = new WProperty("", "", "", this->type, 0);
		valueE->setByte(enumByte);
		this->addEnum(valueE);
	}

	void addEnumString(String enumString) {
		if (type != STRING) {
			return;
		}
		WProperty* valueE = new WProperty("", "", "", this->type, this->length - 1);
		valueE->setString(enumString);
		this->addEnum(valueE);
	}

	void addEnum(WProperty* propEnum) {
		WProperty* lastEnum = firstEnum;
		while ((lastEnum != nullptr) && (lastEnum->next != nullptr)) {
			lastEnum = lastEnum->next;
		}
		if (lastEnum != nullptr) {
			lastEnum->next = propEnum;
		} else {
			firstEnum = propEnum;
		}
	}

	bool hasEnum() {
		return (firstEnum != nullptr);
	}

	WPropertyVisibility getVisibility() {
		return visibility;
	}

	void setVisibility(WPropertyVisibility visibility) {
		this->visibility = visibility;
	}

	bool isVisible(WPropertyVisibility visibility) {
		return ((this->visibility == ALL) || (this->visibility == visibility));
	}

protected:
	String atType;

	void initialize(String id, String title, String description, WPropertyType type, byte length) {
		this->id = id;
		this->title = title;
		this->description = description;
		this->type = type;
		this->visibility = ALL;
		this->supportingWebthing = true;
		this->valueNull = true;
		this->requested = false;
		this->valueRequesting = false;
		this->readOnly = false;
		this->unit = "";
		this->multipleOf = 0.0;
		this->next = nullptr;;
		switch (type) {
		case STRING:
			this->length = length;
			value.string = new char[length + 1];
			break;
		case DOUBLE:
			this->length = sizeof(double);
			break;
		case INTEGER:
			this->length = 2;
			break;
		case LONG:
			this->length = 4;
			break;
		case BYTE:
		case BOOLEAN:
			this->length = 1;
			break;
		}
	}

	void setValue(WPropertyValue newValue) {
		this->value = newValue;
		this->valueNull = false;
		valueChanged();
		notify();
	}

	virtual void valueChanged() {
	}

	virtual void toJsonStructureAdditionalParameters(WJson* json) {

	}

private:
	String id;
	String title;
	String description;
	WPropertyType type;
	WPropertyVisibility visibility;
	bool supportingMqtt;
	bool supportingWebthing;
	byte length;
	bool readOnly;
	String unit;
	double multipleOf;
	TOnPropertyChange onChange;
	TOnPropertyChange onValueRequest;
	TOnPropertyChange deviceNotification;
	WPropertyValue value = {false};
	bool valueNull;
	bool requested;
	bool valueRequesting;

	WProperty* firstEnum = nullptr;

	void notify() {
		if (!valueRequesting) {
			if (onChange) {
				onChange(this);
			}
			if (deviceNotification) {
				deviceNotification(this);
			}
		}
	}

	void requestValue() {
		if (onValueRequest) {
			valueRequesting = true;
			onValueRequest(this);
			valueRequesting = false;
		}
	}

};

#endif
