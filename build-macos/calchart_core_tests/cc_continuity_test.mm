//
//  cc_continuity_test.m
//  cc_continuity_test
//
//  Created by Richard Powell on 12/9/17.
//

#import <XCTest/XCTest.h>
#import "cc_continuity.h"

@interface cc_continuity_tests : XCTestCase

@end

@implementation cc_continuity_tests

- (void)test_builtin {
    CalChart::Continuity_UnitTests();
}

@end
