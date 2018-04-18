/*
 * config.cpp
 *
 *  Created on: Apr 17, 2018
 *      Author: abody
 */
#include "config.h"
#include <memory.h>
#include <sg/Strutil.h>

Config::ConfigItemDescriptor const Config::s_configItems[] = {
		{ CFG_LCDI2CADDRESS, &ConfigData::lcdI2cAddress },
		{ CFG_PASSTIMEOUT, &ConfigData::passTimeout },
		{ CFG_HURRYTIMEOUT, &ConfigData::hurryTimeout },
		{ CFG_RELAXEDPOS, &ConfigData::relaxedPos },
		{ CFG_RELAXEDDATETIME, &ConfigData::relaxedDateTime },
};

//////////////////////////////////////////////////////////////////////////////
bool Config::Load()
{
	HAL_StatusTypeDef hs = HAL_OK;
	Header	h;
	auto &eeprom = MainI2cEeprom::Instance();

	if((hs = eeprom.Read(&h, 0, sizeof(h))) == HAL_OK) {
		eeprom.Sync();
		if(h.magic == header.magic && h.version == header.version ) {
			hs = eeprom.Read(static_cast<ConfigData*>(this), 0, sizeof(ConfigData));
			if(hs == HAL_OK)
				eeprom.Sync();
		}
		else
			return false;
	}
	return hs == HAL_OK;
}

//////////////////////////////////////////////////////////////////////////////
bool Config::Save()
{
	return MainI2cEeprom::Instance().Write(static_cast<ConfigData*>(this), 0, sizeof(ConfigData)) == HAL_OK;
}

//////////////////////////////////////////////////////////////////////////////
bool Config::Reset()
{
	Header	h;
	memset(&h, 0, sizeof(h));
	bool ret = (MainI2cEeprom::Instance().Write(&h, 0, sizeof(h)) == HAL_OK);
	*static_cast<ConfigData*>(this) = ConfigData();
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
bool Config::Set(const char *name, const char *value)
{
	bool ret = false;

	for(ConfigItemDescriptor const &desc : s_configItems)
	{
		if(!strncmp(desc.mnemonic, name, strlen(desc.mnemonic)))
		{
			switch(desc.type)
			{
			case ConfigItemDescriptor::uint8_t:
				{
					int32_t val = sg::GetIntParam(value);
					if(val != -1) {
						static_cast<ConfigData*>(this)->*(desc.uint8ptr) = static_cast<uint8_t>(val);
						ret = true;
					}
				}
				break;

			case ConfigItemDescriptor::bool_t:
				{
					bool val;
					int32_t tmp;
					ret = true;

					if(!strncmp(value, "true", 4))
						val = true;
					else if(!strncmp(value, "false", 5))
						val = false;
					else if((tmp = sg::GetIntParam(value)) != -1)
						val = tmp != 0;
					else {
						ret = false;
						break;
					}
					static_cast<ConfigData*>(this)->*(desc.boolptr) = val;
				}
				break;
			}
		}

	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////
bool Config::Get(char* buffer, const char *name)
{
	bool ret = false;

	for(ConfigItemDescriptor const &desc : s_configItems)
	{
		if(!strncmp(desc.mnemonic, name, strlen(desc.mnemonic))) {
			ToBuffer(buffer, desc);
			ret = true;
			break;
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
bool Config::Get(char* buffer, uint8_t index)
{
	if(index < sizeof(s_configItems)/sizeof(s_configItems[0]))
	{
		const char *src = s_configItems[index].mnemonic;
		while(*src) *buffer++ = *src++;
		*buffer++ = ':';
		*buffer++ = ' ';
		ToBuffer(buffer, s_configItems[index]);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
void Config::ToBuffer(char *buffer, ConfigItemDescriptor const &desc)
{
	switch(desc.type)
	{
	case ConfigItemDescriptor::uint8_t:
		sg::ToDec(buffer, static_cast<ConfigData*>(this)->*(desc.uint8ptr));
		break;

	case ConfigItemDescriptor::bool_t:
		strcpy(buffer, static_cast<ConfigData*>(this)->*(desc.boolptr) ? "true":"false");
		break;
	}
}
