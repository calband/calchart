//
//  cc_coord_test.m
//  cc_coord_test
//
//  Created by Richard Powell on 12/9/17.
//

#import <XCTest/XCTest.h>
#import "cc_show.h"

@interface cc_coord_tests : XCTestCase

@end

@implementation cc_coord_tests

- (void)test_builtin {
    CalChart::Coord_UnitTests();
}

@end
