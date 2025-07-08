#pragma once
#include <INIReader.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <type_traits>

struct Settings;


class ISetting {
public:
    virtual ~ISetting() = default;

    // Getters for writing the default INI file
    virtual const std::string& getKey() const = 0;
    virtual const std::string& getComment() const = 0;
    virtual std::string getDefaultValueAsString() const = 0;

    // Functions to apply values to the Settings struct
    virtual void load(INIReader& reader, Settings& settings) const = 0;
    virtual void applyDefault(Settings& settings) const = 0;
};

template<typename T>
class Setting : public ISetting {
public:

    Setting(std::string key, T defaultValue, T Settings::* memberPtr, std::string comment)
        : m_key(std::move(key)),
        m_defaultValue(defaultValue),
        m_memberPtr(memberPtr),
        m_comment(std::move(comment)) {
    }

    const std::string& getKey() const override { return m_key; }
    const std::string& getComment() const override { return m_comment; }

    std::string getDefaultValueAsString() const override {
        if constexpr (std::is_same_v<T, bool>) {
            return m_defaultValue ? "true" : "false";
        }
        else {
            return std::to_string(m_defaultValue);
        }
    }

    void load(INIReader& reader, Settings& settings) const override {
        if constexpr (std::is_same_v<T, bool>) {
            settings.*m_memberPtr = reader.GetBoolean("main", m_key, m_defaultValue);
        }
        else if constexpr (std::is_floating_point_v<T>) {
            settings.*m_memberPtr = reader.GetReal("main", m_key, m_defaultValue);
        }
        else if constexpr (std::is_integral_v<T>) {
            settings.*m_memberPtr = reader.GetInteger("main", m_key, m_defaultValue);
        }
    }

    void applyDefault(Settings& settings) const override {
        settings.*m_memberPtr = m_defaultValue;
    }

private:
    std::string m_key;
    T m_defaultValue;
    T Settings::* m_memberPtr; 
    std::string m_comment;
};


struct Settings {
public:

    // Declare members here
    bool debug;
    FLOAT mp_multiplier_required_for_item;
    FLOAT mp_multiplier_on_item_use;
    FLOAT mp_reduction_on_item_use;

    bool use_fixed_max_mp;
    FLOAT fixed_max_mp;
    FLOAT max_mp_multiplier;

    bool use_fixed_max_hp;
    int fixed_max_hp;
    FLOAT max_hp_multiplier;

    FLOAT passive_mp_recovery_multiplier;

    bool enable_mp_recovery_on_hit;
    FLOAT fixed_mp_recovery_on_hit;
    FLOAT multiplier_mp_recovery_on_hit;

    FLOAT attack_stat_multiplier;
    FLOAT magic_attack_stat_multiplier;
    FLOAT defense_stat_multiplier;
    FLOAT magic_defense_stat_multiplier;

    Settings() {
        // Define members here

#ifdef NDEBUG
        registerSetting("debug", false, &Settings::debug, "Opens a debug console window. Set to true for troubleshooting.");
#else
        registerSetting("debug", true, &Settings::debug, "Opens a debug console window. Set to true for troubleshooting.");
#endif

        registerSetting("mp_multiplier_required_for_item", 1.0f, &Settings::mp_multiplier_required_for_item, "Using an item fails if current MP is below this fraction of max MP. (e.g., 0.5 = 50%)");
        registerSetting("mp_multiplier_on_item_use", 0.0f, &Settings::mp_multiplier_on_item_use, "On item use, current MP is multiplied by this value. (e.g., 0.0 = lose all MP)");
        registerSetting("mp_reduction_on_item_use", 0.0f, &Settings::mp_reduction_on_item_use, "On item use, this flat amount of MP is subtracted after the multiplier.");

        registerSetting("use_fixed_max_mp", true, &Settings::use_fixed_max_mp, "If true, max MP is set to a fixed value. If false, it's based on a multiplier.");
        registerSetting("fixed_max_mp", 100.0f, &Settings::fixed_max_mp, "The fixed value for max MP if use_fixed_max_mp is true.");
        registerSetting("max_mp_multiplier", 1.0f, &Settings::max_mp_multiplier, "Multiplies max MP if use_fixed_max_mp is false.");

        registerSetting("use_fixed_max_hp", false, &Settings::use_fixed_max_hp, "If true, max HP is set to a fixed value. If false, it's based on a multiplier.");
        registerSetting("fixed_max_hp", 250, &Settings::fixed_max_hp, "The fixed value for max HP if use_fixed_max_hp is true.");
        registerSetting("max_hp_multiplier", 0.75f, &Settings::max_hp_multiplier, "Multiplies max HP if use_fixed_max_hp is false.");

        registerSetting("passive_mp_recovery_multiplier", 0.25f, &Settings::passive_mp_recovery_multiplier, "Multiplier for the passive MP regeneration rate.");

        registerSetting("enable_mp_recovery_on_hit", true, &Settings::enable_mp_recovery_on_hit, "If true, you will gain MP when hitting an enemy.");
        registerSetting("fixed_mp_recovery_on_hit", 0.0f, &Settings::fixed_mp_recovery_on_hit, "A flat amount of MP gained on each hit.");
        registerSetting("multiplier_mp_recovery_on_hit", 0.05f, &Settings::multiplier_mp_recovery_on_hit, "A fraction of max MP gained on each hit. (e.g., 0.1 = 10% of max MP).");

        registerSetting("attack_stat_multiplier", 2.0f, &Settings::attack_stat_multiplier, "Attack multiplier");
        registerSetting("magic_attack_stat_multiplier", 1.2f, &Settings::magic_attack_stat_multiplier, "Magic attack multiplier");
        registerSetting("defense_stat_multiplier", 1.0f, &Settings::defense_stat_multiplier, "Defense multiplier");
        registerSetting("magic_defense_stat_multiplier", 1.0f, &Settings::magic_defense_stat_multiplier, "Magic Defense multiplier");

        

    }

    // Returns: 0 = success, 1 = used defaults (corrupt file), 2 = used defaults (created new file)
    int LoadFromFile(const std::string& path = "NieR_Replicant_Hardcore.ini") {

        std::ifstream fileCheck(path);
        if (!fileCheck.good()) {
            // File does not exist. Create it.
            WriteDefaultIni(path);
            // Load the default values into the struct.
            for (const auto& setting : m_settings) {
                setting->applyDefault(*this);
            }
            return 2; 
        }
        fileCheck.close();

        // File exists, now try to parse it.
        INIReader reader(path);
        if (reader.ParseError() != 0) {
            MessageBoxW(0,
                L"The INI file is corrupt or contains an error.\n\n"
                L"Default settings will be used for this session.\n\n"
                L"To fix this, you can either correct the error or delete the INI file.\n"
                L"A new, clean INI will be generated on the next game launch.",
                L"INI Parse Error", 0);

            // Load the default values 
            for (const auto& setting : m_settings) {
                setting->applyDefault(*this);
            }
            return 1; 
        }

        // File exists and is valid
        for (const auto& setting : m_settings) {
            setting->load(reader, *this);
        }
        return 0; 
    }

private:
    // Helper function to make registration cleaner.
    template<typename T>
    void registerSetting(const std::string& key, T defaultValue, T Settings::* memberPtr, const std::string& comment) {
        m_settings.push_back(std::make_unique<Setting<T>>(key, defaultValue, memberPtr, comment));
    }

    // Writes a new INI file with all defaults and comments
    void WriteDefaultIni(const std::string& path) const {
        std::ofstream file(path);
        if (!file.is_open()) {
            return;
        }

        file << "# NieR Replicant Hardcore Mod Settings\n";
        file << "# You can edit the values below to customize the mod.\n\n";

        file << "[main]\n\n";

        for (const auto& setting : m_settings) {
            file << "# " << setting->getComment() << "\n";
            file << setting->getKey() << " = " << setting->getDefaultValueAsString() << "\n\n";
        }
    }

    std::vector<std::unique_ptr<ISetting>> m_settings;
};