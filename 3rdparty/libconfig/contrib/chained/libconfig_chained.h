/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   libconfig chained - Extension for reading the configuration and defining 
                       the configuration specification at once.
   Copyright (C) 2016 Richard Schubert

   This file is part of libconfig contributions.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

#pragma once
#ifndef _CHAINED_LIBCONFIG_H_
#define _CHAINED_LIBCONFIG_H_

#include <libconfig.h++>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>

namespace libconfig 
{
	class ChainedSetting
	{
		struct Variant
		{
		private:
			bool			isSet;
			Setting::Type	type;

			bool			value_bool;
			int64_t			value_int;
			double			value_float;
			std::string		value_string;

		public:

			Variant()
				: isSet(false)
				, type(Setting::TypeNone)
			{
			}
			Variant(bool value)
			{
				value_bool = value;
				isSet = true;
				type = Setting::TypeBoolean;
			}
			Variant(int32_t value)
			{
				value_int = value;
				isSet = true;
				type = Setting::TypeInt;
			}
			Variant(int64_t value)
			{
				value_int = value;
				isSet = true;
				type = Setting::TypeInt64;
			}
			Variant(double value)
			{
				value_float = value;
				isSet = true;
				type = Setting::TypeFloat;
			}
			Variant(std::string& value)
			{
				value_string = value;
				isSet = true;
				type = Setting::TypeString;
			}
			Variant(const char* value)
			{
				value_string = value;
				isSet = true;
				type = Setting::TypeString;
			}

			operator bool() const { return value_bool; }
			operator int() const { return (int)value_int; }
			operator unsigned int() const { return (unsigned int)value_int; }
			operator long() const { return (long)value_int; }
			operator unsigned long() const { return (unsigned long)value_int; }
			operator long long() const { return (long long)value_int; }
			operator unsigned long long() const { return (unsigned long long)value_int; }
			operator double() const { return value_float; }
			operator float() const { return (float)value_float; }
			operator std::string() const { return value_string; }

			const bool IsSet() const
			{
				return isSet;
			}

			const Setting::Type GetType() const
			{
				return type;
			}
		};

	public:

		// Starting point for method chained libconfig.
		// Pass a custom ostream to intercept any error messages (useful for Applications with UI).
		ChainedSetting(Setting& setting, std::ostream& err = std::cerr)
			: name(setting.isRoot() ? "<root>" : (setting.getName() ? setting.getName() : ""))
			, index(setting.getIndex())
			, parent(NULL)
			, setting(&setting)
			, err(err)
			, isSettingMandatory(false)
			, anySettingIsMissing(false)
			, anyMandatorySettingIsMissing(false)
			, capturedSpecification(NULL)
			, capturedSetting(NULL)
		{
		}

		// Starts capturing any configuration readings into the temporary config object.
		void captureExpectedSpecification(Config* temporaryConfigSpecification)
		{
			capturedSpecification = temporaryConfigSpecification;
			capturedSetting = &capturedSpecification->getRoot();
		}

		// Returns the captured configuration specification, 
		// premised captureExpectedSpecification() was called earlier.
		// The path parameter is needed to write the configuration 
		// to disk before it can be read into a usable string.
		std::string getCapturedSpecification(const std::string& tempFilePath)
		{
			try
			{
				capturedSpecification->writeFile(tempFilePath.c_str());
			}
			catch (const FileIOException&)
			{
				err << "I/O error while writing temporary setting file: " << tempFilePath << std::endl;
				return "";
			}

			std::ifstream t(tempFilePath);
			if (!t.is_open())
			{
				err << "I/O error while reading temporary setting file: " << tempFilePath << std::endl;
				return "";
			}
			std::stringstream buffer;
			buffer << t.rdbuf();

			capturedSpecification = NULL;

			return buffer.str();
		}

		// Defines the default value for this setting if missing from config file.
		template<typename T>
		ChainedSetting& defaultValue(T defaultValue)
		{
			defaultVal = defaultValue;
			return *this;
		}

		// Defines the inclusive minimum value for this setting.
		// A lesser value set in a configuration file will be clamped to this limit.
		template<typename T>
		ChainedSetting& min(T min)
		{
			minVal = min;
			return *this;
		}

		// Defines the inclusive maximum value for this setting.
		// A greater value set in a configuration file will be clamped to this limit.
		template<typename T>
		ChainedSetting& max(T max)
		{
			maxVal = max;
			return *this;
		}

		// Defines this setting to be mandatory.
		// Any mandatory value missing in the configuration file will raise an error.
		// Use isAnyMandatorySettingMissing() to check for any violations.
		ChainedSetting& isMandatory()
		{
			isSettingMandatory = true;
			if (parent) parent->isMandatory();
			return *this;
		}

		template<typename T>
		operator T()
		{
			auto requestedType = GetRequestedType<T>();
			CheckType(defaultVal, requestedType);
			CheckType(minVal, requestedType);
			CheckType(maxVal, requestedType);

			CaptureSetting<T>(requestedType);

			if (!setting)
			{
				if (isSettingMandatory)
				{
					AlertMandatorySettingMissing<T>();
				}
				PropagateAnySettingIsMissing();

				return GetDefaultValue<T>();
			}

			try
			{
				T value = *setting;
				if (minVal.IsSet())
				{
					T min = minVal;
					if (value < min)
					{
						err << "'" << setting->getPath() << "' setting is out of valid bounds (min: " << min << "). Value was: " << value << std::endl;
						value = min;
					}
				}
				if (maxVal.IsSet())
				{
					T max = maxVal;
					if (value > max)
					{
						err << "'" << setting->getPath() << "' setting is out of valid bounds (max: " << max << "). Value was: " << value << std::endl;
						value = max;
					}
				}
				return value;
			}
			catch (const SettingTypeException& tex)
			{
				err << "'" << tex.getPath() << "' setting is of wrong type." << std::endl;
			}

			return GetDefaultValue<T>();
		}

		ChainedSetting operator[](const char *name)
		{
			CaptureSetting<Setting>(Setting::TypeGroup);

			if (!setting)
			{
				return ChainedSetting(name, this);
			}

			if(setting->exists(name))
			{
				return ChainedSetting((*setting)[name], this);
			}
			else
			{
				return ChainedSetting(name, this);
			}
		}

		inline ChainedSetting operator[](const std::string &name)
		{
			return(operator[](name.c_str()));
		}

		ChainedSetting operator[](int index)
		{
			// This could also be an TypeArray but we cannot be sure here.
			// By using TypeList we ensure it will always work.
			CaptureSetting<Setting>(Setting::TypeList);

			if (!setting)
			{
				return ChainedSetting(index, this);
			}

			if (index >= 0 && index < setting->getLength())
			{
				return ChainedSetting((*setting)[index], this);
			}
			else
			{
				return ChainedSetting(index, this);
			}
		}

		int getLength() const
		{
			return setting ? setting->getLength() : 0;
		}

		Setting::Type getType() const
		{
			return setting ? setting->getType() : Setting::TypeNone;
		}

		// Indicates whether this setting is present in the read configuration file.
		bool exists() const
		{
			return setting != NULL;
		}

		bool isAnyMandatorySettingMissing() const
		{
			return anyMandatorySettingIsMissing;
		}

		bool isAnySettingMissing() const
		{
			return anySettingIsMissing;
		}

		void clearAnySettingMissingFlag()
		{
			anySettingIsMissing = false;
		}

	private:

		ChainedSetting(Setting& setting, ChainedSetting* parent)
			: name(setting.isRoot() ? "<root>" : (setting.getName() ? setting.getName() : ""))
			, index(setting.getIndex())
			, parent(parent)
			, setting(&setting)
			, err(parent->err)
			, isSettingMandatory(false)
			, anySettingIsMissing(false)
			, anyMandatorySettingIsMissing(false)
			, capturedSpecification(NULL)
			, capturedSetting(NULL)
		{
		}
		
		ChainedSetting(const std::string& name, ChainedSetting* parent)
			: name(name)
			, index(-1)
			, parent(parent)
			, setting(NULL)
			, err(parent->err)
			, isSettingMandatory(false)
			, anySettingIsMissing(true)
			, anyMandatorySettingIsMissing(false)
			, capturedSpecification(NULL)
			, capturedSetting(NULL)
		{
		}

		ChainedSetting(int index, ChainedSetting* parent)
			: name("")
			, index(index)
			, parent(parent)
			, setting(NULL)
			, err(parent->err)
			, isSettingMandatory(false)
			, anySettingIsMissing(true)
			, anyMandatorySettingIsMissing(false)
			, capturedSpecification(NULL)
			, capturedSetting(NULL)
		{
		}

		template<typename T>
		void ConditionalSetCapturedDefaultValue()
		{
			*capturedSetting = GetDefaultValue<T>();
		}

		

		template<typename T>
		void CaptureSetting(Setting::Type type)
		{
			if (!capturedSetting && parent && parent->capturedSetting)
			{
				if (name.length() > 0)
				{
					if (!parent->capturedSetting->exists(name))
					{
						capturedSetting = &parent->capturedSetting->add(name, type);
					}
					else
					{
						capturedSetting = &(*parent->capturedSetting)[name.c_str()];
					}
				}
				else
				{
					if (index < parent->capturedSetting->getLength())
					{
						capturedSetting = &(*parent->capturedSetting)[0];
					}
					else
					{
						assert(index == parent->capturedSetting->getLength()); // you requested an index while omitting at least one of its previous siblings
						capturedSetting = &parent->capturedSetting->add(type);
					}
				}

				ConditionalSetCapturedDefaultValue<T>();
			}
		}


		std::string GetPath() const
		{
			if (setting)
			{
				return setting->getPath();
			}

			std::string path = (name.length() > 0) ? name : "[" + std::to_string(index) + "]";
			if (parent)
			{
				auto parentPath = parent->GetPath();
				return (parentPath.length() > 0) ? (parentPath + ((name.length() == 0) ? "" : ".") + path) : path;
			}
			return path;
		}

		void PropagateAnySettingIsMissing()
		{
			anySettingIsMissing = true;
			if (parent)
			{
				parent->PropagateAnySettingIsMissing();
			}
		}

		void PropagateAnyMandatorySettingIsMissing()
		{
			anyMandatorySettingIsMissing = true;
			if (parent)
			{
				parent->PropagateAnyMandatorySettingIsMissing();
			}
		}

		template<typename T>
		void AlertMandatorySettingMissing()
		{
			PropagateAnyMandatorySettingIsMissing();

			err << "Missing '" << GetPath() << "' setting in configuration file." << std::endl;
		}

		template<typename T>
		T GetUnsetDefaultValue() const
		{
			return (T)0;
		}

		

		template<typename T>
		T GetDefaultValue() const
		{
			if (defaultVal.IsSet())
			{
				return (T)defaultVal;
			}

			return GetUnsetDefaultValue<T>();
		}

		template<typename T>
		Setting::Type GetRequestedType() const
		{
			// TODO @ Hemofektik: Check whether the outcommented line is still needed. static_assert(false) is checked on compile time and, well, asserts :)
            // static_assert(false, "should never happen, unless you requested an unsupported type");
			return Setting::TypeNone;
		}
		

		void CheckType(const Variant& variant, Setting::Type expectedType) const
		{
			if (!variant.IsSet()) return;
			if(expectedType != variant.GetType())
			{
				assert(false); // fix your code to match the whole chain of this setting to one single type!
				err << "'" << GetPath() << "' setting limits or default value is of incompatible type." << std::endl;
			}
		}

		std::string name;
		int index;
		ChainedSetting* parent;
		Setting* setting;
		std::ostream& err;
		Variant defaultVal;
		Variant minVal;
		Variant maxVal;
		bool isSettingMandatory;
		bool anySettingIsMissing;
		bool anyMandatorySettingIsMissing;
		Config* capturedSpecification;
		Setting* capturedSetting;
	};

    template<>
    inline 
    void ChainedSetting::ConditionalSetCapturedDefaultValue<Setting>() { }
	
	template<>
    inline 
    std::string ChainedSetting::GetUnsetDefaultValue() const
    {
        return "";
    }
    
    
    template<> inline Setting::Type ChainedSetting::GetRequestedType<int8_t>() const { return Setting::TypeInt; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<uint8_t>() const { return Setting::TypeInt; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<int16_t>() const { return Setting::TypeInt; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<uint16_t>() const { return Setting::TypeInt; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<int32_t>() const { return Setting::TypeInt; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<uint32_t>() const { return Setting::TypeInt; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<int64_t>() const { return Setting::TypeInt64; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<uint64_t>() const { return Setting::TypeInt64; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<float>() const { return Setting::TypeFloat; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<double>() const { return Setting::TypeFloat; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<std::string>() const { return Setting::TypeString; }
    template<> inline Setting::Type ChainedSetting::GetRequestedType<bool>() const { return Setting::TypeBoolean; }
}

#endif