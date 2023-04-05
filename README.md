Intended to be a more-or-less drop-in replacement for std::shared_ptr that supports thread-safe access to held pointer. 

PURPOSE :
        I put this together to better facilitate handling the access and potential removal of individual players/npcs from the network messaging system in my game project I'm working on. As all networking stuff is async, I needed a safe and easy way to manage mutex locks on each character, as well as a fool proof way to check if a character is no longer valid and should be removed from the messaging system. As any passing of these characters was already handled via std::shared_ptr, I wanted to put something together that could essentially be ctrl+R'd into the project with little other modifications. 
        
USAGE :
        As stated this is inteded to be a more-or-les drop-in replacement for shared_ptr, and usage should be familiar. The primary external difference will be that std::make_shared<> is replaced with respective owning_ptr make functions. The implementation is designed such that an initial ''owner'' owning_ptr is created, which can then be implicitly upcast into the shared version intended to be passed to others. While nothing else is reliant on the initial owning_owner_ptr continuing to exist, failure to std::move() to another owner, or destructor being called will set the globally accessible ''alive'' boolean to false. 
        
Async blocking access to the held pointer from owner owning_ptr or any of the sharers is supported via calling get_access(). Member function get_access() returns a structure containing a locked mutex that behaves similar to the original owning_ptr in usage, in which the mutex is released once returned structure has left scope. This can be used within a call such as ''myObject.get_access()->MyFunction()'', or held temporarily within scope as ''auto tempaccess = myObject.get_access()'' then further access within scope can use #tempaccess or normal ''myObject->MyFunction()''. Access via the lock is obviously not required, as depending on design lock could have already been obtained upstream and there is no automatic deadlock prevention currently implemented.

As would be expected, implicit upcasts of shared type to their base's is supported just as would be done using std::shared_ptr; while explicit casting is supported by optr::owning_ptr_cast<>() functions.
   optr::owning_ptr_cast_o<>(...)
   optr::owning_ptr_cast_v<>(...)

Additionally, a feature imitation of std::enable_shared_from_this is present which will look for base class ''optr::enable_owning_share_this'' on shared pointer being created. optr::enable_owning_share_this supplies member function shared_from_this().

This has been put together with two versions of the same idea: owning_ptr_v/owning_owner_v && owning_ptr_o/owning_owner_o.
The reasoning for two versions is to either hold a volatile or owned pointer; Volatile pointer referring to the object not being created by owning_ptr but elsewhere(and lifetime managed elsewhere), and Owned pointer functions exactly as a std::shared_ptr would be expected to as far as handling object lifetimes.
   optr::make_owning_owner_v(...) // raw-pointer
   optr::make_owning_owner_o(...) // copy, move, args-list
   
EXAMPLE:
        basic usage example is provided in main.cpp.
