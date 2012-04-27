//
//  RestKit.h
//  RestKit
//
//  Created by Blake Watters on 2/19/10.
//  Copyright 2010 Two Toasters. All rights reserved.
//

#import "Network/Network.h"
#import "Support/Support.h"
#import "ObjectMapping/ObjectMapping.h"

#define NSLog(__FORMAT__, ...) TFLog((@"%s [Line %d] " __FORMAT__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
