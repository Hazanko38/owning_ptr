#ifndef STR_LIFETIME_PTR_HPP
#define STR_LIFETIME_PTR_HPP

#include <atomic>
#include <mutex>

namespace optr
{
    ///Forward declarations
    template <typename OwnedType>
    class owning_ptr_v;
    template <typename PtrCastType>
    class owning_ptr_v;
    template <typename OwnedType>
    class owning_ptr_o;
    template <typename PtrCastType>
    class owning_ptr_o;
    template <typename OwnedType>
    class owning_owner_v;
    template <typename OwnedType>
    class owning_owner_o;

    class enable_owning_share_this;

    ///Make owning_owner_v declaration
    template <typename OwnedType>
    static inline __attribute__((always_inline))
    owning_owner_v<OwnedType>
        make_owning_owner_v(OwnedType* iPtr);

    ///Cast owning_ptr_v declaration
    template <typename PtrCastType, typename OwnedType>
    static inline __attribute__((always_inline))
    owning_ptr_v<PtrCastType>
        owning_ptr_cast_v(const owning_ptr_v<OwnedType>& cOPtr);

    ///Make owning_owner_o friend function ( copy )
    template <typename OwnedType>
    static inline __attribute__((always_inline))
    owning_owner_o<OwnedType>
        make_owning_owner_o(const OwnedType& cpTp);
    ///Make owning_owner_o friend function ( move )
    template <typename OwnedType>
    static inline __attribute__((always_inline))
    owning_owner_o<OwnedType>
        make_owning_owner_o(OwnedType&& mvTp);
    ///Make owning_owner_o in-place
    template <typename OwnedType, typename... Args>
    static inline __attribute__((always_inline))
    owning_owner_o<OwnedType>
        make_owning_owner_o(Args&&... args);

    ///Cast owning_ptr_o decleration
    template <typename PtrCastType, typename OwnedType>
    static inline __attribute__((always_inline))
    owning_ptr_o<PtrCastType>
        owning_ptr_cast_o(const owning_ptr_o<OwnedType>& cOPtr);

    namespace optr_implem
    {
        ///Forward declarations
        template <typename OwnedType>
        class owning_ptr_mutex_lock;
        template <typename RgstrType, typename OwnedType>
        class owning_ptr_base;
        template <typename RgstrType, typename PtrCastType>
        class owning_ptr_base;
    };  // end of optr_implem namespace

};  // end of optr namespace

namespace optr
{
    namespace optr_implem
    {
        ///Using aliases
        using ATM_U_ = std::atomic<size_t>;
        using ATM_B_ = std::atomic<bool>;
        using MTX_   = std::mutex;

        ///-------------------------------------------------------------------------------------------------------
        ///HOLD OWNING_PTR SHARED BASE INFORMATION                                  ------------------------------
        class owning_ptr_register
        {
            public:
                ///Destructor
                virtual ~owning_ptr_register()
                {};

                ///Increment Operator
                inline __attribute__((always_inline))
                owning_ptr_register&
                    operator++()
                {
                    this->share_count++;
                    return *this;
                };
                ///Decrement Operator
                inline __attribute__((always_inline))
                owning_ptr_register&
                    operator--()
                {
                    this->share_count--;
                    return *this;
                };
                ///Equality Operator for share_count
                inline __attribute__((always_inline))
                bool
                    operator==(const size_t scount) const {
                    return this->share_count == scount;
                };

                ATM_B_ b_alive;         ///< Indicates primary owner still 'alive'
                MTX_ mutex_optr;        ///< Shared access mutex lock

            protected:
              ///Init Constructor
                owning_ptr_register()
                :
                    b_alive(false),
                    share_count(0)
                {};

                ATM_U_ share_count;     ///< count of current share-holders

            private:
                /// - deleted
                owning_ptr_register(const owning_ptr_register&) = delete;
                owning_ptr_register(owning_ptr_register&&) = delete;
        };

        ///-------------------------------------------------------------------------------------------------------
        ///HOLD LIVING_PTR SHARED BASE INFORMATION                                  ------------------------------
        class owning_ptr_register_v
        :
            public owning_ptr_register
        {
            template <typename RgstrType, typename OwnedType>
            friend class owning_ptr_base;

            public:
                ///Destructor
                virtual ~owning_ptr_register_v()
                {};

            protected:
                ///Default Constructor
                owning_ptr_register_v()
                {};

                template <typename RgstrType, typename OwnedType>
                static inline __attribute__((always_inline))
                void
                    destroy(RgstrType*& rPtr,
                            OwnedType*& /**NOT USED*/)
                {
                    delete rPtr;    //delete register
                };

            private:
                /// - deleted
                owning_ptr_register_v(const owning_ptr_register_v&) = delete;
                owning_ptr_register_v(owning_ptr_register_v&&) = delete;
        };  // end of owning_ptr_register_v class

        ///-------------------------------------------------------------------------------------------------------
        ///HOLD OWNING_PTR SHARED BASE INFORMATION                          --------------------------------------
        class owning_ptr_register_o
        :
            public owning_ptr_register
        {
            template <typename RgstrType, typename OwnedType>
            friend class owning_ptr_base;

            public:
                ///Destructor
                virtual ~owning_ptr_register_o()
                {};

            protected:
                ///Default Constructor
                owning_ptr_register_o()
                {};

                template <typename RgstrType, typename OwnedType>
                static inline __attribute__((always_inline))
                void
                    destroy(RgstrType*& rPtr,
                            OwnedType*& oPtr)
                {
                    delete rPtr;    //delete register
                    delete oPtr;    //delete held pointer
                };

            private:
                /// - deleted
                owning_ptr_register_o(const owning_ptr_register_o&) = delete;
                owning_ptr_register_o(owning_ptr_register_o&&) = delete;
        };  // end of owning_ptr_register_o class

        ///-------------------------------------------------------------------------------------------------------
        ///PROVIDE POINTER AND LOCK ON LIVING_PTR UNTIL OUT OF SCOPE                    --------------------------
        template <typename OwnedType>
        class owning_ptr_mutex_lock
        {
            ///Using aliases
            using OPTR_TYPE_  = OwnedType;                          ///< shared type
            using OPTR_PTR_   = OPTR_TYPE_*;                        ///< shared type pointer
            using OPTR_LOCK_ = owning_ptr_mutex_lock<OwnedType>;    ///< mutex-locked container

            public:
                ///Constructor ( lock )
                owning_ptr_mutex_lock(MTX_& mtx,
                                      ATM_B_& bAlv,
                                      OPTR_PTR_ ptr)
                :
                    mutex(mtx),
                    b_alive_r(bAlv),
                    ltptr(ptr)
                {
                    mutex.lock();   //< lock mutex while owning_ptr_mutex_lock exists
                };
                ///Destructor ( unlock )
                virtual ~owning_ptr_mutex_lock()
                {
                    mutex.unlock(); //< unlock mutex once owning_ptr_mutex_lock is destroyed
                };

                ///Access Operator
                inline __attribute__((always_inline))
                OPTR_PTR_
                    operator->(){
                    return ltptr;
                };
                ///Access Operator ( const )
                inline __attribute__((always_inline))
                OPTR_PTR_
                    operator->() const {
                    return ltptr;
                };

                ///Owner-Alive status
                inline __attribute__((always_inline))
                bool
                    alive() const {
                    return b_alive_r;
                };

            private:
                MTX_& mutex;                ///< optr_register mutex
                ATM_B_& b_alive_r;          ///< Original owner is-alive bool
                OPTR_TYPE_* const ltptr;    ///< optr shared pointer

                /// - deleted
                owning_ptr_mutex_lock() = delete;
                owning_ptr_mutex_lock(owning_ptr_mutex_lock&) = delete;
        };  // end of owning_ptr_mutex_lock class

        ///-------------------------------------------------------------------------------------------------------
        ///OWNING_PTR BASE STRUCTURE CONTAINING REGISTER AND POINTER TO OWNEDTYPE               ------------------
        template <typename RgstrType, typename OwnedType>
        class owning_ptr_base
        {
            protected:
                ///Base info struct using alias
                using OPTR_RGSTR_ = RgstrType;                                      ///< owned_ptr register type
                using OPTR_TYPE_  = OwnedType;                                      ///< shared type
                using OPTR_PTR_   = OPTR_TYPE_*;                                    ///< shared type pointer
                using OPTR_PTR_R_ = OPTR_TYPE_*&;
                using OPTR_REF_   = OPTR_TYPE_&;                                    ///< shared type ref
                using OPTR_C_REF_ = const OPTR_TYPE_&;                              ///< shared type const ref
                using OPTR_M_REF_ = OPTR_TYPE_&&;                                   ///< shared type move ref
                using OPTR_LOCK_  = optr_implem::owning_ptr_mutex_lock<OPTR_TYPE_>; ///< mutex-locked container

                ///Friend cast template declare
                template <typename T, typename PtrCastType>
                friend class owning_ptr_base;

                typedef typename optr_implem::owning_ptr_register_v OPTR_RGSTR_V_;      ///< volatile register
                typedef typename optr_implem::owning_ptr_register_o OPTR_RGSTR_O_;      ///< owned register

            public:
                ///Empty Constructor
                inline __attribute__((always_inline))
                owning_ptr_base()
                :
                    o_register(nullptr),
                    o_pointer(nullptr)
                {};
                ///Destructor
                virtual ~owning_ptr_base()
                {
                    clean_base();   //decrement share_count & remove register if last
                };

                ///DeReference Operator
                inline __attribute__((always_inline))
                OPTR_REF_
                    operator*(){
                    return *(this->o_pointer);
                };
                ///Access Operator
                inline __attribute__((always_inline))
                OPTR_PTR_
                    operator->(){
                    return this->o_pointer;
                };
                ///Access Operator ( const )
                inline __attribute__((always_inline))
                OPTR_PTR_
                    operator->() const {
                    return this->o_pointer;
                };
                ///Get Raw Owning_Pointer
                inline __attribute__((always_inline))
                OPTR_PTR_
                    get(){
                    return this->o_pointer;
                };
                ///Get Raw Owning_Pointer (const)
                inline __attribute__((always_inline))
                OPTR_PTR_
                    get() const {
                    return this->o_pointer;
                };

                ///Assignment Operator
                inline __attribute__((always_inline))
                void
                    operator=(const owning_ptr_base& lPtr)
                {
                    if ( this->o_register != lPtr.o_register )
                        clean_base();   //decrement share_count & remove register if last

                    this->o_register = lPtr.o_register;
                    this->o_pointer = lPtr.o_pointer;

                    bump_up();      //increment share_count
                };

                ///Equality Operator
                inline __attribute__((always_inline))
                bool
                    operator==(const owning_ptr_base& eq) const {
                    return this->o_register == eq.o_register;
                };
                ///InEquality Operator
                inline __attribute__((always_inline))
                bool
                    operator!=(const owning_ptr_base& ieq) const {
                    return this->o_register != ieq.o_register;
                };
                ///Equality Operator ( nullpointer )
                inline __attribute__((always_inline))
                bool
                    operator==(std::nullptr_t){
                    return this->o_pointer == nullptr;
                };
                ///InEquality Operator ( nullpointer )
                inline __attribute__((always_inline))
                bool
                    operator!=(std::nullptr_t){
                    return this->o_pointer != nullptr;
                };

                ///Returns mutex-locked container with shared pointer
                inline __attribute__((always_inline))
                OPTR_LOCK_
                    get_lock(){
                    return OPTR_LOCK_{ this->o_register->mutex_optr,
                                       this->o_register->b_alive,
                                       this->o_pointer };
                };
                ///Returns mutex-locked container with shared pointer ( const )
                inline __attribute__((always_inline))
                OPTR_LOCK_
                    get_lock() const {
                    return OPTR_LOCK_{ this->o_register->mutex_optr,
                                       this->o_register->b_alive,
                                       this->o_pointer };
                };

                ///Get owner b_alive status ( false if original owner no longer exists )
                inline __attribute__((always_inline))
                bool
                    alive() const
                {
                    if ( this->o_register == nullptr )
                        return false;   //has not been made

                    return this->o_register->b_alive;
                };

                inline __attribute__((always_inline))
                size_t
                    use_count() const
                {
                    if ( this->o_register == nullptr )
                        return 0;

                    return this->o_register->share_count;
                };

            protected:
                ///Share Constructor
                inline __attribute__((always_inline))
                owning_ptr_base(const owning_ptr_base& cp)
                :
                    o_register(cp.o_register),
                    o_pointer(cp.o_pointer)
                {
                    bump_up();      //increment share_count
                };
                ///Share Cast Constructor ( up-cast )
                template <typename PtrCastType>
                inline __attribute__((always_inline))
                owning_ptr_base(const owning_ptr_base<RgstrType, PtrCastType>& c_cp)
                :
                    o_register(c_cp.o_register),
                    o_pointer(c_cp.o_pointer)
                {
                    bump_up();      //increment share_count
                };
                ///Share Cast Constructor ( down-cast )
                template <typename PtrCastType>
                inline __attribute__((always_inline))
                owning_ptr_base(const owning_ptr_base<RgstrType, PtrCastType>& c_cp,
                                OPTR_PTR_ iPtr)
                :
                    o_register(c_cp.o_register),
                    o_pointer(iPtr)
                {
                    bump_up();      //increment share_count
                };

                ///owning_owner initialize Constructor
                inline __attribute__((always_inline))
                owning_ptr_base(OPTR_PTR_R_ oPtr)
                {
                    this->o_register = new OPTR_RGSTR_;     //make initial o_register ( volatile )

                    this->o_pointer = oPtr;                 //Assign held ptr
                    this->o_register->operator++();         //increment share_count

                    check_enable_share_this();  //Check for enable_owning_share_this
                };

                ///Check for enable_owning_share_this and set pointer if exists
                ///- ( invalidated if owner no longer exists )
                inline constexpr __attribute__((always_inline))
                void
                    check_enable_share_this()
                {
                    constexpr bool has_sharethis = std::is_base_of<enable_owning_share_this, OPTR_TYPE_>::value;
                    if constexpr ( has_sharethis )
                        this->o_pointer->o_sharethis = this; //set new optr_type shared internal pointer
                };

                ///Base Pointers
                OPTR_RGSTR_* o_register = nullptr;  ///< pointer to shared register
                OPTR_TYPE_* o_pointer   = nullptr;  ///< pointer to shared resource

            private:
                ///Increase share_count if needed
                inline __attribute__((always_inline))
                void
                    bump_up()
                {
                    if ( this->o_register != nullptr )
                        this->o_register->operator++();  //increment share_count
                };
                ///Clean o_register share_count if last
                inline __attribute__((always_inline))
                void
                    clean_base()
                {
                    if ( this->o_register == nullptr )
                        return;         //has not been made

                    this->o_register->operator--(); //decrement share_count

                    if ( this->o_register->operator==(0) ) //no more share-holders
                        this->o_register->template destroy<RgstrType, OwnedType>(this->o_register,
                                                                                 this->o_pointer);
                };

                /// virtual - deleted
                virtual void
                    operator=(std::nullptr_t) const = delete;
                /// - deleted
                owning_ptr_base(owning_ptr_base&&) = delete;
        };

    };  // end of optr_implem namespace

    ///-------------------------------------------------------------------------------------------------------
    ///owning_ptr class                 ----------------------------------------------------------------------
    template <typename OwnedType>
    class owning_ptr_v
    :
        public optr_implem::owning_ptr_base<optr_implem::owning_ptr_register_v, OwnedType>
    {
        protected:
            //register_volatile typedef
            typedef optr_implem::owning_ptr_base<optr_implem::owning_ptr_register_v, OwnedType> OPTR_BASE_;
            typedef typename OPTR_BASE_::OPTR_LOCK_ OPTR_LOCK_;

            ///Cast owning_ptr_v friend function
            template <typename PtrCastType, typename OT>
            friend owning_ptr_v<PtrCastType>
                owning_ptr_cast_v(const owning_ptr_v<OT>& cOPtr);

        public:
            ///Empty Constructor
            owning_ptr_v()
            :
                OPTR_BASE_()
            {};
            ///Nullptr Constructor
            owning_ptr_v(std::nullptr_t)
            :
                OPTR_BASE_()
            {};
            ///Share Constructor
            owning_ptr_v(const owning_ptr_v& cp)
            :
                OPTR_BASE_(cp)
            {};
            ///Share Cast Constructor
            template <typename PtrCastType>
            owning_ptr_v(const owning_ptr_v<PtrCastType>& c_cp)
            :
                OPTR_BASE_(c_cp)
            {};

            ///Destructor
            virtual ~owning_ptr_v()
            {};

            ///Assignment Operator
            inline __attribute__((always_inline))
            void
                operator=(const owning_ptr_v& ass){
                OPTR_BASE_::operator=(ass);
            };

        protected:
            ///make_owning_owner_v initialPtr Constructor
            inline __attribute__((always_inline))
            owning_ptr_v(OwnedType* iPtr)
            :
                OPTR_BASE_(iPtr)
            {};

        private:
            ///Share Cast Constructor
            template <typename PtrCastType>
            owning_ptr_v(const owning_ptr_v<PtrCastType>& c_cp,
                       OwnedType* iPtr)
            :
                OPTR_BASE_(c_cp, iPtr)
            {};
    };  // end of owning_ptr_v class

    ///-------------------------------------------------------------------------------------------------------
    ///owning_owner_v class                               ------------------------------------------------------
    template <typename OwnedType>
    class owning_owner_v
    :
        public owning_ptr_v<OwnedType>
    {
        protected:
            typedef typename owning_ptr_v<OwnedType>::OPTR_BASE_ OPTR_BASE_ ;

            ///Make owning_owner_v friend function
            friend owning_owner_v<OwnedType>
                optr::make_owning_owner_v<OwnedType>(OwnedType* iPtr);

        public:
            ///Default Constructor
            owning_owner_v()
            :
                owning_ptr_v<OwnedType>()
            {};
            ///Nullptr Constructor
            owning_owner_v(std::nullptr_t)
            :
                owning_ptr_v<OwnedType>()
            {};
            ///Move Constructor
            owning_owner_v(owning_owner_v&& mv)
            :
                owning_ptr_v<OwnedType>(){
                owning_owner_v::operator=(std::move(mv));
            };

            ///Destructor
            virtual ~owning_owner_v()
            {
                this->o_register->b_alive = false;
            };

            ///Assignment Operator
            inline __attribute__((always_inline))
            void
                operator=(const owning_owner_v& ass){
                OPTR_BASE_::operator=(ass);
            };
            ///Assignment Move Operator
            inline __attribute__((always_inline))
            void
                operator=(owning_owner_v&& mass)
            {
                this->o_register = mass.o_register;
                this->o_pointer = mass.o_pointer;

                mass.o_register = nullptr;

                OPTR_BASE_::check_enable_share_this();  //Check for enable_owning_share_this to update this
            };
            ///NullPtr Assignment Operator
            inline __attribute__((always_inline))
            void
                operator=(std::nullptr_t){
                OPTR_BASE_::operator=(OPTR_BASE_());
            };

        private:
            ///Make Constructor
            owning_owner_v(OwnedType* iPtr)
            :
                owning_ptr_v<OwnedType>(iPtr)
            {
                this->o_register->b_alive = true;
            };

            owning_owner_v(const owning_owner_v&) = delete;
    };  // end of owning_owner_v class

    ///-------------------------------------------------------------------------------------------------------
    ///owning_ptr class                     ------------------------------------------------------------------
    template <typename OwnedType>
    class owning_ptr_o
    :
        public optr_implem::owning_ptr_base<optr_implem::owning_ptr_register_o, OwnedType>
    {
        protected:
            //register_owned typedef
            typedef optr_implem::owning_ptr_base<optr_implem::owning_ptr_register_o, OwnedType> OPTR_BASE_;
            typedef typename OPTR_BASE_::OPTR_LOCK_ OPTR_LOCK_;

            ///Cast owning_ptr_o friend function
            template <typename PtrCastType, typename OT>
            friend owning_ptr_o<PtrCastType>
                owning_ptr_cast_o(const owning_ptr_o<OT>& cOPtr);

        public:
            ///Empty Constructor
            owning_ptr_o()
            :
                OPTR_BASE_()
            {};
            ///Nullptr Constructor
            owning_ptr_o(std::nullptr_t)
            :
                OPTR_BASE_()
            {};
            ///Share Constructor
            owning_ptr_o(const owning_ptr_o& cp)
            :
                OPTR_BASE_(cp)
            {};
            ///Share Cast Constructor
            template <typename PtrCastType>
            owning_ptr_o(const owning_ptr_o<PtrCastType>& c_cp)
            :
                OPTR_BASE_(c_cp)
            {};

            ///Destructor
            virtual ~owning_ptr_o()
            {};

            ///Assignment Operator
            inline __attribute__((always_inline))
            void
                operator=(const owning_ptr_o& ass){
                OPTR_BASE_::operator=(ass);
            };
            ///NullPtr Assignment Operator
            inline __attribute__((always_inline))
            void
                operator=(std::nullptr_t){
                OPTR_BASE_::operator=(OPTR_BASE_());
            };

        protected:
            ///make_owning_owner_o initial Constructor ( Copy )
            inline __attribute__((always_inline))
            owning_ptr_o(const OwnedType& cp)
            :
                OPTR_BASE_(cp)
            {};
            ///make_owning_owner_o initial Constructor ( Move )
            inline __attribute__((always_inline))
            owning_ptr_o(OwnedType&& mv)
            :
                OPTR_BASE_(std::move(mv))
            {};
            ///make_owning_owner_o initial Constructor ( New )
            inline __attribute__((always_inline))
            owning_ptr_o(OwnedType*& newOPtr)
            :
                OPTR_BASE_(newOPtr)
            {};

        private:
            ///Share Cast Constructor
            template <typename PtrCastType>
            owning_ptr_o(const owning_ptr_o<PtrCastType>& c_cp,
                         OwnedType* iPtr)
            :
                OPTR_BASE_(c_cp, iPtr)
            {};

    };  // end of owning_ptr_o class
    ///-------------------------------------------------------------------------------------------------------
    ///owning_owner_o class                       --------------------------------------------------------------
    template <typename OwnedType>
    class owning_owner_o
    :
        public owning_ptr_o<OwnedType>
    {
        protected:
            typedef typename owning_ptr_o<OwnedType>::OPTR_BASE_ OPTR_BASE_;

            ///Make owning_owner_o friend function ( Copy )
            friend owning_owner_o<OwnedType>
                optr::make_owning_owner_o<OwnedType>(const OwnedType& cpTp);
            ///Make owning_owner_o friend function ( Move )
            friend owning_owner_o<OwnedType>
                optr::make_owning_owner_o<OwnedType>(OwnedType&& mvTp);
            ///Make owning_owner_o friend function ( New )
            template <typename T, typename... Args>
            friend owning_owner_o<T>
                optr::make_owning_owner_o(Args&&... args);

        public:
            ///Default Constructor
            owning_owner_o()
            :
                owning_ptr_o<OwnedType>()
            {};
            ///Nullptr Constructor
            owning_owner_o(std::nullptr_t)
            :
                owning_ptr_o<OwnedType>()
            {};
            ///Move Constructor
            owning_owner_o(owning_owner_o&& mv)
            :
                owning_ptr_o<OwnedType>(){
                owning_owner_o::operator=(std::move(mv));
            };

            ///Destructor
            virtual ~owning_owner_o()
            {
                if ( this->o_register != nullptr )
                    this->o_register->b_alive = false;
            };

            ///Assignment Operator
            inline __attribute__((always_inline))
            void
                operator=(const owning_owner_o& ass){
                OPTR_BASE_::operator=(ass);
            };
            ///Assignment Move Operator
            inline __attribute__((always_inline))
            void
                operator=(owning_owner_o&& mass)
            {
                this->o_register = mass.o_register;
                this->o_pointer = mass.o_pointer;

                mass.o_register = nullptr;

                OPTR_BASE_::check_enable_share_this();  //Check for enable_owning_share_this
            };

            ///NullPtr Assignment Operator
            inline __attribute__((always_inline))
            void
                operator=(std::nullptr_t){
                OPTR_BASE_::operator=(OPTR_BASE_());
            };

        private:
            ///Copy Constructor
            owning_owner_o(const OwnedType& cp)
            :
                owning_ptr_o<OwnedType>(cp)
            {
                this->o_register->b_alive = true;
            };
            ///Move Constructor
            owning_owner_o(OwnedType&& mv)
            :
                owning_ptr_o<OwnedType>(std::move(mv))
            {
                this->o_register->b_alive = true;
            };
            ///New Constructor
            owning_owner_o(OwnedType*& newOPtr)
            :
                owning_ptr_o<OwnedType>(newOPtr)
            {
                this->o_register->b_alive = true;
            };

            /// - deleted
            owning_owner_o(const owning_owner_o&) = delete;
    };  // end of owning_owner_o class

    ///-------------------------------------------------------------------------------------------------------
    ///Inherited class to enable sharing owning_ptr from 'this'                     --------------------------
    class enable_owning_share_this
    {
        template <typename RgstrType, typename OwnedType>
        friend class optr_implem::owning_ptr_base;

        protected:
            ///Retrieve shared_from_this owning_ptr
            template <typename OwningType>
            inline __attribute__((always_inline))
            OwningType
                shared_from_this()
            {
                auto ocast = static_cast<OwningType*>(o_sharethis);
                return OwningType(*ocast);
            };
            ///Retrieve shared_from_this owning_ptr ( const )
            template <typename OwningType>
            inline __attribute__((always_inline))
            const OwningType
                shared_from_this() const
            {
                auto ocast = static_cast<OwningType*>(o_sharethis);
                return OwningType(*ocast);
            };

        private:
            void* o_sharethis = nullptr;    ///< pointer to owning_owner
    };

    ///-------------------------------------------------------------------------------------------------------
    ///-------------------------------------------------------------------------------------------------------
    /// ---------- ( HELPER FUNCTIONS ) -------- ( HELPER FUNCTIONS ) -------- ( HELPER FUNCTIONS ) ----------

    ///-------------------------------------------------------------------------------------------------------
    ///Make owning_ptr_v                                    --------------------------------------------------
    template <typename OwnedType>
    static inline __attribute__((always_inline))
    owning_owner_v<OwnedType>
        make_owning_owner_v(OwnedType* iPtr){
        return owning_owner_v<OwnedType>{iPtr};
    };

    ///-------------------------------------------------------------------------------------------------------
    ///Cast owning_ptr_v                                    --------------------------------------------------
    template <typename PtrCastType, typename OwnedType>
    static inline __attribute__((always_inline))
    owning_ptr_v<PtrCastType>
        owning_ptr_cast_v(const owning_ptr_v<OwnedType>& cOPtr)
    {
        auto p = static_cast<PtrCastType*>(cOPtr.get());
        return owning_ptr_v<PtrCastType>{cOPtr, p};
    };

    ///-------------------------------------------------------------------------------------------------------
    ///Make owning_owner_o friend function ( copy )                             ------------------------------
    template <typename OwnedType>
     static inline __attribute__((always_inline))
     owning_owner_o<OwnedType>
        make_owning_owner_o(const OwnedType& cpTp)
    {
        auto newoptr = new OwnedType(cpTp);     //get ptr from register
        return owning_owner_o<OwnedType>(newoptr);
    };
    ///-------------------------------------------------------------------------------------------------------
    ///Make owning_owner_o friend function ( move )                             ------------------------------
    template <typename OwnedType>
    static inline __attribute__((always_inline))
    owning_owner_o<OwnedType>
        make_owning_owner_o(OwnedType&& mvTp)
    {
        auto newoptr = new OwnedType(std::move(mvTp));
        return owning_owner_o<OwnedType>(newoptr);
     };
    ///Make owning_owner_o in-place
    template <typename OwnedType, typename... Args>
    static inline __attribute__((always_inline))
    owning_owner_o<OwnedType>
        make_owning_owner_o(Args&&... args)
    {
        auto newoptr = new OwnedType(args...);
        return owning_owner_o<OwnedType>(newoptr);
    };

    ///-------------------------------------------------------------------------------------------------------
    ///Cast owning_ptr_o                                    --------------------------------------------------
    template <typename PtrCastType, typename OwnedType>
    static inline __attribute__((always_inline))
    owning_ptr_o<PtrCastType>
        owning_ptr_cast_o(const owning_ptr_o<OwnedType>& cOPtr)
    {
        auto p = static_cast<PtrCastType*>(cOPtr.get());
        return owning_ptr_o<PtrCastType>{cOPtr, p};
    };

};  // end of optr namespace

#endif // STR_LIFETIME_PTR_HPP
