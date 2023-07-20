//
//  AuthVM.swift
//  Created by Michael Simms on 6/29/23.
//

import Foundation

let PREF_NAME_SESSION_TOKEN = "Session Token"

class AuthVM : ObservableObject {
	var loginErrorReceived: Bool = false
	@Published var loginStatus: LoginStatus = LoginStatus.LOGIN_STATUS_UNKNOWN

	init() {
		let _ = self.isLoggedIn()

		NotificationCenter.default.addObserver(self, selector: #selector(self.loginStatusUpdated), name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGIN_CHECKED), object: nil)
		NotificationCenter.default.addObserver(self, selector: #selector(self.loginProcessed), name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGIN_PROCESSED), object: nil)
		NotificationCenter.default.addObserver(self, selector: #selector(self.createLoginProcessed), name: Notification.Name(rawValue: NOTIFICATION_NAME_CREATE_LOGIN_PROCESSED), object: nil)
		NotificationCenter.default.addObserver(self, selector: #selector(self.logoutProcessed), name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGGED_OUT), object: nil)
	}
	
	func sessionToken() -> String? {
		let mydefaults: UserDefaults = UserDefaults.standard
		return mydefaults.string(forKey: PREF_NAME_SESSION_TOKEN)
	}

	func setSessionToken(token: String) {
		let mydefaults: UserDefaults = UserDefaults.standard
		mydefaults.set(token, forKey: PREF_NAME_SESSION_TOKEN)
	}

	func isLoggedIn() -> Bool {
		let token = self.sessionToken()
		guard let unwrappedToken = token else {
			return false
		}
		return ApiClient.shared.isLoggedIn(token: unwrappedToken)
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
				
				// Request was valid.
				if responseCode.statusCode == 200 {
					DispatchQueue.main.async { self.loginStatus = LoginStatus.LOGIN_STATUS_SUCCESS }
				}
				else {
					DispatchQueue.main.async { self.loginStatus = LoginStatus.LOGIN_STATUS_FAILURE }
				}
			}
		}
	}
	
	@objc func loginProcessed(notification: NSNotification) {
		var success = false

		do {
			if let data = notification.object as? Dictionary<String, AnyObject> {
				if let responseCode = data[KEY_NAME_RESPONSE_CODE] as? HTTPURLResponse {
					
					// Request was valid.
					if responseCode.statusCode == 200, let responseData = data[KEY_NAME_RESPONSE_DATA] as? Data {
						if let responseDict = try JSONSerialization.jsonObject(with: responseData, options: []) as? Dictionary<String, Any> {
							if let sessionToken = responseDict[PARAM_SESSION_TOKEN] as? String {
								self.setSessionToken(token: sessionToken)
								DispatchQueue.main.async { self.loginStatus = LoginStatus.LOGIN_STATUS_SUCCESS }
								success = true
							}
						}
					}
				}
			}
		}
		catch {
			NSLog(error.localizedDescription)
		}

		if success == false {
			DispatchQueue.main.async { self.loginStatus = LoginStatus.LOGIN_STATUS_FAILURE }
			self.loginErrorReceived = true
		}
	}
	
	@objc func createLoginProcessed(notification: NSNotification) {
		if let data = notification.object as? Dictionary<String, AnyObject> {
			if let responseCode = data[KEY_NAME_RESPONSE_CODE] as? HTTPURLResponse {
				if let sessionToken = data[PARAM_SESSION_TOKEN] as? String {
				}
				if responseCode.statusCode == 200 {
					DispatchQueue.main.async { self.loginStatus = LoginStatus.LOGIN_STATUS_SUCCESS }
				}
				else {
					DispatchQueue.main.async { self.loginStatus = LoginStatus.LOGIN_STATUS_FAILURE }
					self.loginErrorReceived = true
				}
			}
		}
	}
	
	@objc func logoutProcessed(notification: NSNotification) {
		if let data = notification.object as? Dictionary<String, AnyObject> {
			if let responseCode = data[KEY_NAME_RESPONSE_CODE] as? HTTPURLResponse {
				if responseCode.statusCode == 200 {
					DispatchQueue.main.async { self.loginStatus = LoginStatus.LOGIN_STATUS_FAILURE }
				}
			}
		}
	}
}
