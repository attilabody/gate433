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
		enum Type { uint8_t, bool_t } type;
		union {
			::uint8_t	ConfigData::*uint8ptr;
			bool		ConfigData::*boolptr;
		};
	};

	bool Load();
	bool Save();
	bool Reset();
	bool Set(const char *name, const char *value);
	bool Get(char* buffer, const char *name);
	bool Get(char* buffer, uint8_t index);

private:
	static ConfigItemDescriptor const s_configItems[5];
	void ToBuffer(char *buffer, ConfigItemDescriptor const &desc);


};

#endif /* CONFIG_H_ */
