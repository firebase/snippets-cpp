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

#include <cassert>
#include <exception>
#include <string>
#include <utility>

#include "firebase/firestore.h"

namespace firebase {
namespace firestore {

namespace {

// Helper functions to get values from maps of `Variant`s. Return
// a default-constructed value if the required key was not found.

bool TryGetBoolean(const std::map<Variant, Variant>& from, const char* key) {
  auto it = from.find(Variant(key));
  if (it == from.end() || !it->second.is_bool()) {
    return false;
  }

  return it->second.bool_value();
}

int64_t TryGetInteger(const std::map<Variant, Variant>& from, const char* key) {
  auto it = from.find(Variant(key));
  if (it == from.end() || !it->second.is_int64()) {
    return 0;
  }

  return it->second.int64_value();
}

double TryGetDouble(const std::map<Variant, Variant>& from, const char* key) {
  auto it = from.find(Variant(key));
  if (it == from.end() || !it->second.is_double()) {
    return 0.0;
  }

  return it->second.double_value();
}

std::string TryGetString(const std::map<Variant, Variant>& from,
                         const char* key) {
  auto it = from.find(Variant(key));
  if (it == from.end() || !it->second.is_string()) {
    return "";
  }

  return it->second.string_value();
}

// Helper functions to get values from maps of `FieldValue`s. Return
// a default-constructed value if the required key was not found.

bool TryGetBoolean(const MapFieldValue& from, const std::string& key) {
  auto it = from.find(key);
  if (it == from.end() || !it->second.is_boolean()) {
    return false;
  }

  return it->second.boolean_value();
}

std::string TryGetString(const MapFieldValue& from, const std::string& key) {
  auto it = from.find(key);
  if (it == from.end() || !it->second.is_string()) {
    return "";
  }

  return it->second.string_value();
}

std::vector<FieldValue> TryGetArray(const MapFieldValue& from,
                                    const std::string& key) {
  auto it = from.find(key);
  if (it == from.end() || !it->second.is_array()) {
    return {};
  }

  return it->second.array_value();
}

}  // namespace

// `Variant` -> `FieldValue`

FieldValue Converter::ConvertAny(const Variant& from, bool within_array) const {
  switch (from.type()) {
      // Primitives -- one-to-one mapping.

    case Variant::Type::kTypeNull:
      return FieldValue::Null();
    case Variant::Type::kTypeBool:
      return FieldValue::Boolean(from.bool_value());
    case Variant::Type::kTypeInt64:
      return FieldValue::Integer(from.int64_value());
    case Variant::Type::kTypeDouble:
      return FieldValue::Double(from.double_value());

      // Firestore does not have a distinction between static and mutable
      // strings and blobs. In all cases, the resulting `FieldValue` has
      // ownership of the underlying string or blob.

    case Variant::Type::kTypeStaticString:
    case Variant::Type::kTypeMutableString:
      return FieldValue::String(from.string_value());

    case Variant::Type::kTypeStaticBlob:
    case Variant::Type::kTypeMutableBlob:
      return FieldValue::Blob(from.blob_data(), from.blob_size());

      // Containers are converted recursively.

    case Variant::Type::kTypeVector:
      return ConvertArray(from.vector(), within_array);
    case Variant::Type::kTypeMap:
      return ConvertMap(from.map());

    default:
      assert(false && "Unknown Variant type");
      std::terminate();
  }
}

FieldValue Converter::ConvertArray(const std::vector<Variant>& from,
                                   bool within_array) const {
  if (!within_array) {
    return ConvertRegularArray(from);

  } else {
    // Firestore doesn't support nested arrays. As a workaround, create an
    // intermediate map to contain the nested array.
    return FieldValue::Map({
        {"special", FieldValue::Boolean(true)},
        {"type", FieldValue::String("nested_array")},
        {"value", ConvertRegularArray(from)},
    });
  }
}

FieldValue Converter::ConvertRegularArray(
    const std::vector<Variant>& from) const {
  std::vector<FieldValue> result;
  result.reserve(from.size());

  for (const auto& v : from) {
    result.push_back(ConvertAny(v, /*within_array=*/true));
  }

  return FieldValue::Array(std::move(result));
}

FieldValue Converter::ConvertMap(const std::map<Variant, Variant>& from) const {
  // Check if the map represents a special value (e.g. a `Timestamp`) that
  // should round-trip.
  bool is_special = TryGetBoolean(from, "special");
  if (is_special) {
    return ConvertSpecialValue(from);
  } else {
    return ConvertRegularMap(from);
  }
}

FieldValue Converter::ConvertRegularMap(
    const std::map<Variant, Variant>& from) const {
  MapFieldValue result;

  for (const auto& kv : from) {
    // Note: Firestore only supports string keys. If it's possible for the map
    // to contain non-string keys, you would have to convert them to a string
    // representation or skip them.
    assert(kv.first.is_string());
    result[kv.first.string_value()] = ConvertVariantToFieldValue(kv.second);
  }

  return FieldValue::Map(std::move(result));
}

FieldValue Converter::ConvertSpecialValue(
    const std::map<Variant, Variant>& from) const {
  // Special values are Firestore entities encoded as maps because they are not
  // directly supported by `Variant`. The assumption is that the map contains
  // a boolean field "special" set to true and a string field "type" indicating
  // which kind of an entity it contains.

  std::string type = TryGetString(from, "type");
  if (type.empty()) {
    return {};
  }

  if (type == "timestamp") {
    Timestamp result(TryGetInteger(from, "seconds"),
                     TryGetInteger(from, "nanoseconds"));
    return FieldValue::Timestamp(result);

  } else if (type == "geo_point") {
    GeoPoint result(TryGetDouble(from, "latitude"),
                    TryGetDouble(from, "longitude"));
    return FieldValue::GeoPoint(result);

  } else if (type == "document_reference") {
    DocumentReference result =
        firestore_->Document(TryGetString(from, "document_path"));
    return FieldValue::Reference(result);

  } else if (type == "delete") {
    return FieldValue::Delete();

  } else if (type == "server_timestamp") {
    return FieldValue::ServerTimestamp();
  }

  return {};
}

// `FieldValue` -> `Variant`

Variant Converter::ConvertFieldValueToVariant(const FieldValue& from) const {
  switch (from.type()) {
      // Primitives -- one-to-one mapping.

    case FieldValue::Type::kNull:
      return Variant::Null();
    case FieldValue::Type::kBoolean:
      return Variant(from.boolean_value());
    case FieldValue::Type::kInteger:
      return Variant(from.integer_value());
    case FieldValue::Type::kDouble:
      return Variant(from.double_value());

      // `FieldValue` always owns the underlying string or blob, so the safest
      // approach is to copy the value to a `Variant` that will assume
      // ownership.

    case FieldValue::Type::kString:
      return Variant(from.string_value());
    case FieldValue::Type::kBlob:
      return Variant::FromMutableBlob(from.blob_value(), from.blob_size());

      // Containers are converted recursively.

    case FieldValue::Type::kArray:
      return ConvertArray(from.array_value());
    case FieldValue::Type::kMap:
      return ConvertMap(from.map_value());

      // Firestore-specific structs are encoded as maps with a boolean field
      // "special" set to true and a string field "type" indicating the original
      // type.

    case FieldValue::Type::kTimestamp: {
      Timestamp ts = from.timestamp_value();
      MapFieldValue as_map = {
          {"special", FieldValue::Boolean(true)},
          {"type", FieldValue::String("timestamp")},
          {"seconds", FieldValue::Integer(ts.seconds())},
          {"nanoseconds", FieldValue::Integer(ts.nanoseconds())}};
      return ConvertRegularMap(as_map);
      // Note: if using the resulting `Variant` with RTDB, you might want to
      // convert timestamps to the number of milliseconds since Unix epoch:
      // Timestamp ts = from.timestamp_value();
      // int64_t millis = ts.seconds() * 1000 + ts.nanoseconds() / (1000*1000);
      // return Variant(millis);
    }

    case FieldValue::Type::kGeoPoint: {
      GeoPoint gp = from.geo_point_value();
      MapFieldValue as_map = {
          {"special", FieldValue::Boolean(true)},
          {"type", FieldValue::String("geo_point")},
          {"latitude", FieldValue::Double(gp.latitude())},
          {"longitude", FieldValue::Double(gp.longitude())}};
      return ConvertRegularMap(as_map);
    }

    case FieldValue::Type::kReference: {
      DocumentReference ref = from.reference_value();
      std::string path = ref.path();
      MapFieldValue as_map = {
          {"special", FieldValue::Boolean(true)},
          {"type", FieldValue::String("document_reference")},
          {"document_path", FieldValue::String(path)}};
      return ConvertRegularMap(as_map);
    }

      // Firestore-specific sentinel values can also be encoded as maps.

    case FieldValue::Type::kDelete: {
      // Note: if using the resulting `Variant` with RTDB, you might want to
      // convert a `delete` sentinel to null:
      // return Variant::Null();
      MapFieldValue as_map = {{"special", FieldValue::Boolean(true)},
                              {"type", FieldValue::String("delete")}};
      return ConvertRegularMap(as_map);
    }

    case FieldValue::Type::kServerTimestamp: {
      MapFieldValue as_map = {{"special", FieldValue::Boolean(true)},
                              {"type", FieldValue::String("server_timestamp")}};
      // Note: if using the resulting `Variant` with RTDB, you might want to
      // convert the server timestamp to the representation used by RTDB:
      // MapFieldValue as_map = {{".sv", FieldValue::String("timestamp")}};
      return ConvertRegularMap(as_map);
    }

      // Several Firestore sentinel values cannot be converted losslessly
      // (because they don't allow accessing the underlying value(s)). In this
      // example, we will simply assert that these values should never be given
      // to the converter.
    case FieldValue::Type::kArrayUnion:
    case FieldValue::Type::kArrayRemove:
    case FieldValue::Type::kIncrementInteger:
    case FieldValue::Type::kIncrementDouble:
      assert(false && "Unsupported FieldValue type");
      std::terminate();

    default:
      assert(false && "Unknown FieldValue type");
      std::terminate();
  }
}

Variant Converter::ConvertArray(const std::vector<FieldValue>& from) const {
  std::vector<Variant> result;
  result.reserve(from.size());

  for (const auto& v : from) {
    result.push_back(ConvertFieldValueToVariant(v));
  }

  return Variant(result);
}

Variant Converter::ConvertMap(const MapFieldValue& from) const {
  // Firestore doesn't support nested arrays, so nested arrays are instead
  // encoded as an "array-map-array" structure. Make sure nested arrays
  // round-trip.
  bool is_special = TryGetBoolean(from, "special");
  if (is_special) {
    return ConvertSpecialValue(from);
  } else {
    return ConvertRegularMap(from);
  }
}

Variant Converter::ConvertRegularMap(const MapFieldValue& from) const {
  std::map<Variant, Variant> result;

  for (const auto& kv : from) {
    result[Variant(kv.first)] = ConvertFieldValueToVariant(kv.second);
  }

  return Variant(result);
}

Variant Converter::ConvertSpecialValue(const MapFieldValue& from) const {
  std::string type = TryGetString(from, "type");
  if (type.empty()) {
    return Variant();
  }

  if (type == "nested_array") {
    // Unnest the array.
    return ConvertArray(TryGetArray(from, "value"));
  }

  return Variant();
}

}  // namespace firestore
}  // namespace firebase
