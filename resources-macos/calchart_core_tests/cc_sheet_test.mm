//
//  cc_sheet_test.m
//  cc_sheet_tests
//
//  Created by Richard Powell on 12/9/17.
//

#import <XCTest/XCTest.h>
#import "cc_show.h"

@interface cc_sheet_tests : XCTestCase

@end

@implementation cc_sheet_tests

- (void)test_builtin {
    CalChart::Sheet_UnitTests();
}

@end
