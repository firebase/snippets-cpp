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

#ifndef THIRD_PARTY_FIREBASE_CPP_FIRESTORE_INTEGRATION_TEST_INTERNAL_SRC_CONVERTER_H_
#define THIRD_PARTY_FIREBASE_CPP_FIRESTORE_INTEGRATION_TEST_INTERNAL_SRC_CONVERTER_H_

#include <map>
#include <vector>

#include "firebase/variant.h"
#include "firebase/firestore/field_value.h"
#include "firebase/firestore/map_field_value.h"

namespace firebase {
namespace firestore {

class Firestore;

// A sample class to demonstrate conversion between
// `firebase::firestore::FieldValue`s and `firebase::Variant`s.
//
// The converter aims to smooth over the differences between the `FieldValue`s
// and `Variant`s as much as possible:
// - nested `Variant` arrays are converted to "array-map-array" structures when
//   converting to a `FieldValue` (Firestore doesn't support nested arrays).
//   These structures round-trip;
// - Firestore entities that have no direct equivalent in `Variant` (e.g.
//   `Timestamp` or `DocumentReference`) are converted to maps. These maps also
//   round-trip.
//
// In cases where a lossless conversion is not possible (e.g.
// `FieldValue::ArrayUnion`) the converter aborts.
//
// IMPORTANT NOTE: in this sample code error handling is deliberately
// simplified. This is because the error-handling strategy is very much
// application-specific. Your application may want, for example, to return an
// error code, throw an exception, or simply log and continue, among other
// approaches.
class Converter {
 public:
  // The Firestore instance is used for converting Firestore document
  // references.
  explicit Converter(Firestore* firestore) : firestore_(firestore) {}

  FieldValue ConvertVariantToFieldValue(const Variant& from) const {
    return ConvertAny(from);
  }

  Variant ConvertFieldValueToVariant(const FieldValue& from) const;

 private:
  FieldValue ConvertAny(const Variant& from, bool within_array = false) const;
  FieldValue ConvertArray(const std::vector<Variant>& from, bool within_array) const;
  FieldValue ConvertRegularArray(const std::vector<Variant>& from) const;
  FieldValue ConvertMap(const std::map<Variant, Variant>& from) const;
  FieldValue ConvertRegularMap(const std::map<Variant, Variant>& from) const;
  FieldValue ConvertSpecialValue(const std::map<Variant, Variant>& from) const;

  Variant ConvertArray(const std::vector<FieldValue>& from) const;
  Variant ConvertMap(const MapFieldValue& from) const;
  Variant ConvertRegularMap(const MapFieldValue& from) const;
  Variant ConvertSpecialValue(const MapFieldValue& from) const;

  Firestore* firestore_ = nullptr;
};

}  // namespace firestore
}  // namespace firebase

#endif  // THIRD_PARTY_FIREBASE_CPP_FIRESTORE_INTEGRATION_TEST_INTERNAL_SRC_CONVERTER_H_
