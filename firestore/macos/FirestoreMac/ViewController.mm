//
//  ViewController.mm
//  FirestoreMac
//
//  Created by Morgan Chen on 4/21/20.
//  Copyright © 2020 Firebase. All rights reserved.
//

#import "ViewController.h"
#import "snippets.h"

@implementation ViewController {
  SnippetsRunner _snippetsRunner;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  _snippetsRunner = SnippetsRunner();
}

- (void)viewDidAppear {
  [super viewDidAppear];

  _snippetsRunner.runAllSnippets();
}

- (void)setRepresentedObject:(id)representedObject {
  [super setRepresentedObject:representedObject];

  // Update the view, if already loaded.
}


@end
