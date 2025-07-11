// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: Stc.proto
// Protobuf C++ Version: 5.27.3

#ifndef GOOGLE_PROTOBUF_INCLUDED_Stc_2eproto_2epb_2eh
#define GOOGLE_PROTOBUF_INCLUDED_Stc_2eproto_2epb_2eh

#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "google/protobuf/runtime_version.h"
#if PROTOBUF_VERSION != 5027003
#error "Protobuf C++ gencode is built with an incompatible version of"
#error "Protobuf C++ headers/runtime. See"
#error "https://protobuf.dev/support/cross-version-runtime-guarantee/#cpp"
#endif
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/arena.h"
#include "google/protobuf/arenastring.h"
#include "google/protobuf/generated_message_tctable_decl.h"
#include "google/protobuf/generated_message_util.h"
#include "google/protobuf/metadata_lite.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/repeated_field.h"  // IWYU pragma: export
#include "google/protobuf/extension_set.h"  // IWYU pragma: export
#include "google/protobuf/generated_enum_util.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"

#define PROTOBUF_INTERNAL_EXPORT_Stc_2eproto

namespace google {
namespace protobuf {
namespace internal {
class AnyMetadata;
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// Internal implementation detail -- do not use these members.
struct TableStruct_Stc_2eproto {
  static const ::uint32_t offsets[];
};
namespace RatkiniaProtocol {
class LoginResponse;
struct LoginResponseDefaultTypeInternal;
extern LoginResponseDefaultTypeInternal _LoginResponse_default_instance_;
class RegisterResponse;
struct RegisterResponseDefaultTypeInternal;
extern RegisterResponseDefaultTypeInternal _RegisterResponse_default_instance_;
}  // namespace RatkiniaProtocol
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

namespace RatkiniaProtocol {
enum RegisterResponse_FailedReason : int {
  RegisterResponse_FailedReason_Success = 0,
  RegisterResponse_FailedReason_ExistingUserId = 1,
  RegisterResponse_FailedReason_WrongPassword = 2,
  RegisterResponse_FailedReason_RegisterResponse_FailedReason_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  RegisterResponse_FailedReason_RegisterResponse_FailedReason_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool RegisterResponse_FailedReason_IsValid(int value);
extern const uint32_t RegisterResponse_FailedReason_internal_data_[];
constexpr RegisterResponse_FailedReason RegisterResponse_FailedReason_FailedReason_MIN = static_cast<RegisterResponse_FailedReason>(0);
constexpr RegisterResponse_FailedReason RegisterResponse_FailedReason_FailedReason_MAX = static_cast<RegisterResponse_FailedReason>(2);
constexpr int RegisterResponse_FailedReason_FailedReason_ARRAYSIZE = 2 + 1;
const std::string& RegisterResponse_FailedReason_Name(RegisterResponse_FailedReason value);
template <typename T>
const std::string& RegisterResponse_FailedReason_Name(T value) {
  static_assert(std::is_same<T, RegisterResponse_FailedReason>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to FailedReason_Name().");
  return RegisterResponse_FailedReason_Name(static_cast<RegisterResponse_FailedReason>(value));
}
bool RegisterResponse_FailedReason_Parse(absl::string_view name, RegisterResponse_FailedReason* value);

// ===================================================================


// -------------------------------------------------------------------

class RegisterResponse final : public ::google::protobuf::MessageLite
/* @@protoc_insertion_point(class_definition:RatkiniaProtocol.RegisterResponse) */ {
 public:
  inline RegisterResponse() : RegisterResponse(nullptr) {}
  ~RegisterResponse() override;
  template <typename = void>
  explicit PROTOBUF_CONSTEXPR RegisterResponse(
      ::google::protobuf::internal::ConstantInitialized);

  inline RegisterResponse(const RegisterResponse& from) : RegisterResponse(nullptr, from) {}
  inline RegisterResponse(RegisterResponse&& from) noexcept
      : RegisterResponse(nullptr, std::move(from)) {}
  inline RegisterResponse& operator=(const RegisterResponse& from) {
    CopyFrom(from);
    return *this;
  }
  inline RegisterResponse& operator=(RegisterResponse&& from) noexcept {
    if (this == &from) return *this;
    if (GetArena() == from.GetArena()
#ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetArena() != nullptr
#endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const std::string& unknown_fields() const
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString);
  }
  inline std::string* mutable_unknown_fields()
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.mutable_unknown_fields<std::string>();
  }

  static const RegisterResponse& default_instance() {
    return *internal_default_instance();
  }
  static inline const RegisterResponse* internal_default_instance() {
    return reinterpret_cast<const RegisterResponse*>(
        &_RegisterResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages = 1;
  friend void swap(RegisterResponse& a, RegisterResponse& b) { a.Swap(&b); }
  inline void Swap(RegisterResponse* other) {
    if (other == this) return;
#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() != nullptr && GetArena() == other->GetArena()) {
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() == other->GetArena()) {
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::google::protobuf::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(RegisterResponse* other) {
    if (other == this) return;
    ABSL_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  RegisterResponse* New(::google::protobuf::Arena* arena = nullptr) const final {
    return ::google::protobuf::MessageLite::DefaultConstruct<RegisterResponse>(arena);
  }
  void CheckTypeAndMergeFrom(
      const ::google::protobuf::MessageLite& from) final;
  void CopyFrom(const RegisterResponse& from);
  void MergeFrom(const RegisterResponse& from);
  bool IsInitialized() const {
    return true;
  }
  ABSL_ATTRIBUTE_REINITIALIZES void Clear() final;
  ::size_t ByteSizeLong() const final;
  ::uint8_t* _InternalSerialize(
      ::uint8_t* target,
      ::google::protobuf::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::google::protobuf::Arena* arena);
  void SharedDtor();
  void InternalSwap(RegisterResponse* other);
 private:
  friend class ::google::protobuf::internal::AnyMetadata;
  static ::absl::string_view FullMessageName() { return "RatkiniaProtocol.RegisterResponse"; }

 protected:
  explicit RegisterResponse(::google::protobuf::Arena* arena);
  RegisterResponse(::google::protobuf::Arena* arena, const RegisterResponse& from);
  RegisterResponse(::google::protobuf::Arena* arena, RegisterResponse&& from) noexcept
      : RegisterResponse(arena) {
    *this = ::std::move(from);
  }
  const ::google::protobuf::MessageLite::ClassData* GetClassData() const final;

 public:
  // nested types ----------------------------------------------------
  using FailedReason = RegisterResponse_FailedReason;
  static constexpr FailedReason Success = RegisterResponse_FailedReason_Success;
  static constexpr FailedReason ExistingUserId = RegisterResponse_FailedReason_ExistingUserId;
  static constexpr FailedReason WrongPassword = RegisterResponse_FailedReason_WrongPassword;
  static inline bool FailedReason_IsValid(int value) {
    return RegisterResponse_FailedReason_IsValid(value);
  }
  static constexpr FailedReason FailedReason_MIN = RegisterResponse_FailedReason_FailedReason_MIN;
  static constexpr FailedReason FailedReason_MAX = RegisterResponse_FailedReason_FailedReason_MAX;
  static constexpr int FailedReason_ARRAYSIZE = RegisterResponse_FailedReason_FailedReason_ARRAYSIZE;
  template <typename T>
  static inline const std::string& FailedReason_Name(T value) {
    return RegisterResponse_FailedReason_Name(value);
  }
  static inline bool FailedReason_Parse(absl::string_view name, FailedReason* value) {
    return RegisterResponse_FailedReason_Parse(name, value);
  }

  // accessors -------------------------------------------------------
  enum : int {
    kFailedReasonFieldNumber = 1,
  };
  // .RatkiniaProtocol.RegisterResponse.FailedReason failed_reason = 1;
  void clear_failed_reason() ;
  ::RatkiniaProtocol::RegisterResponse_FailedReason failed_reason() const;
  void set_failed_reason(::RatkiniaProtocol::RegisterResponse_FailedReason value);

  private:
  ::RatkiniaProtocol::RegisterResponse_FailedReason _internal_failed_reason() const;
  void _internal_set_failed_reason(::RatkiniaProtocol::RegisterResponse_FailedReason value);

  public:
  // @@protoc_insertion_point(class_scope:RatkiniaProtocol.RegisterResponse)
 private:
  class _Internal;
  friend class ::google::protobuf::internal::TcParser;
  static const ::google::protobuf::internal::TcParseTable<
      0, 1, 0,
      0, 2>
      _table_;

  static constexpr const void* _raw_default_instance_ =
      &_RegisterResponse_default_instance_;

  friend class ::google::protobuf::MessageLite;
  friend class ::google::protobuf::Arena;
  template <typename T>
  friend class ::google::protobuf::Arena::InternalHelper;
  using InternalArenaConstructable_ = void;
  using DestructorSkippable_ = void;
  struct Impl_ {
    inline explicit constexpr Impl_(
        ::google::protobuf::internal::ConstantInitialized) noexcept;
    inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                          ::google::protobuf::Arena* arena);
    inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                          ::google::protobuf::Arena* arena, const Impl_& from,
                          const RegisterResponse& from_msg);
    int failed_reason_;
    mutable ::google::protobuf::internal::CachedSize _cached_size_;
    PROTOBUF_TSAN_DECLARE_MEMBER
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_Stc_2eproto;
};
// -------------------------------------------------------------------

class LoginResponse final : public ::google::protobuf::MessageLite
/* @@protoc_insertion_point(class_definition:RatkiniaProtocol.LoginResponse) */ {
 public:
  inline LoginResponse() : LoginResponse(nullptr) {}
  ~LoginResponse() override;
  template <typename = void>
  explicit PROTOBUF_CONSTEXPR LoginResponse(
      ::google::protobuf::internal::ConstantInitialized);

  inline LoginResponse(const LoginResponse& from) : LoginResponse(nullptr, from) {}
  inline LoginResponse(LoginResponse&& from) noexcept
      : LoginResponse(nullptr, std::move(from)) {}
  inline LoginResponse& operator=(const LoginResponse& from) {
    CopyFrom(from);
    return *this;
  }
  inline LoginResponse& operator=(LoginResponse&& from) noexcept {
    if (this == &from) return *this;
    if (GetArena() == from.GetArena()
#ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetArena() != nullptr
#endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const std::string& unknown_fields() const
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.unknown_fields<std::string>(::google::protobuf::internal::GetEmptyString);
  }
  inline std::string* mutable_unknown_fields()
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.mutable_unknown_fields<std::string>();
  }

  static const LoginResponse& default_instance() {
    return *internal_default_instance();
  }
  static inline const LoginResponse* internal_default_instance() {
    return reinterpret_cast<const LoginResponse*>(
        &_LoginResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages = 0;
  friend void swap(LoginResponse& a, LoginResponse& b) { a.Swap(&b); }
  inline void Swap(LoginResponse* other) {
    if (other == this) return;
#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() != nullptr && GetArena() == other->GetArena()) {
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() == other->GetArena()) {
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::google::protobuf::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(LoginResponse* other) {
    if (other == this) return;
    ABSL_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  LoginResponse* New(::google::protobuf::Arena* arena = nullptr) const final {
    return ::google::protobuf::MessageLite::DefaultConstruct<LoginResponse>(arena);
  }
  void CheckTypeAndMergeFrom(
      const ::google::protobuf::MessageLite& from) final;
  void CopyFrom(const LoginResponse& from);
  void MergeFrom(const LoginResponse& from);
  bool IsInitialized() const {
    return true;
  }
  ABSL_ATTRIBUTE_REINITIALIZES void Clear() final;
  ::size_t ByteSizeLong() const final;
  ::uint8_t* _InternalSerialize(
      ::uint8_t* target,
      ::google::protobuf::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::google::protobuf::Arena* arena);
  void SharedDtor();
  void InternalSwap(LoginResponse* other);
 private:
  friend class ::google::protobuf::internal::AnyMetadata;
  static ::absl::string_view FullMessageName() { return "RatkiniaProtocol.LoginResponse"; }

 protected:
  explicit LoginResponse(::google::protobuf::Arena* arena);
  LoginResponse(::google::protobuf::Arena* arena, const LoginResponse& from);
  LoginResponse(::google::protobuf::Arena* arena, LoginResponse&& from) noexcept
      : LoginResponse(arena) {
    *this = ::std::move(from);
  }
  const ::google::protobuf::MessageLite::ClassData* GetClassData() const final;

 public:
  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------
  enum : int {
    kFailureReasonFieldNumber = 2,
    kSuccessfulFieldNumber = 1,
  };
  // string failure_reason = 2;
  void clear_failure_reason() ;
  const std::string& failure_reason() const;
  template <typename Arg_ = const std::string&, typename... Args_>
  void set_failure_reason(Arg_&& arg, Args_... args);
  std::string* mutable_failure_reason();
  PROTOBUF_NODISCARD std::string* release_failure_reason();
  void set_allocated_failure_reason(std::string* value);

  private:
  const std::string& _internal_failure_reason() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_failure_reason(
      const std::string& value);
  std::string* _internal_mutable_failure_reason();

  public:
  // bool successful = 1;
  void clear_successful() ;
  bool successful() const;
  void set_successful(bool value);

  private:
  bool _internal_successful() const;
  void _internal_set_successful(bool value);

  public:
  // @@protoc_insertion_point(class_scope:RatkiniaProtocol.LoginResponse)
 private:
  class _Internal;
  friend class ::google::protobuf::internal::TcParser;
  static const ::google::protobuf::internal::TcParseTable<
      1, 2, 0,
      53, 2>
      _table_;

  static constexpr const void* _raw_default_instance_ =
      &_LoginResponse_default_instance_;

  friend class ::google::protobuf::MessageLite;
  friend class ::google::protobuf::Arena;
  template <typename T>
  friend class ::google::protobuf::Arena::InternalHelper;
  using InternalArenaConstructable_ = void;
  using DestructorSkippable_ = void;
  struct Impl_ {
    inline explicit constexpr Impl_(
        ::google::protobuf::internal::ConstantInitialized) noexcept;
    inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                          ::google::protobuf::Arena* arena);
    inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                          ::google::protobuf::Arena* arena, const Impl_& from,
                          const LoginResponse& from_msg);
    ::google::protobuf::internal::ArenaStringPtr failure_reason_;
    bool successful_;
    mutable ::google::protobuf::internal::CachedSize _cached_size_;
    PROTOBUF_TSAN_DECLARE_MEMBER
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_Stc_2eproto;
};

// ===================================================================




// ===================================================================


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// -------------------------------------------------------------------

// LoginResponse

// bool successful = 1;
inline void LoginResponse::clear_successful() {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.successful_ = false;
}
inline bool LoginResponse::successful() const {
  // @@protoc_insertion_point(field_get:RatkiniaProtocol.LoginResponse.successful)
  return _internal_successful();
}
inline void LoginResponse::set_successful(bool value) {
  _internal_set_successful(value);
  // @@protoc_insertion_point(field_set:RatkiniaProtocol.LoginResponse.successful)
}
inline bool LoginResponse::_internal_successful() const {
  ::google::protobuf::internal::TSanRead(&_impl_);
  return _impl_.successful_;
}
inline void LoginResponse::_internal_set_successful(bool value) {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.successful_ = value;
}

// string failure_reason = 2;
inline void LoginResponse::clear_failure_reason() {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.failure_reason_.ClearToEmpty();
}
inline const std::string& LoginResponse::failure_reason() const
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  // @@protoc_insertion_point(field_get:RatkiniaProtocol.LoginResponse.failure_reason)
  return _internal_failure_reason();
}
template <typename Arg_, typename... Args_>
inline PROTOBUF_ALWAYS_INLINE void LoginResponse::set_failure_reason(Arg_&& arg,
                                                     Args_... args) {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.failure_reason_.Set(static_cast<Arg_&&>(arg), args..., GetArena());
  // @@protoc_insertion_point(field_set:RatkiniaProtocol.LoginResponse.failure_reason)
}
inline std::string* LoginResponse::mutable_failure_reason() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  std::string* _s = _internal_mutable_failure_reason();
  // @@protoc_insertion_point(field_mutable:RatkiniaProtocol.LoginResponse.failure_reason)
  return _s;
}
inline const std::string& LoginResponse::_internal_failure_reason() const {
  ::google::protobuf::internal::TSanRead(&_impl_);
  return _impl_.failure_reason_.Get();
}
inline void LoginResponse::_internal_set_failure_reason(const std::string& value) {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.failure_reason_.Set(value, GetArena());
}
inline std::string* LoginResponse::_internal_mutable_failure_reason() {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  return _impl_.failure_reason_.Mutable( GetArena());
}
inline std::string* LoginResponse::release_failure_reason() {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  // @@protoc_insertion_point(field_release:RatkiniaProtocol.LoginResponse.failure_reason)
  return _impl_.failure_reason_.Release();
}
inline void LoginResponse::set_allocated_failure_reason(std::string* value) {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.failure_reason_.SetAllocated(value, GetArena());
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
        if (_impl_.failure_reason_.IsDefault()) {
          _impl_.failure_reason_.Set("", GetArena());
        }
  #endif  // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:RatkiniaProtocol.LoginResponse.failure_reason)
}

// -------------------------------------------------------------------

// RegisterResponse

// .RatkiniaProtocol.RegisterResponse.FailedReason failed_reason = 1;
inline void RegisterResponse::clear_failed_reason() {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.failed_reason_ = 0;
}
inline ::RatkiniaProtocol::RegisterResponse_FailedReason RegisterResponse::failed_reason() const {
  // @@protoc_insertion_point(field_get:RatkiniaProtocol.RegisterResponse.failed_reason)
  return _internal_failed_reason();
}
inline void RegisterResponse::set_failed_reason(::RatkiniaProtocol::RegisterResponse_FailedReason value) {
  _internal_set_failed_reason(value);
  // @@protoc_insertion_point(field_set:RatkiniaProtocol.RegisterResponse.failed_reason)
}
inline ::RatkiniaProtocol::RegisterResponse_FailedReason RegisterResponse::_internal_failed_reason() const {
  ::google::protobuf::internal::TSanRead(&_impl_);
  return static_cast<::RatkiniaProtocol::RegisterResponse_FailedReason>(_impl_.failed_reason_);
}
inline void RegisterResponse::_internal_set_failed_reason(::RatkiniaProtocol::RegisterResponse_FailedReason value) {
  ::google::protobuf::internal::TSanWrite(&_impl_);
  _impl_.failed_reason_ = value;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)
}  // namespace RatkiniaProtocol


namespace google {
namespace protobuf {

template <>
struct is_proto_enum<::RatkiniaProtocol::RegisterResponse_FailedReason> : std::true_type {};

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCLUDED_Stc_2eproto_2epb_2eh
