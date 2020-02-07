package com.firebase.firestoresnippetscpp

class SnippetsRunner {

    companion object {
        init {
            System.loadLibrary("firestore-snippets")
        }
    }

    external fun runSnippets()
}
