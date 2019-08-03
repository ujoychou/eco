// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Object.proto

#ifndef PROTOBUF_Object_2eproto__INCLUDED
#define PROTOBUF_Object_2eproto__INCLUDED

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
// @@protoc_insertion_point(includes)

namespace protobuf_Object_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[6];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
void InitDefaultsErrorImpl();
void InitDefaultsError();
void InitDefaultsRequestImpl();
void InitDefaultsRequest();
void InitDefaultsResponseImpl();
void InitDefaultsResponse();
void InitDefaultsSubscribeImpl();
void InitDefaultsSubscribe();
void InitDefaultsRemoveImpl();
void InitDefaultsRemove();
void InitDefaultsPropertyImpl();
void InitDefaultsProperty();
inline void InitDefaults() {
  InitDefaultsError();
  InitDefaultsRequest();
  InitDefaultsResponse();
  InitDefaultsSubscribe();
  InitDefaultsRemove();
  InitDefaultsProperty();
}
}  // namespace protobuf_Object_2eproto
namespace proto {
class Error;
class ErrorDefaultTypeInternal;
extern ErrorDefaultTypeInternal _Error_default_instance_;
class Property;
class PropertyDefaultTypeInternal;
extern PropertyDefaultTypeInternal _Property_default_instance_;
class Remove;
class RemoveDefaultTypeInternal;
extern RemoveDefaultTypeInternal _Remove_default_instance_;
class Request;
class RequestDefaultTypeInternal;
extern RequestDefaultTypeInternal _Request_default_instance_;
class Response;
class ResponseDefaultTypeInternal;
extern ResponseDefaultTypeInternal _Response_default_instance_;
class Subscribe;
class SubscribeDefaultTypeInternal;
extern SubscribeDefaultTypeInternal _Subscribe_default_instance_;
}  // namespace proto
namespace proto {

// ===================================================================

class Error : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.Error) */ {
 public:
  Error();
  virtual ~Error();

  Error(const Error& from);

  inline Error& operator=(const Error& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Error(Error&& from) noexcept
    : Error() {
    *this = ::std::move(from);
  }

  inline Error& operator=(Error&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Error& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Error* internal_default_instance() {
    return reinterpret_cast<const Error*>(
               &_Error_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(Error* other);
  friend void swap(Error& a, Error& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Error* New() const PROTOBUF_FINAL { return New(NULL); }

  Error* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Error& from);
  void MergeFrom(const Error& from);
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
  void InternalSwap(Error* other);
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

  // string message = 2;
  void clear_message();
  static const int kMessageFieldNumber = 2;
  const ::std::string& message() const;
  void set_message(const ::std::string& value);
  #if LANG_CXX11
  void set_message(::std::string&& value);
  #endif
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  ::std::string* mutable_message();
  ::std::string* release_message();
  void set_allocated_message(::std::string* message);

  // int32 id = 1;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::int32 id() const;
  void set_id(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:proto.Error)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr message_;
  ::google::protobuf::int32 id_;
  mutable int _cached_size_;
  friend struct ::protobuf_Object_2eproto::TableStruct;
  friend void ::protobuf_Object_2eproto::InitDefaultsErrorImpl();
};
// -------------------------------------------------------------------

class Request : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.Request) */ {
 public:
  Request();
  virtual ~Request();

  Request(const Request& from);

  inline Request& operator=(const Request& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Request(Request&& from) noexcept
    : Request() {
    *this = ::std::move(from);
  }

  inline Request& operator=(Request&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Request& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Request* internal_default_instance() {
    return reinterpret_cast<const Request*>(
               &_Request_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(Request* other);
  friend void swap(Request& a, Request& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Request* New() const PROTOBUF_FINAL { return New(NULL); }

  Request* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Request& from);
  void MergeFrom(const Request& from);
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
  void InternalSwap(Request* other);
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

  // uint64 user_id = 10;
  void clear_user_id();
  static const int kUserIdFieldNumber = 10;
  ::google::protobuf::uint64 user_id() const;
  void set_user_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:proto.Request)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint64 user_id_;
  mutable int _cached_size_;
  friend struct ::protobuf_Object_2eproto::TableStruct;
  friend void ::protobuf_Object_2eproto::InitDefaultsRequestImpl();
};
// -------------------------------------------------------------------

class Response : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.Response) */ {
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
  static const ::google::protobuf::Descriptor* descriptor();
  static const Response& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Response* internal_default_instance() {
    return reinterpret_cast<const Response*>(
               &_Response_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    2;

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

  // .proto.Error error = 1;
  bool has_error() const;
  void clear_error();
  static const int kErrorFieldNumber = 1;
  const ::proto::Error& error() const;
  ::proto::Error* release_error();
  ::proto::Error* mutable_error();
  void set_allocated_error(::proto::Error* error);

  // uint64 user_id = 10;
  void clear_user_id();
  static const int kUserIdFieldNumber = 10;
  ::google::protobuf::uint64 user_id() const;
  void set_user_id(::google::protobuf::uint64 value);

  // uint64 object_id = 11;
  void clear_object_id();
  static const int kObjectIdFieldNumber = 11;
  ::google::protobuf::uint64 object_id() const;
  void set_object_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:proto.Response)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::proto::Error* error_;
  ::google::protobuf::uint64 user_id_;
  ::google::protobuf::uint64 object_id_;
  mutable int _cached_size_;
  friend struct ::protobuf_Object_2eproto::TableStruct;
  friend void ::protobuf_Object_2eproto::InitDefaultsResponseImpl();
};
// -------------------------------------------------------------------

class Subscribe : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.Subscribe) */ {
 public:
  Subscribe();
  virtual ~Subscribe();

  Subscribe(const Subscribe& from);

  inline Subscribe& operator=(const Subscribe& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Subscribe(Subscribe&& from) noexcept
    : Subscribe() {
    *this = ::std::move(from);
  }

  inline Subscribe& operator=(Subscribe&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Subscribe& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Subscribe* internal_default_instance() {
    return reinterpret_cast<const Subscribe*>(
               &_Subscribe_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    3;

  void Swap(Subscribe* other);
  friend void swap(Subscribe& a, Subscribe& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Subscribe* New() const PROTOBUF_FINAL { return New(NULL); }

  Subscribe* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Subscribe& from);
  void MergeFrom(const Subscribe& from);
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
  void InternalSwap(Subscribe* other);
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

  // uint32 prop = 11;
  void clear_prop();
  static const int kPropFieldNumber = 11;
  ::google::protobuf::uint32 prop() const;
  void set_prop(::google::protobuf::uint32 value);

  // uint64 user_id = 10;
  void clear_user_id();
  static const int kUserIdFieldNumber = 10;
  ::google::protobuf::uint64 user_id() const;
  void set_user_id(::google::protobuf::uint64 value);

  // uint64 value = 12;
  void clear_value();
  static const int kValueFieldNumber = 12;
  ::google::protobuf::uint64 value() const;
  void set_value(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:proto.Subscribe)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 prop_;
  ::google::protobuf::uint64 user_id_;
  ::google::protobuf::uint64 value_;
  mutable int _cached_size_;
  friend struct ::protobuf_Object_2eproto::TableStruct;
  friend void ::protobuf_Object_2eproto::InitDefaultsSubscribeImpl();
};
// -------------------------------------------------------------------

class Remove : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.Remove) */ {
 public:
  Remove();
  virtual ~Remove();

  Remove(const Remove& from);

  inline Remove& operator=(const Remove& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Remove(Remove&& from) noexcept
    : Remove() {
    *this = ::std::move(from);
  }

  inline Remove& operator=(Remove&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Remove& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Remove* internal_default_instance() {
    return reinterpret_cast<const Remove*>(
               &_Remove_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    4;

  void Swap(Remove* other);
  friend void swap(Remove& a, Remove& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Remove* New() const PROTOBUF_FINAL { return New(NULL); }

  Remove* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Remove& from);
  void MergeFrom(const Remove& from);
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
  void InternalSwap(Remove* other);
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

  // uint64 user_id = 10;
  void clear_user_id();
  static const int kUserIdFieldNumber = 10;
  ::google::protobuf::uint64 user_id() const;
  void set_user_id(::google::protobuf::uint64 value);

  // uint64 object_id = 11;
  void clear_object_id();
  static const int kObjectIdFieldNumber = 11;
  ::google::protobuf::uint64 object_id() const;
  void set_object_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:proto.Remove)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint64 user_id_;
  ::google::protobuf::uint64 object_id_;
  mutable int _cached_size_;
  friend struct ::protobuf_Object_2eproto::TableStruct;
  friend void ::protobuf_Object_2eproto::InitDefaultsRemoveImpl();
};
// -------------------------------------------------------------------

class Property : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.Property) */ {
 public:
  Property();
  virtual ~Property();

  Property(const Property& from);

  inline Property& operator=(const Property& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Property(Property&& from) noexcept
    : Property() {
    *this = ::std::move(from);
  }

  inline Property& operator=(Property&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Property& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Property* internal_default_instance() {
    return reinterpret_cast<const Property*>(
               &_Property_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    5;

  void Swap(Property* other);
  friend void swap(Property& a, Property& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Property* New() const PROTOBUF_FINAL { return New(NULL); }

  Property* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Property& from);
  void MergeFrom(const Property& from);
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
  void InternalSwap(Property* other);
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

  // string name = 12;
  void clear_name();
  static const int kNameFieldNumber = 12;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  #if LANG_CXX11
  void set_name(::std::string&& value);
  #endif
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // string value = 13;
  void clear_value();
  static const int kValueFieldNumber = 13;
  const ::std::string& value() const;
  void set_value(const ::std::string& value);
  #if LANG_CXX11
  void set_value(::std::string&& value);
  #endif
  void set_value(const char* value);
  void set_value(const char* value, size_t size);
  ::std::string* mutable_value();
  ::std::string* release_value();
  void set_allocated_value(::std::string* value);

  // uint64 user_id = 10;
  void clear_user_id();
  static const int kUserIdFieldNumber = 10;
  ::google::protobuf::uint64 user_id() const;
  void set_user_id(::google::protobuf::uint64 value);

  // uint64 object_id = 11;
  void clear_object_id();
  static const int kObjectIdFieldNumber = 11;
  ::google::protobuf::uint64 object_id() const;
  void set_object_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:proto.Property)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  ::google::protobuf::internal::ArenaStringPtr value_;
  ::google::protobuf::uint64 user_id_;
  ::google::protobuf::uint64 object_id_;
  mutable int _cached_size_;
  friend struct ::protobuf_Object_2eproto::TableStruct;
  friend void ::protobuf_Object_2eproto::InitDefaultsPropertyImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Error

// int32 id = 1;
inline void Error::clear_id() {
  id_ = 0;
}
inline ::google::protobuf::int32 Error::id() const {
  // @@protoc_insertion_point(field_get:proto.Error.id)
  return id_;
}
inline void Error::set_id(::google::protobuf::int32 value) {
  
  id_ = value;
  // @@protoc_insertion_point(field_set:proto.Error.id)
}

// string message = 2;
inline void Error::clear_message() {
  message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Error::message() const {
  // @@protoc_insertion_point(field_get:proto.Error.message)
  return message_.GetNoArena();
}
inline void Error::set_message(const ::std::string& value) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.Error.message)
}
#if LANG_CXX11
inline void Error::set_message(::std::string&& value) {
  
  message_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.Error.message)
}
#endif
inline void Error::set_message(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.Error.message)
}
inline void Error::set_message(const char* value, size_t size) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.Error.message)
}
inline ::std::string* Error::mutable_message() {
  
  // @@protoc_insertion_point(field_mutable:proto.Error.message)
  return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Error::release_message() {
  // @@protoc_insertion_point(field_release:proto.Error.message)
  
  return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Error::set_allocated_message(::std::string* message) {
  if (message != NULL) {
    
  } else {
    
  }
  message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
  // @@protoc_insertion_point(field_set_allocated:proto.Error.message)
}

// -------------------------------------------------------------------

// Request

// uint64 user_id = 10;
inline void Request::clear_user_id() {
  user_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Request::user_id() const {
  // @@protoc_insertion_point(field_get:proto.Request.user_id)
  return user_id_;
}
inline void Request::set_user_id(::google::protobuf::uint64 value) {
  
  user_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Request.user_id)
}

// -------------------------------------------------------------------

// Response

// .proto.Error error = 1;
inline bool Response::has_error() const {
  return this != internal_default_instance() && error_ != NULL;
}
inline void Response::clear_error() {
  if (GetArenaNoVirtual() == NULL && error_ != NULL) {
    delete error_;
  }
  error_ = NULL;
}
inline const ::proto::Error& Response::error() const {
  const ::proto::Error* p = error_;
  // @@protoc_insertion_point(field_get:proto.Response.error)
  return p != NULL ? *p : *reinterpret_cast<const ::proto::Error*>(
      &::proto::_Error_default_instance_);
}
inline ::proto::Error* Response::release_error() {
  // @@protoc_insertion_point(field_release:proto.Response.error)
  
  ::proto::Error* temp = error_;
  error_ = NULL;
  return temp;
}
inline ::proto::Error* Response::mutable_error() {
  
  if (error_ == NULL) {
    error_ = new ::proto::Error;
  }
  // @@protoc_insertion_point(field_mutable:proto.Response.error)
  return error_;
}
inline void Response::set_allocated_error(::proto::Error* error) {
  ::google::protobuf::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == NULL) {
    delete error_;
  }
  if (error) {
    ::google::protobuf::Arena* submessage_arena = NULL;
    if (message_arena != submessage_arena) {
      error = ::google::protobuf::internal::GetOwnedMessage(
          message_arena, error, submessage_arena);
    }
    
  } else {
    
  }
  error_ = error;
  // @@protoc_insertion_point(field_set_allocated:proto.Response.error)
}

// uint64 user_id = 10;
inline void Response::clear_user_id() {
  user_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Response::user_id() const {
  // @@protoc_insertion_point(field_get:proto.Response.user_id)
  return user_id_;
}
inline void Response::set_user_id(::google::protobuf::uint64 value) {
  
  user_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Response.user_id)
}

// uint64 object_id = 11;
inline void Response::clear_object_id() {
  object_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Response::object_id() const {
  // @@protoc_insertion_point(field_get:proto.Response.object_id)
  return object_id_;
}
inline void Response::set_object_id(::google::protobuf::uint64 value) {
  
  object_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Response.object_id)
}

// -------------------------------------------------------------------

// Subscribe

// uint64 user_id = 10;
inline void Subscribe::clear_user_id() {
  user_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Subscribe::user_id() const {
  // @@protoc_insertion_point(field_get:proto.Subscribe.user_id)
  return user_id_;
}
inline void Subscribe::set_user_id(::google::protobuf::uint64 value) {
  
  user_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Subscribe.user_id)
}

// uint32 prop = 11;
inline void Subscribe::clear_prop() {
  prop_ = 0u;
}
inline ::google::protobuf::uint32 Subscribe::prop() const {
  // @@protoc_insertion_point(field_get:proto.Subscribe.prop)
  return prop_;
}
inline void Subscribe::set_prop(::google::protobuf::uint32 value) {
  
  prop_ = value;
  // @@protoc_insertion_point(field_set:proto.Subscribe.prop)
}

// uint64 value = 12;
inline void Subscribe::clear_value() {
  value_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Subscribe::value() const {
  // @@protoc_insertion_point(field_get:proto.Subscribe.value)
  return value_;
}
inline void Subscribe::set_value(::google::protobuf::uint64 value) {
  
  value_ = value;
  // @@protoc_insertion_point(field_set:proto.Subscribe.value)
}

// -------------------------------------------------------------------

// Remove

// uint64 user_id = 10;
inline void Remove::clear_user_id() {
  user_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Remove::user_id() const {
  // @@protoc_insertion_point(field_get:proto.Remove.user_id)
  return user_id_;
}
inline void Remove::set_user_id(::google::protobuf::uint64 value) {
  
  user_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Remove.user_id)
}

// uint64 object_id = 11;
inline void Remove::clear_object_id() {
  object_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Remove::object_id() const {
  // @@protoc_insertion_point(field_get:proto.Remove.object_id)
  return object_id_;
}
inline void Remove::set_object_id(::google::protobuf::uint64 value) {
  
  object_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Remove.object_id)
}

// -------------------------------------------------------------------

// Property

// uint64 user_id = 10;
inline void Property::clear_user_id() {
  user_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Property::user_id() const {
  // @@protoc_insertion_point(field_get:proto.Property.user_id)
  return user_id_;
}
inline void Property::set_user_id(::google::protobuf::uint64 value) {
  
  user_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Property.user_id)
}

// uint64 object_id = 11;
inline void Property::clear_object_id() {
  object_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Property::object_id() const {
  // @@protoc_insertion_point(field_get:proto.Property.object_id)
  return object_id_;
}
inline void Property::set_object_id(::google::protobuf::uint64 value) {
  
  object_id_ = value;
  // @@protoc_insertion_point(field_set:proto.Property.object_id)
}

// string name = 12;
inline void Property::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Property::name() const {
  // @@protoc_insertion_point(field_get:proto.Property.name)
  return name_.GetNoArena();
}
inline void Property::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.Property.name)
}
#if LANG_CXX11
inline void Property::set_name(::std::string&& value) {
  
  name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.Property.name)
}
#endif
inline void Property::set_name(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.Property.name)
}
inline void Property::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.Property.name)
}
inline ::std::string* Property::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:proto.Property.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Property::release_name() {
  // @@protoc_insertion_point(field_release:proto.Property.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Property::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:proto.Property.name)
}

// string value = 13;
inline void Property::clear_value() {
  value_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Property::value() const {
  // @@protoc_insertion_point(field_get:proto.Property.value)
  return value_.GetNoArena();
}
inline void Property::set_value(const ::std::string& value) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.Property.value)
}
#if LANG_CXX11
inline void Property::set_value(::std::string&& value) {
  
  value_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.Property.value)
}
#endif
inline void Property::set_value(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.Property.value)
}
inline void Property::set_value(const char* value, size_t size) {
  
  value_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.Property.value)
}
inline ::std::string* Property::mutable_value() {
  
  // @@protoc_insertion_point(field_mutable:proto.Property.value)
  return value_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Property::release_value() {
  // @@protoc_insertion_point(field_release:proto.Property.value)
  
  return value_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Property::set_allocated_value(::std::string* value) {
  if (value != NULL) {
    
  } else {
    
  }
  value_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set_allocated:proto.Property.value)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace proto

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_Object_2eproto__INCLUDED
