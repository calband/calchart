//
//  cc_point_test.m
//  cc_point_tests
//
//  Created by Richard Powell on 12/9/17.
//

#import <XCTest/XCTest.h>
#import "cc_show.h"

@interface cc_point_tests : XCTestCase

@end

@implementation cc_point_tests

- (void)test_builtin {
    CalChart::Point_UnitTests();
}

@end
