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

#include "snippets_runner.h"
#include "snippets.h"
#include "firebase/app.h"

#include <jni.h>

extern "C" {

JNIEXPORT void JNICALL Java_com_firebase_firestoresnippetscpp_SnippetsRunner_runSnippets(JNIEnv *env, jobject /* this */) {
  auto runner = SnippetsRunner();
  runner.runAllSnippets();
}

JNIEXPORT void JNICALL Java_com_firebase_firestoresnippetscpp_MainActivity_initializeFirebase(JNIEnv *env, jobject object) {
  firebase::App::Create(env, object);
}

}
