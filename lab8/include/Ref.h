#ifndef REF_H
#define REF_H

template<class T>
class ref;

#include "llvm/Support/Casting.h"

using llvm::cast;
using llvm::cast_or_null;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;
using llvm::isa_and_nonnull;

/// Reference counter to be used as part of a ref-managed struct or class
class ReferenceCounter {
    template<class T>
    friend class ref;

    /// Count how often the object has been referenced.
    unsigned refCount = 0;

    public:
    ReferenceCounter() = default;
    ~ReferenceCounter() = default;

    // Explicitly initialise reference counter with 0 again
    // As this object is part of another object, the copy-constructor
    // might be invoked as part of the other one.
    ReferenceCounter(const ReferenceCounter& ) {}

    /// Returns the number of parallel references of this objects
    /// \return number of references on this object
    unsigned getCount() {return refCount;}

    // Copy assignment operator
    ReferenceCounter &operator=(const ReferenceCounter &a) {
        if (this == &a)
        return *this;
        // The new copy won't be referenced
        refCount = 0;
        return *this;
    }

    // Do not allow move operations for the reference counter
    // as otherwise, references become incorrect.
    ReferenceCounter(ReferenceCounter &&r) noexcept = delete;
    ReferenceCounter &operator=(ReferenceCounter &&other) noexcept = delete;
};

template <class T>
class ref
{
    T *ptr;

public:
    // default constructor: create a NULL reference
    ref() : ptr(nullptr) {}
    ~ref() { dec(); }

private:
    void inc() const
    {
        if (ptr)
            ++ptr->_refCount.refCount;
    }

    void dec() const
    {
        if (ptr && --ptr->_refCount.refCount == 0)
            delete ptr;
    }

public:
    template <class U>
    friend class ref;

    // constructor from pointer
    ref(T *p) : ptr(p)
    {
        inc();
    }

    // normal copy constructor
    ref(const ref<T> &r) : ptr(r.ptr)
    {
        inc();
    }

    // conversion constructor
    template <class U>
    ref(const ref<U> &r) : ptr(r.ptr)
    {
        inc();
    }

    // normal move constructor: invoke the move assignment operator
    ref(ref<T> &&r) noexcept : ptr(nullptr) { *this = std::move(r); }

    // conversion move constructors: invoke the move assignment operator
    template <class U>
    ref(ref<U> &&r) noexcept : ptr(nullptr)
    {
        *this = std::move(r);
    }

    // pointer operations
    T *get() const
    {
        return ptr;
    }

    /* The copy assignment operator must also explicitly be defined,
     * despite a redundant template. */
    ref<T> &operator=(const ref<T> &r)
    {
        r.inc();
        // Create a copy of the pointer as the
        // referenced object might get destroyed by the following dec(),
        // like in the following example:
        // ````````````````````````
        //    Expr {
        //        ref<Expr> next;
        //    }
        //
        //    ref<Expr> root;
        //    root = root->next;
        // ````````````````````````
        T *saved_ptr = r.ptr;
        dec();
        ptr = saved_ptr;

        return *this;
    }

    template <class U>
    ref<T> &operator=(const ref<U> &r)
    {
        r.inc();
        // Create a copy of the pointer as the currently
        // referenced object might get destroyed by the following dec(),
        // like in the following example:
        // ````````````````````````
        //    Expr {
        //        ref<Expr> next;
        //    }
        //
        //    ref<Expr> root;
        //    root = root->next;
        // ````````````````````````

        U *saved_ptr = r.ptr;
        dec();
        ptr = saved_ptr;

        return *this;
    }

    // Move assignment operator
    ref<T> &operator=(ref<T> &&r) noexcept
    {
        if (this == &r)
            return *this;
        dec();
        ptr = r.ptr;
        r.ptr = nullptr;
        return *this;
    }

    // Move assignment operator
    template <class U>
    ref<T> &operator=(ref<U> &&r) noexcept
    {
        if (static_cast<void *>(this) == static_cast<void *>(&r))
            return *this;

        // Do not swap as the types might be not compatible
        // Decrement local counter to not hold reference anymore
        dec();

        // Assign to this ref
        ptr = cast_or_null<T>(r.ptr);

        // Invalidate old ptr
        r.ptr = nullptr;

        // Return this pointer
        return *this;
    }

    T &operator*() const
    {
        return *ptr;
    }

    T *operator->() const
    {
        return ptr;
    }

    bool isNull() const { return ptr == nullptr; }
    explicit operator bool() const noexcept { return !isNull(); }

    // assumes non-null arguments
    int compare(const ref &rhs) const
    {
        assert(!isNull() && !rhs.isNull() && "Invalid call to compare()");
        return get()->compare(*rhs.get());
    }

    // assumes non-null arguments
    bool operator<(const ref &rhs) const { return compare(rhs) < 0; }
    bool operator==(const ref &rhs) const { return compare(rhs) == 0; }
    bool operator!=(const ref &rhs) const { return compare(rhs) != 0; }
};

#endif // REF_H