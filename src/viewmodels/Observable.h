#ifndef CLEANGRADUATOR_OBSERVABLE_H
#define CLEANGRADUATOR_OBSERVABLE_H

#include <any>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mvvm {

/// Observable<T> — потокобезопасное наблюдаемое значение для MVVM.
///
/// - subscribe(...) возвращает Subscription (RAII): при разрушении автоматически отписывает.
/// - set(...) меняет значение и уведомляет подписчиков (если change-policy разрешает).
/// - transaction() позволяет сгруппировать серию set(...) в один "change" в конце.
template <typename T>
class Observable final {
public:
    using value_type = T;

    /// Какое событие приходит подписчику.
    struct Change {
        const T& old_value;
        const T& new_value;
    };

    /// Подписчик на изменение.
    using Observer = std::function<void(const Change&)>;

    /// Хук до изменения: можно запретить изменение (вернуть false) или модифицировать входящее значение.
    /// Сигнатура: bool(const T& old_value, T& proposed_new_value)
    using BeforeChange = std::function<bool(const T&, T&)>;

    /// Хук после изменения: вызывается после обновления (после уведомления или до — на твой вкус).
    /// Здесь вызываем после записи нового значения, но *до* notify (можно переставить при желании).
    using AfterChange = std::function<void(const Change&)>;

    /// Subscription — RAII токен отписки.
    class Subscription {
    public:
        Subscription() noexcept = default;

        Subscription(Observable* owner, std::uint64_t id) noexcept
            : owner_(owner), id_(id) {}

        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;

        Subscription(Subscription&& other) noexcept { move_from(std::move(other)); }
        Subscription& operator=(Subscription&& other) noexcept {
            if (this != &other) { reset(); move_from(std::move(other)); }
            return *this;
        }

        ~Subscription() { reset(); }

        void reset() noexcept {
            if (owner_) {
                owner_->unsubscribe(id_);
                owner_ = nullptr;
                id_ = 0;
            }
        }

        explicit operator bool() const noexcept { return owner_ != nullptr; }

    private:
        void move_from(Subscription&& other) noexcept {
            owner_ = other.owner_;
            id_ = other.id_;
            other.owner_ = nullptr;
            other.id_ = 0;
        }

        Observable* owner_ = nullptr;
        std::uint64_t id_ = 0;
    };

    /// Политика сравнения (distinct).
    struct DefaultEqual {
        bool operator()(const T& a, const T& b) const noexcept(noexcept(a == b)) {
            return a == b;
        }
    };

    /// Конструкторы
    template <
        typename U = T,
        typename = std::enable_if_t<std::is_default_constructible<U>::value>
    >
    Observable() : value_{} {}

    explicit Observable(T initial)
        : value_(std::move(initial)) {}

    Observable(const Observable&) = delete;
    Observable& operator=(const Observable&) = delete;

    Observable(Observable&&) = delete;
    Observable& operator=(Observable&&) = delete;

    ~Observable() = default;

    /// Получить копию значения (безопасно, но может копировать T).
    T get_copy() const {
        std::lock_guard lk(m_);
        return value_;
    }

    /// Получить значение по const& через пользовательскую функцию (без копий).
    /// Внутри держит лок только на время вызова f.
    template <class F>
    decltype(auto) with_value(F&& f) const {
        std::lock_guard lk(m_);
        return std::forward<F>(f)(static_cast<const T&>(value_));
    }

    /// Быстрая проверка: есть ли подписчики (не гарантирует, что прямо сейчас их не удалят).
    bool has_observers() const noexcept {
        std::lock_guard lk(m_);
        return !observers_.empty();
    }

    /// Подписка на изменения. Можно указать: distinct=true, и/или кастомный equal.
    ///
    /// distinct:
    ///  - true  => уведомлять только если old != new (по equal)
    ///  - false => уведомлять всегда при set(...)
    template <class Equal = DefaultEqual>
    Subscription subscribe(
        Observer cb,
        bool distinct = true,
        Equal equal = Equal{})
    {
        if (!cb) return {};

        std::lock_guard lk(m_);
        const auto id = next_id_++;
        observers_.emplace(id, ObserverEntry{
            std::move(cb),
            distinct,
            [eq = std::move(equal)](const T& a, const T& b) { return eq(a, b); }
        });
        return Subscription{this, id};
    }

    /// Хук до изменения (может быть только один; можно сделать мульти — если надо).
    void set_before_change(BeforeChange hook) {
        std::lock_guard lk(m_);
        before_ = std::move(hook);
    }

    /// Хук после изменения (тоже один).
    void set_after_change(AfterChange hook) {
        std::lock_guard lk(m_);
        after_ = std::move(hook);
    }

    template <typename, typename = void>
    struct has_equal : std::false_type {};

    template <typename U>
    struct has_equal<U, std::void_t<
        decltype(std::declval<const U&>() == std::declval<const U&>())
    >> : std::true_type {};

    /// Установить значение. Возвращает true, если реально поменялось (или было принудительно).
    ///
    /// force_notify:
    ///  - true  => уведомить даже если equal(old,new)==true (удобно для "rebind"/"refresh")
    ///  - false => уважать distinct настройки подписчиков + общую семантику
    template <class U = T>
    bool set(U&& v, bool force_notify = false) {
        // Сначала подготовим новое значение (с move, если можно)
        T proposed = static_cast<T>(std::forward<U>(v));

        std::optional<T> old_copy; // держим копию старого для уведомления
        std::vector<Observer> to_call;
        std::vector<bool> call_filter; // "пропускать ли уведомление" на уровне подписчика (distinct)
        AfterChange after;
        BeforeChange before;

        bool changed = false;
        bool in_tx = false;

        {
            std::lock_guard lk(m_);
            before = before_;
            after  = after_;

            // before-change hook
            if (before) {
                if (!before(value_, proposed)) {
                    return false; // изменение запрещено
                }
            }

            // Определим "changed" для семантики Observable
            // Здесь используем оператор== если доступен, иначе считаем changed=true всегда.
            if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::any>>) {
                changed = true; // не сравниваем
            }
            else if constexpr (has_equal<T>::value) {
                changed = !(value_ == proposed);
            }
            else {
                changed = true;
            }

            // Если нет транзакции — готовим уведомления сразу
            in_tx = (tx_depth_ > 0);

            // Запомним старое значение при необходимости уведомления/after
            // (Если не changed и не force_notify и in_tx — тоже не нужно.)
            const bool should_record = (force_notify || changed || after || in_tx);
            if (should_record) old_copy = value_;

            // Применяем
            value_ = std::move(proposed);

            if (in_tx) {
                // В транзакции: откладываем нотификацию.
                // Храним "последнее old" от начала транзакции и финальное new после последнего set.
                if (!tx_old_) tx_old_ = *old_copy;   // первое старое в транзакции
                tx_dirty_ = tx_dirty_ || force_notify || changed;
                // tx_new_ не нужно — новое можно взять из value_ в commit
                return force_notify || changed;
            }

            // Не в транзакции — готовим вызовы подписчиков.
            to_call.reserve(observers_.size());
            call_filter.reserve(observers_.size());

            for (auto const& [id, entry] : observers_) {
                (void)id;
                // distinct на уровне подписчика: сравнение custom equal
                const bool equal = entry.equal && old_copy
                    ? entry.equal(*old_copy, value_)
                    : false;

                const bool skip = (!force_notify) && entry.distinct && equal;
                to_call.push_back(entry.cb);
                call_filter.push_back(!skip);
            }
        } // unlock

        // after-change hook (вне лока)
        if (after && old_copy) {
            Change ch{*old_copy, with_value([](const T& x) -> const T& { return x; })};
            // Важно: new_value должен ссылаться на стабильное значение.
            // Мы берем ссылку через with_value (внутри снова лок на короткое время).
            // Альтернатива: скопировать новое значение, но это дороже.
            after(ch);
        }

        // Notify observers (вне лока)
        if (old_copy) {
            // Берём стабильную ссылку на new через короткий лок
            T new_copy = get_copy(); // проще и безопаснее; можно оптимизировать (см. ниже)
            Change ch{*old_copy, new_copy};

            for (std::size_t i = 0; i < to_call.size(); ++i) {
                if (call_filter[i]) {
                    to_call[i](ch);
                }
            }
        }

        return force_notify || changed;
    }

    /// Изменение "на месте": передаёшь лямбду, которая мутирует копию, потом set.
    /// Удобно для T=struct/DTO.
    template <class Mutator>
    bool update(Mutator&& mut, bool force_notify = false) {
        T cur = get_copy();
        std::forward<Mutator>(mut)(cur);
        return set(std::move(cur), force_notify);
    }

    /// Транзакция: в пределах scope нотификации откладываются и схлопываются в одну.
    class Transaction {
    public:
        explicit Transaction(Observable& o) : o_(o) { o_.begin_tx_(); }
        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;
        Transaction(Transaction&&) = delete;
        Transaction& operator=(Transaction&&) = delete;
        ~Transaction() { o_.end_tx_(); }
    private:
        Observable& o_;
    };

    /// Начать транзакцию (RAII).
    Transaction transaction() { return Transaction{*this}; }

private:
    struct ObserverEntry {
        Observer cb;
        bool distinct = true;
        std::function<bool(const T&, const T&)> equal; // true если equal
    };

    void unsubscribe(std::uint64_t id) noexcept {
        std::lock_guard lk(m_);
        observers_.erase(id);
    }

    void begin_tx_() {
        std::lock_guard lk(m_);
        ++tx_depth_;
    }

    void end_tx_() {
        std::optional<T> old_copy;
        std::vector<Observer> to_call;
        std::vector<bool> call_filter;
        AfterChange after;

        bool should_fire = false;

        {
            std::lock_guard lk(m_);
            if (tx_depth_ == 0) return; // на всякий

            --tx_depth_;
            if (tx_depth_ != 0) return; // вложенная транзакция — ждём внешнюю

            // Коммит: если не было изменений — ничего не делаем
            should_fire = tx_dirty_.value_or(false);
            if (!should_fire || !tx_old_) {
                tx_old_.reset();
                tx_dirty_.reset();
                return;
            }

            old_copy = std::move(tx_old_);
            tx_old_.reset();

            after = after_;

            // соберём подписчиков
            to_call.reserve(observers_.size());
            call_filter.reserve(observers_.size());

            for (auto const& [id, entry] : observers_) {
                (void)id;
                const bool equal = entry.equal
                    ? entry.equal(*old_copy, value_)
                    : false;
                const bool skip = entry.distinct && equal;
                to_call.push_back(entry.cb);
                call_filter.push_back(!skip);
            }

            tx_dirty_.reset();
        } // unlock

        // after hook
        if (after && old_copy) {
            T new_copy = get_copy();
            Change ch{*old_copy, new_copy};
            after(ch);
        }

        // notify
        if (old_copy) {
            T new_copy = get_copy();
            Change ch{*old_copy, new_copy};
            for (std::size_t i = 0; i < to_call.size(); ++i) {
                if (call_filter[i]) {
                    to_call[i](ch);
                }
            }
        }
    }

private:
    mutable std::mutex m_;
    T value_{};

    std::unordered_map<std::uint64_t, ObserverEntry> observers_;
    std::uint64_t next_id_ = 1;

    BeforeChange before_;
    AfterChange after_;

    // transaction state
    std::uint32_t tx_depth_ = 0;
    std::optional<T> tx_old_;
    std::optional<bool> tx_dirty_;
};

} // namespace mvvm


#endif //CLEANGRADUATOR_OBSERVABLE_H