// Utility.hpp

//---------------------------------------------------------------------------//

template <class T, typename Name>
interface ICollection : public IUnknown
{
    virtual T*     __stdcall at(size_t index) const = 0;
    virtual Name&  __stdcall name(size_t index) const = 0;
    virtual size_t __stdcall size() const = 0;

    virtual void   __stdcall Append(const Name& name) = 0;
    virtual void   __stdcall Remove(const Name& name) = 0;

    virtual void   __stdcall Append(const T* t) = 0;
    virtual void   __stdcall Remove(const T* t) = 0;

    virtual T*     __stdcall Find(const Name& name, ...) = 0;
};

//---------------------------------------------------------------------------//

// Utility.hpp