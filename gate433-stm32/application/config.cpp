/*
 * config.cpp
 *
 *  Created on: Apr 17, 2018
 *      Author: abody
 */
#include "config.h"
#include <memory.h>
#include <sg/Strutil.h>

//////////////////////////////////////////////////////////////////////////////
bool Config::Load()
{
	HAL_StatusTypeDef ret = HAL_OK;
	Header	h;
	if(	(ret = MainI2cEeprom::Instance().Read(&h, 0, sizeof(h))) == HAL_OK) {
		if(h.magic == header.magic && h.version == header.version )
			ret = MainI2cEeprom::Instance().Read(static_cast<ConfigData*>(this), 0, sizeof(ConfigData));
		else
			return false;
	}
	return ret == HAL_OK;
}

//////////////////////////////////////////////////////////////////////////////
bool Config::Save()
{
	return MainI2cEeprom::Instance().Write(static_cast<ConfigData*>(this), 0, sizeof(ConfigData)) == HAL_OK;
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
			case ConfigItemDescriptor::_uint8_t:
				{
					int32_t val = sg::GetIntParam(value);
					if(val != -1)
						static_cast<ConfigData*>(this)->*(desc.uint8ptr) = static_cast<uint8_t>(val);
				}
				break;

			case ConfigItemDescriptor::_bool:
				{
					bool val;
					int32_t tmp;

					if(!strncmp(value, "true", 4))
						val = true;
					else if(!strncmp(value, "false", 5))
						val = false;
					else if((tmp = sg::GetIntParam(value)) != -1)
						val = tmp != 0;
					else
						break;
					static_cast<ConfigData*>(this)->*(desc.boolptr) = val;
				}
				break;
			}
		}

	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////
