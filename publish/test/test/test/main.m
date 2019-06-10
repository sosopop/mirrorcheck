//
//  main.m
//  test
//
//  Created by sosopop on 2018/12/12.
//  Copyright © 2018 sosopop. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#include <mirroraccel/mirroraccel.h>

int main(int argc, char * argv[]) {
    mirror_accel_init();
    int port = mirror_accel_create(
                        "0.0.0.0:0",
                        "{"\
                        "\"targets\":["\
                        "{"\
                        "\"url\":\"http://42.51.203.25/xszr.mp4?id=1\""\
                        "},"\
                        "{"\
                        " \"url\":\"http://42.51.203.25/xszr.mp4?id=2\""\
                        "},"\
                        "{"\
                        " \"url\":\"http://42.51.203.25/xszr.mp4?id=3\""\
                        "},"\
                        "{"\
                        " \"url\":\"http://42.51.203.25/xszr.mp4?id=4\""\
                        "},"\
                        "{"\
                        " \"url\":\"http://42.51.203.25/xszr.mp4?id=5\""\
                        "}"\
                        "]"\
                        "}"
                        );
    printf("http://127.0.0.1:%d/stream/test.mp4\n", port);
    
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
