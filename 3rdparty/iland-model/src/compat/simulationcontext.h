#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>

class SimulationSettings;

// A run owns its state. TLS only selects that state for legacy static accessors;
// worker threads explicitly inherit the caller's context through Scope.
class SimulationContext {
  struct StateBase {
    virtual ~StateBase() = default;
  };
  template <class T> struct State final : StateBase { T value{}; };
  std::recursive_mutex statesMutex_;
  std::unordered_map<std::type_index, std::unique_ptr<StateBase>> states_;
  std::vector<std::type_index> destructionOrder_;
  std::vector<std::function<void()>> cleanup_;
  inline static thread_local SimulationContext *active_ = nullptr;
  inline static thread_local std::uint64_t binding_ = 0;

public:
  class Scope {
    SimulationContext *previous_;
  public:
    explicit Scope(SimulationContext &context) : previous_(active_) {
      active_ = &context;
      ++binding_;
    }
    Scope(const Scope &) = delete;
    Scope &operator=(const Scope &) = delete;
    ~Scope() { active_ = previous_; ++binding_; }
  };

  SimulationContext() = default;
  SimulationContext(const SimulationContext &) = delete;
  SimulationContext &operator=(const SimulationContext &) = delete;
  // Borrowed reference; this context owns the settings for its full lifetime.
  SimulationSettings &settings();
  ~SimulationContext() {
    Scope scope(*this);
    while (!cleanup_.empty()) {
      auto cleanup = std::move(cleanup_.back());
      cleanup_.pop_back();
      cleanup();
    }
    // Reverse construction order keeps earlier services alive for dependents.
    // Erasing first permits a destructor to access another state safely.
    while (!destructionOrder_.empty()) {
      auto key = destructionOrder_.back();
      destructionOrder_.pop_back();
      auto node = states_.extract(key);
      ++binding_; // cached references to destroyed slots must never be reused
      node.mapped().reset();
    }
  }
  static SimulationContext *active() noexcept { return active_; }
  void onExit(std::function<void()> cleanup) {
    cleanup_.push_back(std::move(cleanup));
  }
  static SimulationContext &current() {
    if (!active_)
      throw std::logic_error("Simulation state accessed without a run context");
    return *active_;
  }
  template <class T> static T &state() {
    // The binding generation also handles nested calls and reused stack addresses.
    struct Cache { std::uint64_t binding = 0; T *value = nullptr; };
    static thread_local Cache cache;
    if (cache.binding != binding_ || !cache.value) {
      auto &context = current();
      std::lock_guard<std::recursive_mutex> lock(context.statesMutex_);
      auto key = std::type_index(typeid(T));
      auto found = context.states_.find(key);
      if (found == context.states_.end()) {
        auto value = std::make_unique<State<T>>();
        found = context.states_.emplace(key, std::move(value)).first;
        context.destructionOrder_.push_back(key);
      }
      cache.value = &static_cast<State<T> *>(found->second.get())->value;
      cache.binding = binding_;
    }
    return *cache.value;
  }
};
