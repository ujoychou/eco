// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Test.proto

#ifndef PROTOBUF_Test_2eproto__INCLUDED
#define PROTOBUF_Test_2eproto__INCLUDED

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
#include "Object.pb.h"
// @@protoc_insertion_point(includes)

namespace protobuf_Test_2eproto {
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
void InitDefaultsTestReqImpl();
void InitDefaultsTestReq();
void InitDefaultsTestRspImpl();
void InitDefaultsTestRsp();
inline void InitDefaults() {
  InitDefaultsTestReq();
  InitDefaultsTestRsp();
}
}  // namespace protobuf_Test_2eproto
namespace proto {
class TestReq;
class TestReqDefaultTypeInternal;
extern TestReqDefaultTypeInternal _TestReq_default_instance_;
class TestRsp;
class TestRspDefaultTypeInternal;
extern TestRspDefaultTypeInternal _TestRsp_default_instance_;
}  // namespace proto
namespace proto {

// ===================================================================

class TestReq : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.TestReq) */ {
 public:
  TestReq();
  virtual ~TestReq();

  TestReq(const TestReq& from);

  inline TestReq& operator=(const TestReq& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  TestReq(TestReq&& from) noexcept
    : TestReq() {
    *this = ::std::move(from);
  }

  inline TestReq& operator=(TestReq&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const TestReq& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const TestReq* internal_default_instance() {
    return reinterpret_cast<const TestReq*>(
               &_TestReq_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(TestReq* other);
  friend void swap(TestReq& a, TestReq& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline TestReq* New() const PROTOBUF_FINAL { return New(NULL); }

  TestReq* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const TestReq& from);
  void MergeFrom(const TestReq& from);
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
  void InternalSwap(TestReq* other);
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

  // string str1 = 14;
  void clear_str1();
  static const int kStr1FieldNumber = 14;
  const ::std::string& str1() const;
  void set_str1(const ::std::string& value);
  #if LANG_CXX11
  void set_str1(::std::string&& value);
  #endif
  void set_str1(const char* value);
  void set_str1(const char* value, size_t size);
  ::std::string* mutable_str1();
  ::std::string* release_str1();
  void set_allocated_str1(::std::string* str1);

  // string str2 = 15;
  void clear_str2();
  static const int kStr2FieldNumber = 15;
  const ::std::string& str2() const;
  void set_str2(const ::std::string& value);
  #if LANG_CXX11
  void set_str2(::std::string&& value);
  #endif
  void set_str2(const char* value);
  void set_str2(const char* value, size_t size);
  ::std::string* mutable_str2();
  ::std::string* release_str2();
  void set_allocated_str2(::std::string* str2);

  // string str3 = 16;
  void clear_str3();
  static const int kStr3FieldNumber = 16;
  const ::std::string& str3() const;
  void set_str3(const ::std::string& value);
  #if LANG_CXX11
  void set_str3(::std::string&& value);
  #endif
  void set_str3(const char* value);
  void set_str3(const char* value, size_t size);
  ::std::string* mutable_str3();
  ::std::string* release_str3();
  void set_allocated_str3(::std::string* str3);

  // string str4 = 17;
  void clear_str4();
  static const int kStr4FieldNumber = 17;
  const ::std::string& str4() const;
  void set_str4(const ::std::string& value);
  #if LANG_CXX11
  void set_str4(::std::string&& value);
  #endif
  void set_str4(const char* value);
  void set_str4(const char* value, size_t size);
  ::std::string* mutable_str4();
  ::std::string* release_str4();
  void set_allocated_str4(::std::string* str4);

  // uint64 int1 = 10;
  void clear_int1();
  static const int kInt1FieldNumber = 10;
  ::google::protobuf::uint64 int1() const;
  void set_int1(::google::protobuf::uint64 value);

  // uint64 int2 = 11;
  void clear_int2();
  static const int kInt2FieldNumber = 11;
  ::google::protobuf::uint64 int2() const;
  void set_int2(::google::protobuf::uint64 value);

  // uint64 int3 = 12;
  void clear_int3();
  static const int kInt3FieldNumber = 12;
  ::google::protobuf::uint64 int3() const;
  void set_int3(::google::protobuf::uint64 value);

  // uint64 int4 = 13;
  void clear_int4();
  static const int kInt4FieldNumber = 13;
  ::google::protobuf::uint64 int4() const;
  void set_int4(::google::protobuf::uint64 value);

  // double dbl1 = 18;
  void clear_dbl1();
  static const int kDbl1FieldNumber = 18;
  double dbl1() const;
  void set_dbl1(double value);

  // double dbl2 = 19;
  void clear_dbl2();
  static const int kDbl2FieldNumber = 19;
  double dbl2() const;
  void set_dbl2(double value);

  // double dbl3 = 20;
  void clear_dbl3();
  static const int kDbl3FieldNumber = 20;
  double dbl3() const;
  void set_dbl3(double value);

  // double dbl4 = 21;
  void clear_dbl4();
  static const int kDbl4FieldNumber = 21;
  double dbl4() const;
  void set_dbl4(double value);

  // @@protoc_insertion_point(class_scope:proto.TestReq)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr str1_;
  ::google::protobuf::internal::ArenaStringPtr str2_;
  ::google::protobuf::internal::ArenaStringPtr str3_;
  ::google::protobuf::internal::ArenaStringPtr str4_;
  ::google::protobuf::uint64 int1_;
  ::google::protobuf::uint64 int2_;
  ::google::protobuf::uint64 int3_;
  ::google::protobuf::uint64 int4_;
  double dbl1_;
  double dbl2_;
  double dbl3_;
  double dbl4_;
  mutable int _cached_size_;
  friend struct ::protobuf_Test_2eproto::TableStruct;
  friend void ::protobuf_Test_2eproto::InitDefaultsTestReqImpl();
};
// -------------------------------------------------------------------

class TestRsp : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.TestRsp) */ {
 public:
  TestRsp();
  virtual ~TestRsp();

  TestRsp(const TestRsp& from);

  inline TestRsp& operator=(const TestRsp& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  TestRsp(TestRsp&& from) noexcept
    : TestRsp() {
    *this = ::std::move(from);
  }

  inline TestRsp& operator=(TestRsp&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const TestRsp& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const TestRsp* internal_default_instance() {
    return reinterpret_cast<const TestRsp*>(
               &_TestRsp_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(TestRsp* other);
  friend void swap(TestRsp& a, TestRsp& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline TestRsp* New() const PROTOBUF_FINAL { return New(NULL); }

  TestRsp* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const TestRsp& from);
  void MergeFrom(const TestRsp& from);
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
  void InternalSwap(TestRsp* other);
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

  // .proto.TestReq data = 2;
  bool has_data() const;
  void clear_data();
  static const int kDataFieldNumber = 2;
  const ::proto::TestReq& data() const;
  ::proto::TestReq* release_data();
  ::proto::TestReq* mutable_data();
  void set_allocated_data(::proto::TestReq* data);

  // @@protoc_insertion_point(class_scope:proto.TestRsp)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::proto::Error* error_;
  ::proto::TestReq* data_;
  mutable int _cached_size_;
  friend struct ::protobuf_Test_2eproto::TableStruct;
  friend void ::protobuf_Test_2eproto::InitDefaultsTestRspImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// TestReq

// uint64 int1 = 10;
inline void TestReq::clear_int1() {
  int1_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 TestReq::int1() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.int1)
  return int1_;
}
inline void TestReq::set_int1(::google::protobuf::uint64 value) {
  
  int1_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.int1)
}

// uint64 int2 = 11;
inline void TestReq::clear_int2() {
  int2_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 TestReq::int2() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.int2)
  return int2_;
}
inline void TestReq::set_int2(::google::protobuf::uint64 value) {
  
  int2_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.int2)
}

// uint64 int3 = 12;
inline void TestReq::clear_int3() {
  int3_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 TestReq::int3() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.int3)
  return int3_;
}
inline void TestReq::set_int3(::google::protobuf::uint64 value) {
  
  int3_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.int3)
}

// uint64 int4 = 13;
inline void TestReq::clear_int4() {
  int4_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 TestReq::int4() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.int4)
  return int4_;
}
inline void TestReq::set_int4(::google::protobuf::uint64 value) {
  
  int4_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.int4)
}

// string str1 = 14;
inline void TestReq::clear_str1() {
  str1_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& TestReq::str1() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.str1)
  return str1_.GetNoArena();
}
inline void TestReq::set_str1(const ::std::string& value) {
  
  str1_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.TestReq.str1)
}
#if LANG_CXX11
inline void TestReq::set_str1(::std::string&& value) {
  
  str1_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.TestReq.str1)
}
#endif
inline void TestReq::set_str1(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  str1_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.TestReq.str1)
}
inline void TestReq::set_str1(const char* value, size_t size) {
  
  str1_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.TestReq.str1)
}
inline ::std::string* TestReq::mutable_str1() {
  
  // @@protoc_insertion_point(field_mutable:proto.TestReq.str1)
  return str1_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* TestReq::release_str1() {
  // @@protoc_insertion_point(field_release:proto.TestReq.str1)
  
  return str1_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestReq::set_allocated_str1(::std::string* str1) {
  if (str1 != NULL) {
    
  } else {
    
  }
  str1_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), str1);
  // @@protoc_insertion_point(field_set_allocated:proto.TestReq.str1)
}

// string str2 = 15;
inline void TestReq::clear_str2() {
  str2_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& TestReq::str2() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.str2)
  return str2_.GetNoArena();
}
inline void TestReq::set_str2(const ::std::string& value) {
  
  str2_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.TestReq.str2)
}
#if LANG_CXX11
inline void TestReq::set_str2(::std::string&& value) {
  
  str2_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.TestReq.str2)
}
#endif
inline void TestReq::set_str2(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  str2_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.TestReq.str2)
}
inline void TestReq::set_str2(const char* value, size_t size) {
  
  str2_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.TestReq.str2)
}
inline ::std::string* TestReq::mutable_str2() {
  
  // @@protoc_insertion_point(field_mutable:proto.TestReq.str2)
  return str2_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* TestReq::release_str2() {
  // @@protoc_insertion_point(field_release:proto.TestReq.str2)
  
  return str2_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestReq::set_allocated_str2(::std::string* str2) {
  if (str2 != NULL) {
    
  } else {
    
  }
  str2_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), str2);
  // @@protoc_insertion_point(field_set_allocated:proto.TestReq.str2)
}

// string str3 = 16;
inline void TestReq::clear_str3() {
  str3_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& TestReq::str3() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.str3)
  return str3_.GetNoArena();
}
inline void TestReq::set_str3(const ::std::string& value) {
  
  str3_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.TestReq.str3)
}
#if LANG_CXX11
inline void TestReq::set_str3(::std::string&& value) {
  
  str3_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.TestReq.str3)
}
#endif
inline void TestReq::set_str3(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  str3_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.TestReq.str3)
}
inline void TestReq::set_str3(const char* value, size_t size) {
  
  str3_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.TestReq.str3)
}
inline ::std::string* TestReq::mutable_str3() {
  
  // @@protoc_insertion_point(field_mutable:proto.TestReq.str3)
  return str3_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* TestReq::release_str3() {
  // @@protoc_insertion_point(field_release:proto.TestReq.str3)
  
  return str3_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestReq::set_allocated_str3(::std::string* str3) {
  if (str3 != NULL) {
    
  } else {
    
  }
  str3_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), str3);
  // @@protoc_insertion_point(field_set_allocated:proto.TestReq.str3)
}

// string str4 = 17;
inline void TestReq::clear_str4() {
  str4_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& TestReq::str4() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.str4)
  return str4_.GetNoArena();
}
inline void TestReq::set_str4(const ::std::string& value) {
  
  str4_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.TestReq.str4)
}
#if LANG_CXX11
inline void TestReq::set_str4(::std::string&& value) {
  
  str4_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.TestReq.str4)
}
#endif
inline void TestReq::set_str4(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  str4_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.TestReq.str4)
}
inline void TestReq::set_str4(const char* value, size_t size) {
  
  str4_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.TestReq.str4)
}
inline ::std::string* TestReq::mutable_str4() {
  
  // @@protoc_insertion_point(field_mutable:proto.TestReq.str4)
  return str4_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* TestReq::release_str4() {
  // @@protoc_insertion_point(field_release:proto.TestReq.str4)
  
  return str4_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void TestReq::set_allocated_str4(::std::string* str4) {
  if (str4 != NULL) {
    
  } else {
    
  }
  str4_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), str4);
  // @@protoc_insertion_point(field_set_allocated:proto.TestReq.str4)
}

// double dbl1 = 18;
inline void TestReq::clear_dbl1() {
  dbl1_ = 0;
}
inline double TestReq::dbl1() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.dbl1)
  return dbl1_;
}
inline void TestReq::set_dbl1(double value) {
  
  dbl1_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.dbl1)
}

// double dbl2 = 19;
inline void TestReq::clear_dbl2() {
  dbl2_ = 0;
}
inline double TestReq::dbl2() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.dbl2)
  return dbl2_;
}
inline void TestReq::set_dbl2(double value) {
  
  dbl2_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.dbl2)
}

// double dbl3 = 20;
inline void TestReq::clear_dbl3() {
  dbl3_ = 0;
}
inline double TestReq::dbl3() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.dbl3)
  return dbl3_;
}
inline void TestReq::set_dbl3(double value) {
  
  dbl3_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.dbl3)
}

// double dbl4 = 21;
inline void TestReq::clear_dbl4() {
  dbl4_ = 0;
}
inline double TestReq::dbl4() const {
  // @@protoc_insertion_point(field_get:proto.TestReq.dbl4)
  return dbl4_;
}
inline void TestReq::set_dbl4(double value) {
  
  dbl4_ = value;
  // @@protoc_insertion_point(field_set:proto.TestReq.dbl4)
}

// -------------------------------------------------------------------

// TestRsp

// .proto.Error error = 1;
inline bool TestRsp::has_error() const {
  return this != internal_default_instance() && error_ != NULL;
}
inline const ::proto::Error& TestRsp::error() const {
  const ::proto::Error* p = error_;
  // @@protoc_insertion_point(field_get:proto.TestRsp.error)
  return p != NULL ? *p : *reinterpret_cast<const ::proto::Error*>(
      &::proto::_Error_default_instance_);
}
inline ::proto::Error* TestRsp::release_error() {
  // @@protoc_insertion_point(field_release:proto.TestRsp.error)
  
  ::proto::Error* temp = error_;
  error_ = NULL;
  return temp;
}
inline ::proto::Error* TestRsp::mutable_error() {
  
  if (error_ == NULL) {
    error_ = new ::proto::Error;
  }
  // @@protoc_insertion_point(field_mutable:proto.TestRsp.error)
  return error_;
}
inline void TestRsp::set_allocated_error(::proto::Error* error) {
  ::google::protobuf::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == NULL) {
    delete reinterpret_cast< ::google::protobuf::MessageLite*>(error_);
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
  // @@protoc_insertion_point(field_set_allocated:proto.TestRsp.error)
}

// .proto.TestReq data = 2;
inline bool TestRsp::has_data() const {
  return this != internal_default_instance() && data_ != NULL;
}
inline void TestRsp::clear_data() {
  if (GetArenaNoVirtual() == NULL && data_ != NULL) {
    delete data_;
  }
  data_ = NULL;
}
inline const ::proto::TestReq& TestRsp::data() const {
  const ::proto::TestReq* p = data_;
  // @@protoc_insertion_point(field_get:proto.TestRsp.data)
  return p != NULL ? *p : *reinterpret_cast<const ::proto::TestReq*>(
      &::proto::_TestReq_default_instance_);
}
inline ::proto::TestReq* TestRsp::release_data() {
  // @@protoc_insertion_point(field_release:proto.TestRsp.data)
  
  ::proto::TestReq* temp = data_;
  data_ = NULL;
  return temp;
}
inline ::proto::TestReq* TestRsp::mutable_data() {
  
  if (data_ == NULL) {
    data_ = new ::proto::TestReq;
  }
  // @@protoc_insertion_point(field_mutable:proto.TestRsp.data)
  return data_;
}
inline void TestRsp::set_allocated_data(::proto::TestReq* data) {
  ::google::protobuf::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == NULL) {
    delete data_;
  }
  if (data) {
    ::google::protobuf::Arena* submessage_arena = NULL;
    if (message_arena != submessage_arena) {
      data = ::google::protobuf::internal::GetOwnedMessage(
          message_arena, data, submessage_arena);
    }
    
  } else {
    
  }
  data_ = data;
  // @@protoc_insertion_point(field_set_allocated:proto.TestRsp.data)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace proto

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_Test_2eproto__INCLUDED
