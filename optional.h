#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    Optional(const T& value)
        : value_(new (&data_[0]) T(value)) {};
    Optional(T&& value)
        : value_(new (&data_[0]) T(std::move(value))) {};
    Optional(const Optional& other) {
        if (other.HasValue()) {
            value_ = new (&data_[0]) T(*other.value_);
        }
    }
    Optional(Optional&& other) {
        if (other.HasValue()) {
            value_ =  new (&data_[0]) T(std::move(*other.value_)) ;
        }
    };

    Optional& operator=(const T& value) {
        if (HasValue()) {
            *value_ = value;
        }
        else {
            value_ = new (&data_[0]) T(value);
        }
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (HasValue()) {
            *value_ = std::move(rhs);
        }
        else {
            value_ = new (&data_[0]) T(std::move(rhs));
        }
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (HasValue() && rhs.HasValue()) {
            *value_ = *rhs.value_;
        }
        else if (HasValue() && !rhs.HasValue()) {
            Reset();
        }
        else if (!HasValue() && rhs.HasValue()) {
            value_ = new (&data_[0]) T(*rhs.value_);
        }

        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (HasValue() && rhs.HasValue()) {
            *value_ = std::move(*rhs.value_);
        }
        else if (HasValue() && !rhs.HasValue()) {
            Reset();
        }
        else if (!HasValue() && rhs.HasValue()) {
            value_ = new (&data_[0]) T(std::move(*rhs.value_));
        }

        return *this;
    }

    ~Optional() { 
        Reset();
    }

    bool HasValue() const {
        return value_ != nullptr;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() {
        return *value_;
    }
    const T& operator*() const {
        return *value_;
    }
    T* operator->() {
        return value_;
    }
    const T* operator->() const {
        return value_;
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value() {
        if (HasValue()) {
            return *value_;
        }
        else {
            throw BadOptionalAccess();
        }
    }
    const T& Value() const {
        if (HasValue()) {
            return *value_;
        }
        else {
            throw BadOptionalAccess();
        }
    }

    void Reset() {
        if (value_ != nullptr) {
            value_->~T();
            value_ = nullptr;
        }
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) std::byte data_[sizeof(T)];
    T* value_{ nullptr };
};
