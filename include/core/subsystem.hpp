#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <any>

namespace core {

// Base interface for all subsystems
class ISubsystem {
public:
    virtual ~ISubsystem() = default;
    virtual void initialize() = 0;
    virtual void update() = 0;
    virtual void disable() = 0;
    virtual bool isEnabled() const = 0;
    virtual const std::string& getName() const = 0;
};

// Configuration base class
class SubsystemConfig {
public:
    virtual ~SubsystemConfig() = default;
    bool dev_mode = false;
};

// Base class for subsystems with common functionality
template<typename Config = SubsystemConfig>
class Subsystem : public ISubsystem {
protected:
    std::string name_;
    bool enabled_ = false;
    Config config_;

public:
    Subsystem(const std::string& name, const Config& config = Config())
        : name_(name), config_(config) {}

    virtual void initialize() override { enabled_ = true; }
    virtual void disable() override { enabled_ = false; }
    virtual bool isEnabled() const override { return enabled_; }
    virtual const std::string& getName() const override { return name_; }
    
    const Config& getConfig() const { return config_; }
};

// Registry for managing subsystems
class SubsystemRegistry {
private:
    static SubsystemRegistry* instance_;
    std::unordered_map<std::string, std::shared_ptr<ISubsystem>> subsystems_;
    std::unordered_map<std::type_index, std::any> type_cache_;

    SubsystemRegistry() = default;

public:
    static SubsystemRegistry& getInstance() {
        if (!instance_) {
            instance_ = new SubsystemRegistry();
        }
        return *instance_;
    }

    template<typename T>
    void registerSubsystem(const std::shared_ptr<T>& subsystem) {
        auto name = subsystem->getName();
        subsystems_[name] = subsystem;
        type_cache_[std::type_index(typeid(T))] = subsystem;
        subsystem->initialize();
    }

    template<typename T>
    std::shared_ptr<T> getSubsystem(const std::string& name) {
        auto it = subsystems_.find(name);
        if (it != subsystems_.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    template<typename T>
    std::shared_ptr<T> getSubsystemByType() {
        auto it = type_cache_.find(std::type_index(typeid(T)));
        if (it != type_cache_.end()) {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        }
        return nullptr;
    }

    void updateAll() {
        for (auto& [name, subsystem] : subsystems_) {
            if (subsystem->isEnabled()) {
                subsystem->update();
            }
        }
    }

    void disableAll() {
        for (auto& [name, subsystem] : subsystems_) {
            subsystem->disable();
        }
    }
};

// Event system for inter-subsystem communication
class EventSystem {
private:
    static EventSystem* instance_;
    std::unordered_map<std::string, std::vector<std::function<void(const std::any&)>>> handlers_;

    EventSystem() = default;

public:
    static EventSystem& getInstance() {
        if (!instance_) {
            instance_ = new EventSystem();
        }
        return *instance_;
    }

    template<typename T>
    void subscribe(const std::string& event_type, std::function<void(const T&)> handler) {
        handlers_[event_type].push_back(
            [handler](const std::any& data) {
                if (auto* typed_data = std::any_cast<T>(&data)) {
                    handler(*typed_data);
                }
            }
        );
    }

    template<typename T>
    void emit(const std::string& event_type, const T& data) {
        auto it = handlers_.find(event_type);
        if (it != handlers_.end()) {
            for (const auto& handler : it->second) {
                handler(data);
            }
        }
    }
};

} // namespace core
