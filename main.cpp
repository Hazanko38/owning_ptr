#include <iostream>

#include <str_owning_ptr.hpp>

class Class_Base
{
    public:
        Class_Base(const int val_init)
        :
            cValue(val_init)
        {};
        virtual ~Class_Base()
        {};

        int
            Value() const {
            return cValue;
        };

    protected:
        const int cValue;
};

class Class_Derived
:
    public Class_Base
{
    public:
        Class_Derived(const int val_init)
        :
            Class_Base(val_init)
        {};
        virtual ~Class_Derived()
        {};

        int
            Value() const {
            return cValue * 2;
        };
};

int main()
{
    //Create owner of owning_ptr
    auto newowner = optr::make_owning_owner_o<Class_Derived>({20});
    optr::owning_ptr_o<Class_Derived> newowning;

    std::cout << "Owner is alive? -> " << newowner.alive() << "\n";
    std::cout << "Share count -> " << newowning.use_count() << "\n\n";

    {
        //Explicitly move original owner to new
        auto owner2 = std::move(newowner);
        newowning = owner2; //Shared ptr
        std::cout << "Owner is alive (inscope) ? -> " << newowning.alive() << "\n";
        std::cout << "OwningPtr Value -> " << newowning->Value() << "\n";
        std::cout << "Share count -> " << newowning.use_count() << "\n\n";
    }

    std::cout << "Owner is alive (outofscope) ? -> " << newowning.alive() << "\n";
    std::cout << "OwningPtr Value -> " << newowning->Value() << "\n";
    std::cout << "Share count -> " << newowning.use_count() << "\n\n";

    optr::owning_ptr_o<Class_Base> owning_upcast = newowning;

    std::cout << "OwningPtr Upcast thread-safe Value -> " << owning_upcast.get_lock()->Value() << "\n";
    std::cout << "Share count -> " << owning_upcast.use_count() << "\n\n";

    auto owning_downcast = optr::owning_ptr_cast_o<Class_Derived>(owning_upcast);

    std::cout << "Share count -> " << owning_upcast.use_count() << "\n\n";

    owning_upcast = nullptr;

    std::cout << "OwningPtr Downcast thread-safe Value -> " << owning_downcast.get_lock()->Value() << "\n";
    std::cout << "Share count -> " << owning_downcast.use_count() << "\n\n";

    return 0;
}
