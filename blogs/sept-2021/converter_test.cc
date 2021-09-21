/*
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "converter.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "firebase/variant.h"
#include "firebase/firestore.h"
#include "firestore_integration_test.h"
#include "gtest/gtest.h"

namespace firebase {
namespace firestore {
namespace {

class ConverterTest : public FirestoreIntegrationTest {
 protected:
  FieldValue Convert(const Variant& from) const {
    return converter_.ConvertVariantToFieldValue(from);
  }

  Variant Convert(const FieldValue& from) const {
    return converter_.ConvertFieldValueToVariant(from);
  }

 private:
  Converter converter_{TestFirestore()};
};

TEST_F(ConverterTest, PrimitivesToVariant) {
  Variant null = Convert(FieldValue::Null());
  EXPECT_TRUE(null.is_null());

  Variant b = Convert(FieldValue::Boolean(true));
  EXPECT_EQ(b.bool_value(), true);

  Variant i = Convert(FieldValue::Integer(42));
  EXPECT_EQ(i.int64_value(), 42);

  Variant d = Convert(FieldValue::Double(42.0));
  EXPECT_EQ(d.double_value(), 42.0);

  Variant s = Convert(FieldValue::String("abc"));
  EXPECT_TRUE(s.is_mutable_string());
  EXPECT_STREQ(s.string_value(), "abc");

  uint8_t raw_blob[] = "( ͡° ͜ʖ ͡°)";
  Variant blob = Convert(FieldValue::Blob(raw_blob, sizeof(raw_blob)));
  EXPECT_TRUE(blob.is_mutable_blob());
  ASSERT_EQ(blob.blob_size(), sizeof(raw_blob));
  EXPECT_TRUE(std::equal(blob.blob_data(), blob.blob_data() + blob.blob_size(),
                         raw_blob));
}

TEST_F(ConverterTest, ArraysToVariant) {
  std::vector<FieldValue> vec = {
      FieldValue::Null(), FieldValue::Boolean(true), FieldValue::Integer(42),
      FieldValue::Double(123.0), FieldValue::String("abc")};
  Variant array = Convert(FieldValue::Array(vec));
  ASSERT_TRUE(array.is_vector());
  EXPECT_TRUE(array.vector()[0].is_null());
  EXPECT_EQ(array.vector()[1].bool_value(), true);
  EXPECT_EQ(array.vector()[2].int64_value(), 42);
  EXPECT_EQ(array.vector()[3].double_value(), 123.0);
  EXPECT_STREQ(array.vector()[4].string_value(), "abc");
}

TEST_F(ConverterTest, MapsToVariant) {
  std::vector<FieldValue> nested_vec = {FieldValue::String("def"),
                                        FieldValue::Null()};
  MapFieldValue nested_map = {{"boolean", FieldValue::Boolean(false)},
                              {"integer", FieldValue::Integer(456)}};
  MapFieldValue map_fv = {
      {"null", FieldValue::Null()},
      {"boolean", FieldValue::Boolean(true)},
      {"integer", FieldValue::Integer(42)},
      {"double", FieldValue::Double(123.0)},
      {"string", FieldValue::String("abc")},
      {"nested_array", FieldValue::Array(nested_vec)},
      {"nested_map", FieldValue::Map(nested_map)},
  };

  Variant map_variant = Convert(FieldValue::Map(map_fv));
  ASSERT_TRUE(map_variant.is_map());
  auto m = map_variant.map();

  EXPECT_TRUE(m[Variant("null")].is_null());
  EXPECT_EQ(m[Variant("boolean")].bool_value(), true);
  EXPECT_EQ(m[Variant("integer")].int64_value(), 42);
  EXPECT_EQ(m[Variant("double")].double_value(), 123.0);
  EXPECT_STREQ(m[Variant("string")].string_value(), "abc");

  EXPECT_STREQ(m[Variant("nested_array")].vector()[0].string_value(), "def");
  EXPECT_TRUE(m[Variant("nested_array")].vector()[1].is_null());

  EXPECT_EQ(m[Variant("nested_map")].map()[Variant("boolean")].bool_value(),
            false);
  EXPECT_EQ(m[Variant("nested_map")].map()[Variant("integer")].int64_value(),
            456);
}

TEST_F(ConverterTest, SpecialValuesToVariants) {
  {
    Timestamp original(123, 456);
    Variant ts = Convert(FieldValue::Timestamp(original));
    ASSERT_TRUE(ts.is_map());

    auto m = ts.map();
    EXPECT_TRUE(m[Variant("special")].bool_value());
    EXPECT_STREQ(m[Variant("type")].string_value(), "timestamp");
    EXPECT_EQ(m[Variant("seconds")].int64_value(), 123);
    EXPECT_EQ(m[Variant("nanoseconds")].int64_value(), 456);

    EXPECT_EQ(Convert(ts).timestamp_value(), original);
  }

  {
    GeoPoint original(43.0, 80.0);
    Variant gp = Convert(FieldValue::GeoPoint(original));
    ASSERT_TRUE(gp.is_map());

    auto m = gp.map();
    EXPECT_TRUE(m[Variant("special")].bool_value());
    EXPECT_STREQ(m[Variant("type")].string_value(), "geo_point");
    EXPECT_EQ(m[Variant("latitude")].double_value(), 43.0);
    EXPECT_EQ(m[Variant("longitude")].double_value(), 80.0);

    EXPECT_EQ(Convert(gp).geo_point_value(), original);
  }

  {
    DocumentReference doc = TestFirestore()->Document("foo/bar");
    Variant ref = Convert(FieldValue::Reference(doc));
    ASSERT_TRUE(ref.is_map());
    auto m = ref.map();
    EXPECT_TRUE(m[Variant("special")].bool_value());
    EXPECT_STREQ(m[Variant("type")].string_value(), "document_reference");
    EXPECT_STREQ(m[Variant("document_path")].string_value(), "foo/bar");

    EXPECT_EQ(Convert(ref).reference_value(), doc);
  }

  {
    Variant del = Convert(FieldValue::Delete());
    ASSERT_TRUE(del.is_map());
    auto m = del.map();
    EXPECT_TRUE(m[Variant("special")].bool_value());
    EXPECT_STREQ(m[Variant("type")].string_value(), "delete");

    EXPECT_EQ(Convert(del).type(), FieldValue::Type::kDelete);
  }

  {
    Variant server_ts = Convert(FieldValue::ServerTimestamp());
    ASSERT_TRUE(server_ts.is_map());
    auto m = server_ts.map();
    EXPECT_TRUE(m[Variant("special")].bool_value());
    EXPECT_STREQ(m[Variant("type")].string_value(), "server_timestamp");

    EXPECT_EQ(Convert(server_ts).type(), FieldValue::Type::kServerTimestamp);
  }
}

TEST_F(ConverterTest, PrimitivesToFieldValue) {
  FieldValue null = Convert(Variant::Null());
  EXPECT_TRUE(null.is_null());

  FieldValue b = Convert(Variant(true));
  EXPECT_EQ(b.boolean_value(), true);

  FieldValue i = Convert(Variant(42));
  EXPECT_EQ(i.integer_value(), 42);

  FieldValue d = Convert(Variant(42.0));
  EXPECT_EQ(d.double_value(), 42.0);

  FieldValue static_str = Convert(Variant("abc"));
  EXPECT_EQ(static_str.string_value(), "abc");

  FieldValue mutable_str = Convert(Variant(std::string("abc")));
  EXPECT_EQ(mutable_str.string_value(), "abc");

  uint8_t raw_blob[] = "( ͡° ͜ʖ ͡°)";
  FieldValue static_blob =
      Convert(Variant::FromStaticBlob(raw_blob, sizeof(raw_blob)));
  ASSERT_EQ(static_blob.blob_size(), sizeof(raw_blob));
  EXPECT_TRUE(std::equal(static_blob.blob_value(),
                         static_blob.blob_value() + static_blob.blob_size(),
                         raw_blob));

  FieldValue mutable_blob =
      Convert(Variant::FromMutableBlob(raw_blob, sizeof(raw_blob)));
  ASSERT_EQ(mutable_blob.blob_size(), sizeof(raw_blob));
  EXPECT_TRUE(std::equal(mutable_blob.blob_value(),
                         mutable_blob.blob_value() + mutable_blob.blob_size(),
                         raw_blob));
}

TEST_F(ConverterTest, ArraysToFieldValue) {
  std::vector<Variant> vec = {Variant::Null(), Variant(true), Variant(42),
                              Variant(123.0), Variant("abc")};
  FieldValue array = Convert(Variant(vec));
  ASSERT_TRUE(array.is_array());
  EXPECT_TRUE(array.array_value()[0].is_null());
  EXPECT_EQ(array.array_value()[1].boolean_value(), true);
  EXPECT_EQ(array.array_value()[2].integer_value(), 42);
  EXPECT_EQ(array.array_value()[3].double_value(), 123.0);
  EXPECT_EQ(array.array_value()[4].string_value(), "abc");
}

TEST_F(ConverterTest, NestedArraysToFieldValue) {
  std::vector<Variant> vec = {Variant(std::vector<Variant>{Variant("abc")})};

  FieldValue array = Convert(Variant(vec));
  ASSERT_TRUE(array.is_array());

  ASSERT_EQ(array.array_value().size(), 1);
  ASSERT_TRUE(array.array_value()[0].is_map());
  auto m = array.array_value()[0].map_value();

  EXPECT_TRUE(m["special"].boolean_value());
  EXPECT_EQ(m["type"].string_value(), "nested_array");
  ASSERT_TRUE(m["value"].is_array());

  std::vector<FieldValue> nested = m["value"].array_value();
  ASSERT_EQ(nested.size(), 1);
  EXPECT_EQ(nested[0].string_value(), "abc");

  Variant roundtrip = Convert(array);
  ASSERT_TRUE(roundtrip.is_vector());
  ASSERT_EQ(roundtrip.vector().size(), 1);
  ASSERT_TRUE(roundtrip.vector()[0].is_vector());
  ASSERT_EQ(roundtrip.vector()[0].vector().size(), 1);
  ASSERT_STREQ(roundtrip.vector()[0].vector()[0].string_value(), "abc");
}

TEST_F(ConverterTest, MapsToFieldValue) {
  std::vector<Variant> nested_vec = {Variant("def"), Variant::Null()};
  std::map<Variant, Variant> nested_map = {{Variant("boolean"), Variant(false)},
                                           {Variant("integer"), Variant(456)}};
  std::map<Variant, Variant> map_variant = {
      {Variant("null"), Variant::Null()},
      {Variant("boolean"), Variant(true)},
      {Variant("integer"), Variant(42)},
      {Variant("double"), Variant(123.0)},
      {Variant("string"), Variant("abc")},
      {Variant("nested_array"), Variant(nested_vec)},
      {Variant("nested_map"), Variant(nested_map)},
  };

  FieldValue map_fv = Convert(Variant(map_variant));
  ASSERT_TRUE(map_fv.is_map());
  auto m = map_fv.map_value();

  EXPECT_TRUE(m["null"].is_null());
  EXPECT_EQ(m["boolean"].boolean_value(), true);
  EXPECT_EQ(m["integer"].integer_value(), 42);
  EXPECT_EQ(m["double"].double_value(), 123.0);
  EXPECT_EQ(m["string"].string_value(), "abc");

  EXPECT_EQ(m["nested_array"].array_value()[0].string_value(), "def");
  EXPECT_TRUE(m["nested_array"].array_value()[1].is_null());

  EXPECT_EQ(m["nested_map"].map_value()["boolean"].boolean_value(), false);
  EXPECT_EQ(m["nested_map"].map_value()["integer"].integer_value(), 456);
}

}  // namespace
}  // namespace firestore
}  // namespace firebase
