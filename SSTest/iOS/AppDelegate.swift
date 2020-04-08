//
//  AppDelegate.swift
//  SSTest
//
//  Created by Tim DeBenedictis on 4/5/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {



    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Override point for customization after application launch.

/*      This was the original test code. We've graduated beyond this, but keeping for posterity ... for now.

         // get path to SSData folder within main bundle as C string for input
        
        let inpath = ( Bundle.main.bundlePath.appending ("/SSData") as NSString ).utf8String
        
        // get path to Documents directory as C string for output
        
        let fileMgr = FileManager.default
        let docsDirURL = try! fileMgr.url(for: .documentDirectory, in: .userDomainMask, appropriateFor: nil, create: true)
        let outpath = ( docsDirURL.path as NSString ).utf8String
        
        // call SSTest main() function
        
        let SSTestMain ( inpath, outpath )
        return result == 0 ? true : false
*/
        return true
    }

    // MARK: UISceneSession Lifecycle

    func application(_ application: UIApplication, configurationForConnecting connectingSceneSession: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        // Called when a new scene session is being created.
        // Use this method to select a configuration to create the new scene with.
        return UISceneConfiguration(name: "Default Configuration", sessionRole: connectingSceneSession.role)
    }

    func application(_ application: UIApplication, didDiscardSceneSessions sceneSessions: Set<UISceneSession>) {
        // Called when the user discards a scene session.
        // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
        // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
    }


}

