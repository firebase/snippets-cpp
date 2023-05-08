//
//  Copyright (c) 2020 Google Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include <cstdint>
#include <iostream>
#include <string>

#include "snippets.h"
#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/auth/user.h"
#include "firebase/firestore.h"
#include "firebase/util.h"

/*
 * A collection of code snippets for the Firestore C++ SDK. These snippets
 * were modeled after the existing Firestore guide, which can be found
 * here: https://firebase.google.com/docs/firestore.
 *
 * Note that not all of the Firestore API has been implemented yet, so some
 * snippets are incomplete/missing.
 */

namespace snippets {

// https://firebase.google.com/docs/firestore/data-model#references
void DataModelReferenceDeclarations(firebase::firestore::Firestore* db) {
  using firebase::firestore::DocumentReference;
  using firebase::firestore::CollectionReference;

  // [START doc_reference]
  DocumentReference alovelace_document_reference =
      db->Collection("users").Document("alovelace");
  // [END doc_reference]

  // [START collection_reference]
  CollectionReference users_collection_reference = db->Collection("users");
  // [END collection_reference]

  // https://firebase.google.com/docs/firestore/data-model#hierarchical-data
  // [START subcollection_reference]
  DocumentReference message_reference = db->Collection("rooms")
      .Document("roomA")
      .Collection("messages")
      .Document("message1");
  // [END subcollection_reference]

  // [START path_reference]
  DocumentReference alovelace_document = db->Document("users/alovelace");
  // [END path_reference]
}

// https://firebase.google.com/docs/firestore/quickstart#add_data
void QuickstartAddData(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;

  // Firestore stores data in Documents, which are stored in Collections.
  // Firestore creates collections and documents implicitly the first time
  // you add data to the document. You do not need to explicitly create
  // collections or documents.

  // [START add_ada_lovelace]
  // Add a new document with a generated ID
  Future<DocumentReference> user_ref =
      db->Collection("users").Add({{"first", FieldValue::String("Ada")},
                                   {"last", FieldValue::String("Lovelace")},
                                   {"born", FieldValue::Integer(1815)}});

  user_ref.OnCompletion([](const Future<DocumentReference>& future) {
    if (future.error() == Error::kErrorOk) {
      std::cout << "DocumentSnapshot added with ID: " << future.result()->id()
                << std::endl;
    } else {
      std::cout << "Error adding document: " << future.error_message() << std::endl;
    }
  });
  // [END add_ada_lovelace]

  // Now add another document to the users collection. Notice that this document
  // includes a key-value pair (middle name) that does not appear in the first
  // document. Documents in a collection can contain different sets of
  // information.

  // [START add_alan_turing]
  db->Collection("users")
      .Add({{"first", FieldValue::String("Alan")},
            {"middle", FieldValue::String("Mathison")},
            {"last", FieldValue::String("Turing")},
            {"born", FieldValue::Integer(1912)}})
      .OnCompletion([](const Future<DocumentReference>& future) {
        if (future.error() == Error::kErrorOk) {
          std::cout << "DocumentSnapshot added with ID: "
                    << future.result()->id() << std::endl;
        } else {
          std::cout << "Error adding document: " << future.error_message()
                    << std::endl;
        }
      });
  // [END add_alan_turing]
}

// https://firebase.google.com/docs/firestore/quickstart#read_data
void QuickstartReadData(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::QuerySnapshot;

  // To quickly verify that you've added data to Firestore, use the data
  // viewer in the Firebase console.
  //
  // You can also use the "Get" method to retrieve the entire collection.
  // [START get_collection]
  Future<QuerySnapshot> users = db->Collection("users").Get();
  users.OnCompletion([](const Future<QuerySnapshot>& future) {
    if (future.error() == Error::kErrorOk) {
      for (const DocumentSnapshot& document : future.result()->documents()) {
        std::cout << document << std::endl;
      }
    } else {
      std::cout << "Error getting documents: " << future.error_message()
                << std::endl;
    }
  });
  // [END get_collection]
}

// https://firebase.google.com/docs/firestore/manage-data/add-data#set_a_document
void AddDataSetDocument(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::SetOptions;

  // To create or overwrite a single document, use the Set() method:
  // [START set_document]
  // Add a new document in collection 'cities'
  db->Collection("cities")
      .Document("LA")
      .Set({{"name", FieldValue::String("Los Angeles")},
            {"state", FieldValue::String("CA")},
            {"country", FieldValue::String("USA")}})
      .OnCompletion([](const Future<void>& future) {
        if (future.error() == Error::kErrorOk) {
          std::cout << "DocumentSnapshot successfully written!" << std::endl;
        } else {
          std::cout << "Error writing document: " << future.error_message()
                    << std::endl;
        }
      });
  // [END set_document]

  // If the document does not exist, it will be created. If the document does
  // exist, its contents will be overwritten with the newly provided data,
  // unless you specify that the data should be merged into the existing
  // document, as follows:
  // [START create_if_missing]
  db->Collection("cities").Document("BJ").Set(
      {{"capital", FieldValue::Boolean(true)}}, SetOptions::Merge());
  // [END create_if_missing]
}

// https://firebase.google.com/docs/firestore/manage-data/add-data#data_types
void AddDataDataTypes(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::Timestamp;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::MapFieldValue;

  // Firestore lets you write a variety of data types inside a document,
  // including strings, booleans, numbers, dates, null, and nested arrays and
  // objects. Firestore always stores numbers as doubles, regardless of
  // what type of number you use in your code.
  // [START data_types]
  MapFieldValue doc_data{
      {"stringExample", FieldValue::String("Hello world!")},
      {"booleanExample", FieldValue::Boolean(true)},
      {"numberExample", FieldValue::Double(3.14159265)},
      {"dateExample", FieldValue::Timestamp(Timestamp::Now())},
      {"arrayExample", FieldValue::Array({FieldValue::Integer(1),
                                          FieldValue::Integer(2),
                                          FieldValue::Integer(3)})},
      {"nullExample", FieldValue::Null()},
      {"objectExample",
       FieldValue::Map(
           {{"a", FieldValue::Integer(5)},
            {"b", FieldValue::Map(
                      {{"nested", FieldValue::String("foo")}})}})},
  };

  db->Collection("data").Document("one").Set(doc_data).OnCompletion(
      [](const Future<void>& future) {
        if (future.error() == Error::kErrorOk) {
          std::cout << "DocumentSnapshot successfully written!" << std::endl;
        } else {
          std::cout << "Error writing document: " << future.error_message()
                    << std::endl;
        }
      });
  // [END data_types]
}

// https://firebase.google.com/docs/firestore/manage-data/add-data#add_a_document
void AddDataAddDocument(firebase::firestore::Firestore* db) {
  using firebase::firestore::DocumentReference;

  // When you use Set() to create a document, you must specify an ID for the
  // document to create. For example:
  // [START set_data]
  db->Collection("cities").Document("SF").Set({/*some data*/});
  // [END set_data]

  // But sometimes there isn't a meaningful ID for the document, and it's more
  // convenient to let Firestore auto-generate an ID for you. You can do
  // this by calling Add():
  // [START add_document]
  db->Collection("cities").Add({/*some data*/});
  // [END add_document]

  // In some cases, it can be useful to create a document reference with an
  // auto-generated ID, then use the reference later. For this use case, you can
  // call Document():

  // [START new_document]
  DocumentReference new_city_ref = db->Collection("cities").Document();
  // [END new_document]
  // Behind the scenes, Add(...) and Document().Set(...) are completely
  // equivalent, so you can use whichever is more convenient.
}

// https://firebase.google.com/docs/firestore/manage-data/add-data#update-data
void AddDataUpdateDocument(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::FieldValue;

  // To update some fields of a document without overwriting the entire
  // document, use the Update() method:
  // [START update_document]
  DocumentReference washington_ref = db->Collection("cities").Document("DC");
  // Set the "capital" field of the city "DC".
  washington_ref.Update({{"capital", FieldValue::Boolean(true)}});
  // [END update_document]

  // You can set a field in your document to a server timestamp which tracks
  // when the server receives the update.
  // [START server_timestamp]
  DocumentReference doc_ref = db->Collection("objects").Document("some-id");
  doc_ref.Update({{"timestamp", FieldValue::ServerTimestamp()}})
      .OnCompletion([](const Future<void>& future) {
        // ...
      });
  // [END server_timestamp]
}

// https://firebase.google.com/docs/firestore/manage-data/add-data#update_fields_in_nested_objects
void AddDataUpdateNestedObjects(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::FieldValue;

  // If your document contains nested objects, you can use "dot notation" to
  // reference nested fields within the document when you call Update():
  // [START update_document_nested]
  // Assume the document contains:
  // {
  //   name: "Frank",
  //   favorites: { food: "Pizza", color: "Blue", subject: "recess" }
  //   age: 12
  // }
  //
  // To update age and favorite color:
  db->Collection("users").Document("frank").Update({
      {"age", FieldValue::Integer(13)},
      {"favorites.color", FieldValue::String("red")},
  });
  // [END update_document_nested]
  // Dot notation allows you to update a single nested field without overwriting
  // other nested fields. If you update a nested field without dot notation, you
  // will overwrite the entire map field.
}

// https://firebase.google.com/docs/firestore/manage-data/transactions#batched-writes
void AddDataBatchedWrites(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::WriteBatch;

  // If you do not need to read any documents in your operation set, you can
  // execute multiple write operations as a single batch that contains any
  // combination of set(), update(), or delete() operations. A batch of writes
  // completes atomically and can write to multiple documents. The following
  // example shows how to build and commit a write batch:

  // [START write_batch]
  // Get a new write batch
  WriteBatch batch = db->batch();

  // Set the value of 'NYC'
  DocumentReference nyc_ref = db->Collection("cities").Document("NYC");
  batch.Set(nyc_ref, {});

  // Update the population of 'SF'
  DocumentReference sf_ref = db->Collection("cities").Document("SF");
  batch.Update(sf_ref, {{"population", FieldValue::Integer(1000000)}});

  // Delete the city 'LA'
  DocumentReference la_ref = db->Collection("cities").Document("LA");
  batch.Delete(la_ref);

  // Commit the batch
  batch.Commit().OnCompletion([](const Future<void>& future) {
    if (future.error() == Error::kErrorOk) {
      std::cout << "Write batch success!" << std::endl;
    } else {
      std::cout << "Write batch failure: " << future.error_message() << std::endl;
    }
  });
  // [END write_batch]
}

// https://firebase.google.com/docs/firestore/manage-data/transactions#transactions
void AddDataTransactions(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::Transaction;

  // The following example shows how to create and run a transaction:
  // [START simple_transaction]
  DocumentReference sf_doc_ref = db->Collection("cities").Document("SF");
  db->RunTransaction([sf_doc_ref](Transaction& transaction,
                                  std::string& out_error_message) -> Error {
      Error error = Error::kErrorOk;

      DocumentSnapshot snapshot =
          transaction.Get(sf_doc_ref, &error, &out_error_message);

      // Note: this could be done without a transaction by updating the
      // population using FieldValue::Increment().
      std::int64_t new_population =
          snapshot.Get("population").integer_value() + 1;
      transaction.Update(
          sf_doc_ref,
          {{"population", FieldValue::Integer(new_population)}});

      return Error::kErrorOk;
    }).OnCompletion([](const Future<void>& future) {
    if (future.error() == Error::kErrorOk) {
      std::cout << "Transaction success!" << std::endl;
    } else {
      std::cout << "Transaction failure: " << future.error_message() << std::endl;
    }
  });
  // [END simple_transaction]
}

// https://firebase.google.com/docs/firestore/manage-data/delete-data#delete_documents
void AddDataDeleteDocuments(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::Error;

  // To delete a document, use the Delete() method:
  // [START delete_document]
  db->Collection("cities").Document("DC").Delete().OnCompletion(
      [](const Future<void>& future) {
        if (future.error() == Error::kErrorOk) {
          std::cout << "DocumentSnapshot successfully deleted!" << std::endl;
        } else {
          std::cout << "Error deleting document: " << future.error_message()
                    << std::endl;
        }
      });
  // [END delete_document]
  // WARNING: Deleting a document does not delete its subcollections!
}

// https://firebase.google.com/docs/firestore/manage-data/delete-data#fields
void AddDataDeleteFields(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::FieldValue;

  // To delete specific fields from a document, use the FieldValue.delete()
  // method when you update a document:
  // [START delete_field]
  DocumentReference doc_ref = db->Collection("cities").Document("BJ");
  doc_ref.Update({{"capital", FieldValue::Delete()}})
      .OnCompletion([](const Future<void>& future) { /*...*/ });
  // [END delete_field]

  // https://firebase.google.com/docs/firestore/manage-data/delete-data#collections
  // To delete an entire collection or subcollection in Firestore,
  // retrieve all the documents within the collection or subcollection and
  // delete them.
  // WARNING: deleting collections from a client SDK is not recommended.
}

// https://firebase.google.com/docs/firestore/query-data/get-data#example_data
void ReadDataExampleData(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;

  // To get started, write some data about cities so we can look at different
  // ways to read it back:

  // [START example_data]
  CollectionReference cities = db->Collection("cities");

  cities.Document("SF").Set({
      {"name", FieldValue::String("San Francisco")},
      {"state", FieldValue::String("CA")},
      {"country", FieldValue::String("USA")},
      {"capital", FieldValue::Boolean(false)},
      {"population", FieldValue::Integer(860000)},
      {"regions", FieldValue::Array({FieldValue::String("west_coast"),
                                     FieldValue::String("norcal")})},
  });

  cities.Document("LA").Set({
      {"name", FieldValue::String("Los Angeles")},
      {"state", FieldValue::String("CA")},
      {"country", FieldValue::String("USA")},
      {"capital", FieldValue::Boolean(false)},
      {"population", FieldValue::Integer(3900000)},
      {"regions", FieldValue::Array({FieldValue::String("west_coast"),
                                     FieldValue::String("socal")})},
  });

  cities.Document("DC").Set({
      {"name", FieldValue::String("Washington D.C.")},
      {"state", FieldValue::Null()},
      {"country", FieldValue::String("USA")},
      {"capital", FieldValue::Boolean(true)},
      {"population", FieldValue::Integer(680000)},
      {"regions",
       FieldValue::Array({FieldValue::String("east_coast")})},
  });

  cities.Document("TOK").Set({
      {"name", FieldValue::String("Tokyo")},
      {"state", FieldValue::Null()},
      {"country", FieldValue::String("Japan")},
      {"capital", FieldValue::Boolean(true)},
      {"population", FieldValue::Integer(9000000)},
      {"regions", FieldValue::Array({FieldValue::String("kanto"),
                                     FieldValue::String("honshu")})},
  });

  cities.Document("BJ").Set({
      {"name", FieldValue::String("Beijing")},
      {"state", FieldValue::Null()},
      {"country", FieldValue::String("China")},
      {"capital", FieldValue::Boolean(true)},
      {"population", FieldValue::Integer(21500000)},
      {"regions", FieldValue::Array({FieldValue::String("jingjinji"),
                                     FieldValue::String("hebei")})},
  });
  // [END example_data]
}

// https://firebase.google.com/docs/firestore/query-data/get-data#get_a_document
void ReadDataGetDocument(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;

  // The following example shows how to retrieve the contents of a single
  // document using Get():
  // [START get_document]
  DocumentReference doc_ref = db->Collection("cities").Document("SF");
  doc_ref.Get().OnCompletion([](const Future<DocumentSnapshot>& future) {
    if (future.error() == Error::kErrorOk) {
      const DocumentSnapshot& document = *future.result();
      if (document.exists()) {
        std::cout << "DocumentSnapshot id: " << document.id() << std::endl;
      } else {
        std::cout << "no such document" << std::endl;
      }
    } else {
      std::cout << "Get failed with: " << future.error_message() << std::endl;
    }
  });
  // [END get_document]
}

void ReadDataSourceOptions(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::Source;

  // You can set the source option to control how a get call uses the offline
  // cache.
  //
  // By default, a get call will attempt to fetch the latest document snapshot
  // from your database. On platforms with offline support, the client library
  // will use the offline cache if the network is unavailable or if the request
  // times out.
  //
  // You can specify the source option in a Get() call to change the default
  // behavior. You can fetch from only the database and ignore the offline
  // cache, or you can fetch from only the offline cache. For example:
  // [START get_document_options]
  DocumentReference doc_ref = db->Collection("cities").Document("SF");
  Source source = Source::kCache;
  doc_ref.Get(source).OnCompletion([](const Future<DocumentSnapshot>& future) {
    if (future.error() == Error::kErrorOk) {
      const DocumentSnapshot& document = *future.result();
      if (document.exists()) {
        std::cout << "Cached document id: " << document.id() << std::endl;
      } else {
      }
    } else {
      std::cout << "Cached get failed: " << future.error_message() << std::endl;
    }
  });
  // [END get_document_options]
}

// https://firebase.google.com/docs/firestore/query-data/get-data#get_multiple_documents_from_a_collection
void ReadDataGetMultipleDocumentsFromCollection(
    firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::QuerySnapshot;

  // You can also retrieve multiple documents with one request by querying
  // documents in a collection. For example, you can use Where() to query for
  // all of the documents that meet a certain condition, then use Get() to
  // retrieve the results:
  // [START get_multiple]
  db->Collection("cities")
      .WhereEqualTo("capital", FieldValue::Boolean(true))
      .Get()
      .OnCompletion([](const Future<QuerySnapshot>& future) {
        if (future.error() == Error::kErrorOk) {
          for (const DocumentSnapshot& document :
               future.result()->documents()) {
            std::cout << document << std::endl;
          }
        } else {
          std::cout << "Error getting documents: " << future.error_message()
                    << std::endl;
        }
      });
  // [END get_multiple]
}

// https://firebase.google.com/docs/firestore/query-data/get-data#get_all_documents_in_a_collection
void ReadDataGetAllDocumentsInCollection(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::QuerySnapshot;

  // In addition, you can retrieve all documents in a collection by omitting the
  // Where() filter entirely:
  // [START get_multiple_all]
  db->Collection("cities").Get().OnCompletion(
      [](const Future<QuerySnapshot>& future) {
        if (future.error() == Error::kErrorOk) {
          for (const DocumentSnapshot& document :
               future.result()->documents()) {
            std::cout << document << std::endl;
          }
        } else {
          std::cout << "Error getting documents: " << future.error_message()
                    << std::endl;
        }
      });
  // [END get_multiple_all]
}

// https://firebase.google.com/docs/firestore/query-data/listen
void ReadDataListen(firebase::firestore::Firestore* db) {
  using firebase::firestore::DocumentReference;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;

  // You can listen to a document with the AddSnapshotListener() method. An
  // initial call using the callback you provide creates a document snapshot
  // immediately with the current contents of the single document. Then, each
  // time the contents change, another call updates the document snapshot.
  // [START listen_document]
  DocumentReference doc_ref = db->Collection("cities").Document("SF");
  doc_ref.AddSnapshotListener(
      [](const DocumentSnapshot& snapshot, Error error, const std::string& errorMsg) {
        if (error == Error::kErrorOk) {
          if (snapshot.exists()) {
            std::cout << "Current data: " << snapshot << std::endl;
          } else {
            std::cout << "Current data: null" << std::endl;
          }
        } else {
          std::cout << "Listen failed: " << error << std::endl;
        }
      });
  // [END listen_document]
}

// https://firebase.google.com/docs/firestore/query-data/listen#events-local-changes
void ReadDataEventsForLocalChanges(firebase::firestore::Firestore* db) {
  using firebase::firestore::DocumentReference;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;

  // Local writes in your app will invoke snapshot listeners immediately. This
  // is because of an important feature called "latency compensation." When you
  // perform a write, your listeners will be notified with the new data before
  // the data is sent to the backend.
  //
  // Retrieved documents have metadata().has_pending_writes() property that
  // indicates whether the document has local changes that haven't been written
  // to the backend yet. You can use this property to determine the source of
  // events received by your snapshot listener:

  // [START listen_document_local]
  DocumentReference doc_ref = db->Collection("cities").Document("SF");
  doc_ref.AddSnapshotListener([](const DocumentSnapshot& snapshot,
                                 Error error, const std::string& errorMsg) {
    if (error == Error::kErrorOk) {
      const char* source =
          snapshot.metadata().has_pending_writes() ? "Local" : "Server";
      if (snapshot.exists()) {
        std::cout << source << " data: " << snapshot.Get("name").string_value()
                  << std::endl;
      } else {
        std::cout << source << " data: null" << std::endl;
      }
    } else {
      std::cout << "Listen failed: " << error << std::endl;
    }
  });
  // [END listen_document_local]
}

// https://firebase.google.com/docs/firestore/query-data/listen#events-metadata-changes
void ReadDataEventsForMetadataChanges(firebase::firestore::Firestore* db) {
  using firebase::firestore::DocumentReference;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::MetadataChanges;

  // When listening for changes to a document, collection, or query, you can
  // pass options to control the granularity of events that your listener will
  // receive.
  //
  // By default, listeners are not notified of changes that only affect
  // metadata. Consider what happens when your app writes a new document:
  //
  // A change event is immediately fired with the new data. The document has not
  // yet been written to the backend so the "pending writes" flag is true.
  // The document is written to the backend.
  // The backend notifies the client of the successful write. There is no change
  // to the document data, but there is a metadata change because the "pending
  // writes" flag is now false.
  // If you want to receive snapshot events when the document or query metadata
  // changes, pass a listen options object when attaching your listener:
  // [START listen_with_metadata]
  DocumentReference doc_ref = db->Collection("cities").Document("SF");
  doc_ref.AddSnapshotListener(
      MetadataChanges::kInclude,
      [](const DocumentSnapshot& snapshot, Error error, const std::string& errorMsg) { /* ... */ });
  // [END listen_with_metadata]
}

// https://firebase.google.com/docs/firestore/query-data/listen#listen_to_multiple_documents_in_a_collection
void ReadDataListenToMultipleDocumentsInCollection(
    firebase::firestore::Firestore* db) {
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::QuerySnapshot;

  // As with documents, you can use AddSnapshotListener() instead of Get() to
  // listen to the results of a query. This creates a query snapshot. For
  // example, to listen to the documents with state CA:
  // [START listen_multiple]
  db->Collection("cities")
      .WhereEqualTo("state", FieldValue::String("CA"))
      .AddSnapshotListener([](const QuerySnapshot& snapshot, Error error, const std::string& errorMsg) {
        if (error == Error::kErrorOk) {
          std::vector<std::string> cities;
          std::cout << "Current cities in CA: " << error << std::endl;
          for (const DocumentSnapshot& doc : snapshot.documents()) {
            cities.push_back(doc.Get("name").string_value());
            std::cout << "" << cities.back() << std::endl;
          }
        } else {
          std::cout << "Listen failed: " << error << std::endl;
        }
      });
  // [END listen_multiple]

  // The snapshot handler will receive a new query snapshot every time the query
  // results change (that is, when a document is added, removed, or modified).
}

// https://firebase.google.com/docs/firestore/query-data/listen#view_changes_between_snapshots
void ReadDataViewChangesBetweenSnapshots(firebase::firestore::Firestore* db) {
  using firebase::firestore::DocumentChange;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::QuerySnapshot;

  // It is often useful to see the actual changes to query results between query
  // snapshots, instead of simply using the entire query snapshot. For example,
  // you may want to maintain a cache as individual documents are added,
  // removed, and modified.
  // [START listen_diffs]
  db->Collection("cities")
      .WhereEqualTo("state", FieldValue::String("CA"))
      .AddSnapshotListener([](const QuerySnapshot& snapshot, Error error, const std::string& errorMsg) {
        if (error == Error::kErrorOk) {
          for (const DocumentChange& dc : snapshot.DocumentChanges()) {
            switch (dc.type()) {
              case DocumentChange::Type::kAdded:
                std::cout << "New city: "
                          << dc.document().Get("name").string_value() << std::endl;
                break;
              case DocumentChange::Type::kModified:
                std::cout << "Modified city: "
                          << dc.document().Get("name").string_value() << std::endl;
                break;
              case DocumentChange::Type::kRemoved:
                std::cout << "Removed city: "
                          << dc.document().Get("name").string_value() << std::endl;
                break;
            }
          }
        } else {
          std::cout << "Listen failed: " << error << std::endl;
        }
      });
  // [END listen_diffs]
}

// https://firebase.google.com/docs/firestore/query-data/listen#detach_a_listener
void ReadDataDetachListener(firebase::firestore::Firestore* db) {
  using firebase::firestore::Error;
  using firebase::firestore::ListenerRegistration;
  using firebase::firestore::Query;
  using firebase::firestore::QuerySnapshot;

  // When you are no longer interested in listening to your data, you must
  // detach your listener so that your event callbacks stop getting called. This
  // allows the client to stop using bandwidth to receive updates. For example:
  // [START detach_listener]
  // Add a listener
  Query query = db->Collection("cities");
  ListenerRegistration registration = query.AddSnapshotListener(
      [](const QuerySnapshot& snapshot, Error error, const std::string& errorMsg) { /* ... */ });
  // Stop listening to changes
  registration.Remove();
  // [END detach_listener]

  // A listen may occasionally fail — for example, due to security permissions,
  // or if you tried to listen on an invalid query. After an error, the listener
  // will not receive any more events, and there is no need to detach your
  // listener.
}

// https://firebase.google.com/docs/firestore/query-data/queries#simple_queries
void ReadDataSimpleQueries(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;
  using firebase::firestore::Query;

  // Firestore provides powerful query functionality for specifying which
  // documents you want to retrieve from a collection.

  // The following query returns all cities with state CA:
  // [START simple_queries]
  CollectionReference cities_ref = db->Collection("cities");
  // Create a query against the collection.
  Query query_ca =
      cities_ref.WhereEqualTo("state", FieldValue::String("CA"));
  // [END simple_queries]

  // The following query returns all the capital cities:
  // [START query_capitals]
  Query capital_cities = db->Collection("cities").WhereEqualTo(
      "capital", FieldValue::Boolean(true));
  // [END query_capitals]
}

// https://firebase.google.com/docs/firestore/query-data/queries#execute_a_query
void ReadDataExecuteQuery(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::QuerySnapshot;

  // After creating a query object, use the Get() function to retrieve the
  // results:
  // This snippet is identical to get_multiple above.
  db->Collection("cities")
      .WhereEqualTo("capital", FieldValue::Boolean(true))
      .Get()
      .OnCompletion([](const Future<QuerySnapshot>& future) {
        if (future.error() == Error::kErrorOk) {
          for (const DocumentSnapshot& document :
               future.result()->documents()) {
            std::cout << document << std::endl;
          }
        } else {
          std::cout << "Error getting documents: " << future.error_message()
                    << std::endl;
        }
      });
}

// https://firebase.google.com/docs/firestore/query-data/queries#query_operators
void ReadDataQueryOperators(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;

  CollectionReference cities_ref = db->Collection("cities");

  // Some example filters:
  // [START example_filters]
  cities_ref.WhereEqualTo("state", FieldValue::String("CA"));
  cities_ref.WhereLessThan("population", FieldValue::Integer(100000));
  cities_ref.WhereGreaterThanOrEqualTo("name",
                                       FieldValue::String("San Francisco"));
  // [END example_filters]

  // [START query_filter_not_eq]
  cities_ref.WhereNotEqualTo("capital", FieldValue::Boolean(false));
  // [END query_filter_not_eq]

}

// https://firebase.google.com/docs/firestore/query-data/queries#array_membership
void ReadDataArrayMembershipOperators(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;

  // Some example filters:
  // [START cpp_array_contains_filter]
  CollectionReference cities_ref = db->Collection("cities");

  cities_ref.WhereArrayContains("region", FieldValue::String("west_coast"));
  // [END cpp_array_contains_filter]

}

// https://firebase.google.com/docs/firestore/query-data/queries#in_not-in_and_array-contains-any
void ReadDataArrayInNotInOperators(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;

  // Some example filters:
  // [START cpp_in_filter]
  CollectionReference cities_ref = db->Collection("cities");

  cities_ref.WhereIn("country", std::vector<FieldValue> {
    FieldValue::String("USA"),
    FieldValue::String("Japan")
  });
  // [END cpp_in_filter]

  // [START cpp_not_in_filter]
  cities_ref.WhereNotIn("country", std::vector<FieldValue> {
    FieldValue::String("USA"),
    FieldValue::String("Japan")
  });
  // [END cpp_not_in_filter]
}

// https://firebase.google.com/docs/firestore/query-data/queries#array-contains-any
void ReadDataArrayContainsAnyOperators(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;

  // Some example filters:
  // [START cpp_array_contains_any_filter]
  CollectionReference cities_ref = db->Collection("cities");

  cities_ref.WhereArrayContainsAny("region", std::vector<FieldValue> {
    FieldValue::String("west_coast"),
    FieldValue::String("east_coast")
  });
  // [END cpp_array_contains_any_filter]

  // [START cpp_in_filter_with_array]
  cities_ref.WhereIn("region", std::vector<FieldValue> {
    FieldValue::String("west_coast"),
    FieldValue::String("east_coast")
  });
  // [END cpp_in_filter_with_array]
}



void QueryCollectionGroupFilterEq(firebase::firestore::Firestore* db) // 2 TODO
{

  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;
  using firebase::firestore::Error;
  using firebase::firestore::QuerySnapshot;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Query;

  // [START query_collection_group_filter_eq]
  db->CollectionGroup("landmarks")
  .WhereEqualTo("type", FieldValue::String("museum")).Get()
  .OnCompletion([](const firebase::Future<QuerySnapshot>& future) {
    if (future.error() == Error::kErrorOk) {
      for (const DocumentSnapshot& document : future.result()->documents()) {
        std::cout << document << std::endl;
      }
    } else {
      std::cout << "Error getting documents: " << future.error_message()
                << std::endl;
    }
  });
  // [END query_collection_group_filter_eq]

}


void QueryCollectionGroupDataset(firebase::firestore::Firestore* db)
{
  using firebase::Future;
  using firebase::firestore::DocumentReference;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::FieldValue;
  using firebase::firestore::QuerySnapshot;
  using firebase::firestore::WriteBatch;

  // [START query_collection_group_dataset]
  // Get a new write batch
  WriteBatch batch = db->batch();

  DocumentReference sf_ref = db->Collection("cities").Document("SF");
  batch.Set(sf_ref,{{"name", FieldValue::String("Golden Gate Bridge")}, {"type", FieldValue::String("bridge")}});
  batch.Set(sf_ref,{{"name", FieldValue::String("Legion of Honor")}, {"type", FieldValue::String("museum")}});

  DocumentReference la_ref = db->Collection("cities").Document("LA");
  batch.Set(la_ref,{{"name", FieldValue::String("Griffith Park")}, {"type", FieldValue::String("park")}});
  batch.Set(la_ref,{{"name", FieldValue::String("The Getty")}, {"type", FieldValue::String("museum")}});

  DocumentReference dc_ref = db->Collection("cities").Document("DC");
  batch.Set(dc_ref,{{"name", FieldValue::String("Lincoln Memorial")}, {"type", FieldValue::String("memorial")}});
  batch.Set(dc_ref,{{"name", FieldValue::String("National Air and Space Museum")}, {"type", FieldValue::String("museum")}});

  DocumentReference tok_ref = db->Collection("cities").Document("TOK");
  batch.Set(tok_ref,{{"name", FieldValue::String("Ueno Park")}, {"type", FieldValue::String("park")}});
  batch.Set(tok_ref,{{"name", FieldValue::String("National Museum of Nature and Science")}, {"type", FieldValue::String("museum")}});

  DocumentReference bj_ref = db->Collection("cities").Document("BJ");
  batch.Set(bj_ref,{{"name", FieldValue::String("Jingshan Park")}, {"type", FieldValue::String("park")}});
  batch.Set(bj_ref,{{"name", FieldValue::String("Beijing Ancient Observatory")}, {"type", FieldValue::String("museum")}});

  // Commit the batch
  batch.Commit().OnCompletion([](const Future<void>& future) {
    if (future.error() == Error::kErrorOk) {
      std::cout << "Write batch success!" << std::endl;
    } else {
      std::cout << "Write batch failure: " << future.error_message() << std::endl;
    }
  });
  // [END query_collection_group_dataset]
}


// https://firebase.google.com/docs/firestore/query-data/queries#compound_queries
void ReadDataCompoundQueries(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;

  CollectionReference cities_ref = db->Collection("cities");

  // You can also chain multiple where() methods to create more specific queries
  // (logical AND). However, to combine the equality operator (==) with a range
  // (<, <=, >, >=) or array-contains clause, make sure to create a composite
  // index.
  // [START chain_filters]
  cities_ref.WhereEqualTo("state", FieldValue::String("CO"))
      .WhereEqualTo("name", FieldValue::String("Denver"));
  cities_ref.WhereEqualTo("state", FieldValue::String("CA"))
      .WhereLessThan("population", FieldValue::Integer(1000000));
  // [END chain_filters]

  // You can only perform range comparisons (<, <=, >, >=) on a single field,
  // and you can include at most one array-contains clause in a compound query:
  // [START valid_range_filters]
  cities_ref.WhereGreaterThanOrEqualTo("state", FieldValue::String("CA"))
      .WhereLessThanOrEqualTo("state", FieldValue::String("IN"));
  cities_ref.WhereEqualTo("state", FieldValue::String("CA"))
      .WhereGreaterThan("population", FieldValue::Integer(1000000));
  // [END valid_range_filters]
}

// This method is left unexecuted to avoid crashing the snippets runner.
// https://firebase.google.com/docs/firestore/query-data/queries#compound_queries
void ReadDataInvalidCompoundQuery(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;

  CollectionReference cities_ref = db->Collection("cities");

  // [START invalid_range_filters]
  // BAD EXAMPLE -- will crash the program:
  cities_ref.WhereGreaterThanOrEqualTo("state", FieldValue::String("CA"))
      .WhereGreaterThan("population", FieldValue::Integer(100000));
  // [END invalid_range_filters]
}

// https://firebase.google.com/docs/firestore/query-data/order-limit-data#order_and_limit_data
void ReadDataOrderAndLimitData(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;
  using firebase::firestore::Query;

  CollectionReference cities_ref = db->Collection("cities");

  // By default, a query retrieves all documents that satisfy the query in
  // ascending order by document ID. You can specify the sort order for your
  // data using OrderBy(), and you can limit the number of documents retrieved
  // using Limit().
  //
  // Note: An OrderBy() clause also filters for existence of the given field.
  // The result set will not include documents that do not contain the given
  // field.
  //
  // For example, you could query for the first 3 cities alphabetically with:
  // [START order_and_limit]
  cities_ref.OrderBy("name").Limit(3);
  // [END order_and_limit]

  // You could also sort in descending order to get the last 3 cities:
  // [START order_and_limit_desc]
  cities_ref.OrderBy("name", Query::Direction::kDescending).Limit(3);
  // [END order_and_limit_desc]

  // You can also order by multiple fields. For example, if you wanted to order
  // by state, and within each state order by population in descending order:
  // [START order_multiple]
  cities_ref.OrderBy("state").OrderBy("name", Query::Direction::kDescending);
  // [END order_multiple]

  // You can combine Where() filters with OrderBy() and Limit(). In the
  // following example, the queries define a population threshold, sort by
  // population in ascending order, and return only the first few results that
  // exceed the threshold:
  // [START filter_and_order]
  cities_ref.WhereGreaterThan("population", FieldValue::Integer(100000))
      .OrderBy("population")
      .Limit(2);
  // [END filter_and_order]
}

// This method is left unexecuted to avoid crashing the snippets runner.
// https://firebase.google.com/docs/firestore/query-data/order-limit-data#order_and_limit_data
void ReadDataInvalidOrderAndLimit(firebase::firestore::Firestore* db) {
  using firebase::firestore::CollectionReference;
  using firebase::firestore::FieldValue;
  using firebase::firestore::Query;

  CollectionReference cities_ref = db->Collection("cities");

  // However, if you have a filter with a range comparison (<, <=, >, >=), your
  // first ordering must be on the same field.
  // [START invalid_filter_and_order]
  // BAD EXAMPLE -- will crash the program:
  cities_ref.WhereGreaterThan("population", FieldValue::Integer(100000))
      .OrderBy("country");
  // [END invalid_filter_and_order]
}

// https://firebase.google.com/docs/firestore/query-data/query-cursors#add_a_simple_cursor_to_a_query
void ReadDataAddSimpleCursorToQuery(firebase::firestore::Firestore* db) {
  using firebase::firestore::FieldValue;

  // Use the StartAt() or StartAfter() methods to define the start point for
  // a query. The StartAt() method includes the start point, while the
  // StartAfter() method excludes it.
  //
  // For example, if you use StartAt(FieldValue::String("A")) in a query, it
  // returns the entire alphabet. If you use
  // StartAftert(FieldValue::String("A")) instead, it returns B-Z.

  // [START cursor_greater_than]
  // Get all cities with a population >= 1,000,000, ordered by population,
  db->Collection("cities")
      .OrderBy("population")
      .StartAt({FieldValue::Integer(1000000)});
  // [END cursor_greater_than]

  // Similarly, use the EndAt() or EndBefore() methods to define an end point
  // for your query results.
  // [START cursor_less_than]
  // Get all cities with a population <= 1,000,000, ordered by population,
  db->Collection("cities")
      .OrderBy("population")
      .EndAt({FieldValue::Integer(1000000)});
  // [END cursor_less_than]
}

// https://firebase.google.com/docs/firestore/query-data/query-cursors#use_a_document_snapshot_to_define_the_query_cursor
void ReadDataDocumentSnapshotInCursor(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::Query;

  // You can also pass a document snapshot to the cursor clause as the start or
  // end point of the query cursor. The values in the document snapshot serve as
  // the values in the query cursor.
  //
  // For example, take a snapshot of a "San Francisco" document in your data set
  // of cities and populations. Then, use that document snapshot as the start
  // point for your population query cursor. Your query will return all the
  // cities with a population larger than or equal to San Francisco's, as
  // defined in the document snapshot.
  // [START snapshot_cursor]
  db->Collection("cities").Document("SF").Get().OnCompletion(
      [db](const Future<DocumentSnapshot>& future) {
        if (future.error() == Error::kErrorOk) {
          const DocumentSnapshot& document_snapshot = *future.result();
          Query bigger_than_sf = db->Collection("cities")
                                     .OrderBy("population")
                                     .StartAt({document_snapshot});
          // ...
        }
      });
  // [END snapshot_cursor]
}

// https://firebase.google.com/docs/firestore/query-data/query-cursors#paginate_a_query
void ReadDataPaginateQuery(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::DocumentSnapshot;
  using firebase::firestore::Error;
  using firebase::firestore::Query;
  using firebase::firestore::QuerySnapshot;

  // Paginate queries by combining query cursors with the Limit() method. For
  // example, use the last document in a batch as the start of a cursor for the
  // next batch.

  // [START paginate]
  // Construct query for first 25 cities, ordered by population
  Query first = db->Collection("cities").OrderBy("population").Limit(25);

  first.Get().OnCompletion([db](const Future<QuerySnapshot>& future) {
    if (future.error() != Error::kErrorOk) {
      // Handle error...
      return;
    }

    // Get the last visible document
    const QuerySnapshot& document_snapshots = *future.result();
    std::vector<DocumentSnapshot> documents = document_snapshots.documents();
    const DocumentSnapshot& last_visible = documents.back();

    // Construct a new query starting at this document,
    // get the next 25 cities.
    Query next = db->Collection("cities")
                     .OrderBy("population")
                     .StartAfter(last_visible)
                     .Limit(25);

    // Use the query for pagination
    // ...
  });
  // [END paginate]
}

// https://firebase.google.com/docs/firestore/bundles#loading_data_bundles_in_the_client
void LoadFirestoreBundles(firebase::firestore::Firestore* db) {
  using firebase::Future;
  using firebase::firestore::Error;
  using firebase::firestore::Firestore;
  using firebase::firestore::LoadBundleTaskProgress;
  using firebase::firestore::Query;
  using firebase::firestore::QuerySnapshot;

  // [START bundled_query]
  db->LoadBundle("bundle_name", [](const LoadBundleTaskProgress& progress) {
    switch(progress.state()) {
      case LoadBundleTaskProgress::State::kError: {
        // The bundle load has errored. Handle the error in the returned future.
        return;
      }
      case LoadBundleTaskProgress::State::kInProgress: {
        std::cout << "Bytes loaded from bundle: " << progress.bytes_loaded()
                  << std::endl;
        break;
      }
      case LoadBundleTaskProgress::State::kSuccess: {
        std::cout << "Bundle load succeeeded" << std::endl;
        break;
      }
    }
  }).OnCompletion([db](const Future<LoadBundleTaskProgress>& future) {
    if (future.error() != Error::kErrorOk) {
      // Handle error...
      return;
    }

    const std::string& query_name = "latest_stories_query";
    db->NamedQuery(query_name).OnCompletion([](const Future<Query>& query_future){
      if (query_future.error() != Error::kErrorOk) {
        // Handle error...
        return;
      }

      const Query* query = query_future.result();
      query->Get().OnCompletion([](const Future<QuerySnapshot> &){
        // ...
      });
    });
  });
  // [END bundled_query]
}

}  // namespace snippets

void RunAllSnippets(firebase::firestore::Firestore* db) {
  snippets::DataModelReferenceDeclarations(db);

  snippets::QuickstartAddData(db);
  snippets::QuickstartReadData(db);

  snippets::AddDataSetDocument(db);
  snippets::AddDataDataTypes(db);
  snippets::AddDataAddDocument(db);
  snippets::AddDataUpdateDocument(db);
  snippets::AddDataUpdateNestedObjects(db);
  snippets::AddDataBatchedWrites(db);
  snippets::AddDataTransactions(db);
  snippets::AddDataDeleteDocuments(db);
  snippets::AddDataDeleteFields(db);

  snippets::ReadDataExampleData(db);
  snippets::ReadDataGetDocument(db);
  snippets::ReadDataSourceOptions(db);
  snippets::ReadDataGetMultipleDocumentsFromCollection(db);
  snippets::ReadDataGetAllDocumentsInCollection(db);

  snippets::ReadDataListen(db);
  snippets::ReadDataEventsForLocalChanges(db);
  snippets::ReadDataEventsForMetadataChanges(db);
  snippets::ReadDataListenToMultipleDocumentsInCollection(db);
  snippets::ReadDataViewChangesBetweenSnapshots(db);
  snippets::ReadDataDetachListener(db);

  snippets::ReadDataSimpleQueries(db);
  snippets::ReadDataExecuteQuery(db);
  snippets::ReadDataQueryOperators(db);
  snippets::ReadDataCompoundQueries(db);
  snippets::QueryCollectionGroupDataset(db);
  snippets::QueryCollectionGroupFilterEq(db);

  snippets::ReadDataOrderAndLimitData(db);

  snippets::ReadDataAddSimpleCursorToQuery(db);

  snippets::ReadDataDocumentSnapshotInCursor(db);
  snippets::ReadDataPaginateQuery(db);
}

SnippetsRunner::SnippetsRunner() {}

void SnippetsRunner::runAllSnippets() {
  auto firestore = firebase::firestore::Firestore::GetInstance();
  RunAllSnippets(firestore);
}
