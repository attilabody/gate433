/*
 * config.h
 *
 *  Created on: Jan 23, 2018
 *      Author: compi
 */
#ifndef CONFIG_H_
#define CONFIG_H_
#include "i2csingletons.h"
#include "commands.h"

struct ConfigData {
	struct Header {
		uint8_t 	magic = 0x5a;
		uint8_t		version = 1;
	} header;
	uint8_t 	lcdI2cAddress = 0x27 << 1;
	uint8_t		passTimeout = 30;
	uint8_t		hurryTimeout = 90;
	bool		relaxedPos = false;
	bool		relaxedDateTime = false;
};

struct Config : public ConfigData, public sg::Singleton<Config>
{
	friend class sg::Singleton<Config>;

	struct ConfigItemDescriptor {
		const char* mnemonic;
		enum Type { _uint8_t, _bool } type;
		union {
			uint8_t	ConfigData::*uint8ptr;
			bool	ConfigData::*boolptr;
		};
	};

	bool Load();
	bool Save();
	bool Set(const char *name, const char *value);

private:
	static constexpr ConfigItemDescriptor s_configItems[] = {
			{ CFG_LCDI2CADDRESS, ConfigItemDescriptor::_uint8_t, { uint8ptr: &ConfigData::lcdI2cAddress }},
			{ CFG_PASSTIMEOUT, ConfigItemDescriptor::_uint8_t, { uint8ptr: &ConfigData::passTimeout }},
			{ CFG_HURRYTIMEOUT, ConfigItemDescriptor::_uint8_t, { uint8ptr: &ConfigData::hurryTimeout }},
			{ CFG_RELAXEDPOS, ConfigItemDescriptor::_bool, { boolptr: &ConfigData::relaxedPos }},
			{ CFG_RELAXEDDATETIME, ConfigItemDescriptor::_bool, { boolptr: &ConfigData::relaxedDateTime }},
	};


};

#endif /* CONFIG_H_ */
