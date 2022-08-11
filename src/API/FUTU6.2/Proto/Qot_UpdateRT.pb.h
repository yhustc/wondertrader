// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Qot_UpdateRT.proto

#ifndef PROTOBUF_Qot_5fUpdateRT_2eproto__INCLUDED
#define PROTOBUF_Qot_5fUpdateRT_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3005001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include "Common.pb.h"
#include "Qot_Common.pb.h"
// @@protoc_insertion_point(includes)

namespace protobuf_Qot_5fUpdateRT_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[2];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
void InitDefaultsS2CImpl();
void InitDefaultsS2C();
void InitDefaultsResponseImpl();
void InitDefaultsResponse();
inline void InitDefaults() {
  InitDefaultsS2C();
  InitDefaultsResponse();
}
}  // namespace protobuf_Qot_5fUpdateRT_2eproto
namespace Qot_UpdateRT {
class Response;
class ResponseDefaultTypeInternal;
extern ResponseDefaultTypeInternal _Response_default_instance_;
class S2C;
class S2CDefaultTypeInternal;
extern S2CDefaultTypeInternal _S2C_default_instance_;
}  // namespace Qot_UpdateRT
namespace Qot_UpdateRT {

// ===================================================================

class S2C : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Qot_UpdateRT.S2C) */ {
 public:
  S2C();
  virtual ~S2C();

  S2C(const S2C& from);

  inline S2C& operator=(const S2C& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  S2C(S2C&& from) noexcept
    : S2C() {
    *this = ::std::move(from);
  }

  inline S2C& operator=(S2C&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const S2C& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const S2C* internal_default_instance() {
    return reinterpret_cast<const S2C*>(
               &_S2C_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(S2C* other);
  friend void swap(S2C& a, S2C& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline S2C* New() const PROTOBUF_FINAL { return New(NULL); }

  S2C* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const S2C& from);
  void MergeFrom(const S2C& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(S2C* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .Qot_Common.TimeShare rtList = 2;
  int rtlist_size() const;
  void clear_rtlist();
  static const int kRtListFieldNumber = 2;
  const ::Qot_Common::TimeShare& rtlist(int index) const;
  ::Qot_Common::TimeShare* mutable_rtlist(int index);
  ::Qot_Common::TimeShare* add_rtlist();
  ::google::protobuf::RepeatedPtrField< ::Qot_Common::TimeShare >*
      mutable_rtlist();
  const ::google::protobuf::RepeatedPtrField< ::Qot_Common::TimeShare >&
      rtlist() const;

  // required .Qot_Common.Security security = 1;
  bool has_security() const;
  void clear_security();
  static const int kSecurityFieldNumber = 1;
  const ::Qot_Common::Security& security() const;
  ::Qot_Common::Security* release_security();
  ::Qot_Common::Security* mutable_security();
  void set_allocated_security(::Qot_Common::Security* security);

  // @@protoc_insertion_point(class_scope:Qot_UpdateRT.S2C)
 private:
  void set_has_security();
  void clear_has_security();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::RepeatedPtrField< ::Qot_Common::TimeShare > rtlist_;
  ::Qot_Common::Security* security_;
  friend struct ::protobuf_Qot_5fUpdateRT_2eproto::TableStruct;
  friend void ::protobuf_Qot_5fUpdateRT_2eproto::InitDefaultsS2CImpl();
};
// -------------------------------------------------------------------

class Response : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Qot_UpdateRT.Response) */ {
 public:
  Response();
  virtual ~Response();

  Response(const Response& from);

  inline Response& operator=(const Response& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Response(Response&& from) noexcept
    : Response() {
    *this = ::std::move(from);
  }

  inline Response& operator=(Response&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Response& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Response* internal_default_instance() {
    return reinterpret_cast<const Response*>(
               &_Response_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(Response* other);
  friend void swap(Response& a, Response& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Response* New() const PROTOBUF_FINAL { return New(NULL); }

  Response* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Response& from);
  void MergeFrom(const Response& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(Response* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string retMsg = 2;
  bool has_retmsg() const;
  void clear_retmsg();
  static const int kRetMsgFieldNumber = 2;
  const ::std::string& retmsg() const;
  void set_retmsg(const ::std::string& value);
  #if LANG_CXX11
  void set_retmsg(::std::string&& value);
  #endif
  void set_retmsg(const char* value);
  void set_retmsg(const char* value, size_t size);
  ::std::string* mutable_retmsg();
  ::std::string* release_retmsg();
  void set_allocated_retmsg(::std::string* retmsg);

  // optional .Qot_UpdateRT.S2C s2c = 4;
  bool has_s2c() const;
  void clear_s2c();
  static const int kS2CFieldNumber = 4;
  const ::Qot_UpdateRT::S2C& s2c() const;
  ::Qot_UpdateRT::S2C* release_s2c();
  ::Qot_UpdateRT::S2C* mutable_s2c();
  void set_allocated_s2c(::Qot_UpdateRT::S2C* s2c);

  // optional int32 errCode = 3;
  bool has_errcode() const;
  void clear_errcode();
  static const int kErrCodeFieldNumber = 3;
  ::google::protobuf::int32 errcode() const;
  void set_errcode(::google::protobuf::int32 value);

  // required int32 retType = 1 [default = -400];
  bool has_rettype() const;
  void clear_rettype();
  static const int kRetTypeFieldNumber = 1;
  ::google::protobuf::int32 rettype() const;
  void set_rettype(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:Qot_UpdateRT.Response)
 private:
  void set_has_rettype();
  void clear_has_rettype();
  void set_has_retmsg();
  void clear_has_retmsg();
  void set_has_errcode();
  void clear_has_errcode();
  void set_has_s2c();
  void clear_has_s2c();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr retmsg_;
  ::Qot_UpdateRT::S2C* s2c_;
  ::google::protobuf::int32 errcode_;
  ::google::protobuf::int32 rettype_;
  friend struct ::protobuf_Qot_5fUpdateRT_2eproto::TableStruct;
  friend void ::protobuf_Qot_5fUpdateRT_2eproto::InitDefaultsResponseImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// S2C

// required .Qot_Common.Security security = 1;
inline bool S2C::has_security() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void S2C::set_has_security() {
  _has_bits_[0] |= 0x00000001u;
}
inline void S2C::clear_has_security() {
  _has_bits_[0] &= ~0x00000001u;
}
inline const ::Qot_Common::Security& S2C::security() const {
  const ::Qot_Common::Security* p = security_;
  // @@protoc_insertion_point(field_get:Qot_UpdateRT.S2C.security)
  return p != NULL ? *p : *reinterpret_cast<const ::Qot_Common::Security*>(
      &::Qot_Common::_Security_default_instance_);
}
inline ::Qot_Common::Security* S2C::release_security() {
  // @@protoc_insertion_point(field_release:Qot_UpdateRT.S2C.security)
  clear_has_security();
  ::Qot_Common::Security* temp = security_;
  security_ = NULL;
  return temp;
}
inline ::Qot_Common::Security* S2C::mutable_security() {
  set_has_security();
  if (security_ == NULL) {
    security_ = new ::Qot_Common::Security;
  }
  // @@protoc_insertion_point(field_mutable:Qot_UpdateRT.S2C.security)
  return security_;
}
inline void S2C::set_allocated_security(::Qot_Common::Security* security) {
  ::google::protobuf::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == NULL) {
    delete reinterpret_cast< ::google::protobuf::MessageLite*>(security_);
  }
  if (security) {
    ::google::protobuf::Arena* submessage_arena = NULL;
    if (message_arena != submessage_arena) {
      security = ::google::protobuf::internal::GetOwnedMessage(
          message_arena, security, submessage_arena);
    }
    set_has_security();
  } else {
    clear_has_security();
  }
  security_ = security;
  // @@protoc_insertion_point(field_set_allocated:Qot_UpdateRT.S2C.security)
}

// repeated .Qot_Common.TimeShare rtList = 2;
inline int S2C::rtlist_size() const {
  return rtlist_.size();
}
inline const ::Qot_Common::TimeShare& S2C::rtlist(int index) const {
  // @@protoc_insertion_point(field_get:Qot_UpdateRT.S2C.rtList)
  return rtlist_.Get(index);
}
inline ::Qot_Common::TimeShare* S2C::mutable_rtlist(int index) {
  // @@protoc_insertion_point(field_mutable:Qot_UpdateRT.S2C.rtList)
  return rtlist_.Mutable(index);
}
inline ::Qot_Common::TimeShare* S2C::add_rtlist() {
  // @@protoc_insertion_point(field_add:Qot_UpdateRT.S2C.rtList)
  return rtlist_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::Qot_Common::TimeShare >*
S2C::mutable_rtlist() {
  // @@protoc_insertion_point(field_mutable_list:Qot_UpdateRT.S2C.rtList)
  return &rtlist_;
}
inline const ::google::protobuf::RepeatedPtrField< ::Qot_Common::TimeShare >&
S2C::rtlist() const {
  // @@protoc_insertion_point(field_list:Qot_UpdateRT.S2C.rtList)
  return rtlist_;
}

// -------------------------------------------------------------------

// Response

// required int32 retType = 1 [default = -400];
inline bool Response::has_rettype() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void Response::set_has_rettype() {
  _has_bits_[0] |= 0x00000008u;
}
inline void Response::clear_has_rettype() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void Response::clear_rettype() {
  rettype_ = -400;
  clear_has_rettype();
}
inline ::google::protobuf::int32 Response::rettype() const {
  // @@protoc_insertion_point(field_get:Qot_UpdateRT.Response.retType)
  return rettype_;
}
inline void Response::set_rettype(::google::protobuf::int32 value) {
  set_has_rettype();
  rettype_ = value;
  // @@protoc_insertion_point(field_set:Qot_UpdateRT.Response.retType)
}

// optional string retMsg = 2;
inline bool Response::has_retmsg() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Response::set_has_retmsg() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Response::clear_has_retmsg() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Response::clear_retmsg() {
  retmsg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_retmsg();
}
inline const ::std::string& Response::retmsg() const {
  // @@protoc_insertion_point(field_get:Qot_UpdateRT.Response.retMsg)
  return retmsg_.GetNoArena();
}
inline void Response::set_retmsg(const ::std::string& value) {
  set_has_retmsg();
  retmsg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Qot_UpdateRT.Response.retMsg)
}
#if LANG_CXX11
inline void Response::set_retmsg(::std::string&& value) {
  set_has_retmsg();
  retmsg_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Qot_UpdateRT.Response.retMsg)
}
#endif
inline void Response::set_retmsg(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_retmsg();
  retmsg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Qot_UpdateRT.Response.retMsg)
}
inline void Response::set_retmsg(const char* value, size_t size) {
  set_has_retmsg();
  retmsg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Qot_UpdateRT.Response.retMsg)
}
inline ::std::string* Response::mutable_retmsg() {
  set_has_retmsg();
  // @@protoc_insertion_point(field_mutable:Qot_UpdateRT.Response.retMsg)
  return retmsg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Response::release_retmsg() {
  // @@protoc_insertion_point(field_release:Qot_UpdateRT.Response.retMsg)
  clear_has_retmsg();
  return retmsg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Response::set_allocated_retmsg(::std::string* retmsg) {
  if (retmsg != NULL) {
    set_has_retmsg();
  } else {
    clear_has_retmsg();
  }
  retmsg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), retmsg);
  // @@protoc_insertion_point(field_set_allocated:Qot_UpdateRT.Response.retMsg)
}

// optional int32 errCode = 3;
inline bool Response::has_errcode() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void Response::set_has_errcode() {
  _has_bits_[0] |= 0x00000004u;
}
inline void Response::clear_has_errcode() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void Response::clear_errcode() {
  errcode_ = 0;
  clear_has_errcode();
}
inline ::google::protobuf::int32 Response::errcode() const {
  // @@protoc_insertion_point(field_get:Qot_UpdateRT.Response.errCode)
  return errcode_;
}
inline void Response::set_errcode(::google::protobuf::int32 value) {
  set_has_errcode();
  errcode_ = value;
  // @@protoc_insertion_point(field_set:Qot_UpdateRT.Response.errCode)
}

// optional .Qot_UpdateRT.S2C s2c = 4;
inline bool Response::has_s2c() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Response::set_has_s2c() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Response::clear_has_s2c() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Response::clear_s2c() {
  if (s2c_ != NULL) s2c_->Clear();
  clear_has_s2c();
}
inline const ::Qot_UpdateRT::S2C& Response::s2c() const {
  const ::Qot_UpdateRT::S2C* p = s2c_;
  // @@protoc_insertion_point(field_get:Qot_UpdateRT.Response.s2c)
  return p != NULL ? *p : *reinterpret_cast<const ::Qot_UpdateRT::S2C*>(
      &::Qot_UpdateRT::_S2C_default_instance_);
}
inline ::Qot_UpdateRT::S2C* Response::release_s2c() {
  // @@protoc_insertion_point(field_release:Qot_UpdateRT.Response.s2c)
  clear_has_s2c();
  ::Qot_UpdateRT::S2C* temp = s2c_;
  s2c_ = NULL;
  return temp;
}
inline ::Qot_UpdateRT::S2C* Response::mutable_s2c() {
  set_has_s2c();
  if (s2c_ == NULL) {
    s2c_ = new ::Qot_UpdateRT::S2C;
  }
  // @@protoc_insertion_point(field_mutable:Qot_UpdateRT.Response.s2c)
  return s2c_;
}
inline void Response::set_allocated_s2c(::Qot_UpdateRT::S2C* s2c) {
  ::google::protobuf::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == NULL) {
    delete s2c_;
  }
  if (s2c) {
    ::google::protobuf::Arena* submessage_arena = NULL;
    if (message_arena != submessage_arena) {
      s2c = ::google::protobuf::internal::GetOwnedMessage(
          message_arena, s2c, submessage_arena);
    }
    set_has_s2c();
  } else {
    clear_has_s2c();
  }
  s2c_ = s2c;
  // @@protoc_insertion_point(field_set_allocated:Qot_UpdateRT.Response.s2c)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace Qot_UpdateRT

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_Qot_5fUpdateRT_2eproto__INCLUDED
