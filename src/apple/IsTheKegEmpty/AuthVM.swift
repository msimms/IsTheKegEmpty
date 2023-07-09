//
//  AuthVM.swift
//  Created by Michael Simms on 6/29/23.
//

import Foundation

class AuthVM : ObservableObject {
	init() {
		NotificationCenter.default.addObserver(self, selector: #selector(self.loginStatusUpdated), name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGIN_CHECKED), object: nil)
		NotificationCenter.default.addObserver(self, selector: #selector(self.loginProcessed), name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGIN_PROCESSED), object: nil)
		NotificationCenter.default.addObserver(self, selector: #selector(self.createLoginProcessed), name: Notification.Name(rawValue: NOTIFICATION_NAME_CREATE_LOGIN_PROCESSED), object: nil)
		NotificationCenter.default.addObserver(self, selector: #selector(self.logoutProcessed), name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGGED_OUT), object: nil)
	}
	
	func login(username: String, password: String) -> Bool {
		return ApiClient.shared.login(username: username, password: password)
	}
	
	func createLogin(username: String, password1: String, password2: String, realname: String) -> Bool {
		return ApiClient.shared.createLogin(username: username, password1: password1, password2: password2, realname: realname)
	}
	
	@objc func loginStatusUpdated(notification: NSNotification) {
		if let data = notification.object as? Dictionary<String, AnyObject> {
			if let responseCode = data[KEY_NAME_RESPONSE_CODE] as? HTTPURLResponse {
				ApiClient.shared.loggedIn = responseCode.statusCode == 200
			}
		}
	}
	
	@objc func loginProcessed(notification: NSNotification) {
		if let data = notification.object as? Dictionary<String, AnyObject> {
			if let responseCode = data[KEY_NAME_RESPONSE_CODE] as? HTTPURLResponse {
				ApiClient.shared.loggedIn = responseCode.statusCode == 200
			}
		}
	}
	
	@objc func createLoginProcessed(notification: NSNotification) {
		if let data = notification.object as? Dictionary<String, AnyObject> {
			if let responseCode = data[KEY_NAME_RESPONSE_CODE] as? HTTPURLResponse {
				ApiClient.shared.loggedIn = responseCode.statusCode == 200
			}
		}
	}
	
	@objc func logoutProcessed(notification: NSNotification) {
		if let data = notification.object as? Dictionary<String, AnyObject> {
			if let responseCode = data[KEY_NAME_RESPONSE_CODE] as? HTTPURLResponse {
				ApiClient.shared.loggedIn = responseCode.statusCode == 200
			}
		}
	}
}
